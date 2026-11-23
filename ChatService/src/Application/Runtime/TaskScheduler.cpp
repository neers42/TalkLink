#include "Application/Runtime/TaskScheduler.hpp"

// CommonLibのスレッドローカル変数を使用
extern thread_local std::string g_current_task_id;

TaskScheduler::TaskScheduler(
    std::shared_ptr<ThreadPoolHandler> heartbeat_pool,
    std::shared_ptr<ThreadPoolHandler> realtime_pool,
    std::shared_ptr<ThreadPoolHandler> background_pool)
    : _heartbeat_pool(std::move(heartbeat_pool)),
      _realtime_pool(std::move(realtime_pool)),
      _background_pool(std::move(background_pool))
{
    if (!_heartbeat_pool || !_realtime_pool || !_background_pool) {
        throw std::runtime_error("ThreadPoolHandler is null");
    }

    LOG_INFO("[TaskScheduler] TaskScheduler initialized as infrastructure layer (no service dependencies)");
}

std::shared_ptr<ThreadPoolHandler>& TaskScheduler::select_pool_for_category(TaskCategory category)
{
    switch (category) {
        case TaskCategory::HEARTBEAT:
            return _heartbeat_pool;
        case TaskCategory::AUTHENTICATION:
        case TaskCategory::USER_MANAGEMENT:
        case TaskCategory::CHAT_PROCESSING:
            return _realtime_pool;
        case TaskCategory::MAINTENANCE:
        case TaskCategory::SYSTEM:
        default:
            return _background_pool;
    }
}

TaskScheduler::~TaskScheduler() {
    // シャットダウン前にすべてのタスクをクリアする
    {
        std::lock_guard<std::mutex> lock(_active_tasks_mutex);
        _active_tasks.clear();
    }
    
    LOG_INFO("[TaskScheduler] TaskScheduler shutting down - Total tasks executed: {}, Failed: {}", 
                _total_tasks_executed.load(), _failed_tasks.load());
}

// ==================== タスク実行 & モニタリング ====================

void TaskScheduler::execute_managed_task(const std::string& task_id, std::function<void()> task,
                                                   std::chrono::milliseconds max_execution_time)
{
    auto start_time = std::chrono::steady_clock::now();

    // スレッドローカルタスクIDを設定
    set_current_task_id(task_id);

    // アクティブタスクを登録
    {
        std::lock_guard<std::mutex> lock(_active_tasks_mutex);
        _active_tasks[task_id] = ActiveTaskInfo{start_time, max_execution_time};
    }
    
    bool success = false;
    try {
        // 過負荷保護をチェック
        if (_overload_protection && _active_tasks.size() > get_overload_config().active_task_hard_limit) {
            LOG_WARNING("[TaskScheduler] Task {} skipped due to overload protection", task_id);
            clear_current_task_id();
            return;
        }
        
        // 実際のタスクを実行
        task();
        success = true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[TaskScheduler] Task {} execution failed: {}", task_id, e.what());
        _failed_tasks++;
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // メトリクスを更新
    _total_tasks_executed++;
    if (success) {
        LOG_TRACE("[TaskScheduler] Task {} completed in {}ms", task_id, execution_time.count());
    }
    
    // アクティブタスクから削除
    {
        std::lock_guard<std::mutex> lock(_active_tasks_mutex);
        _active_tasks.erase(task_id);
    }
    
    // スレッドローカルタスクIDをクリア
    clear_current_task_id();
}

void TaskScheduler::enqueue_by_priority(ManagedTask managed_task)
{
    // カテゴリに応じたプールへ振り分けてから、マネージドタスクを実行するラッパーを登録する
    auto& pool = select_pool_for_category(managed_task.metadata.category);
    pool->enqueue_task([this, task = std::move(managed_task)]() mutable {
        execute_managed_task(task.metadata.task_id, task.task_function, task.max_execution_time);
    });
}

// ==================== ユーティリティメソッド ====================

std::string TaskScheduler::generate_task_id()
{
    auto counter = ++_task_counter;
    auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return "task_" + std::to_string(timestamp) + "_" + std::to_string(counter);
}

TaskScheduler::TaskMetrics TaskScheduler::get_task_metrics() const
{
    std::lock_guard<std::mutex> lock(_active_tasks_mutex);
    
    TaskMetrics metrics;
    metrics.total_executed = _total_tasks_executed.load();
    metrics.failed_tasks = _failed_tasks.load();
    metrics.timeout_tasks = _timeout_tasks.load();
    metrics.active_count = _active_tasks.size();
    
    // 平均実行時間を計算（簡易版）
    double total_time = 0.0;
    if (!_active_tasks.empty()) {
        auto now = std::chrono::steady_clock::now();
        for (const auto& [task_id, info] : _active_tasks) {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - info.start_time);
            total_time += duration.count();
        }
        metrics.average_execution_time = total_time / _active_tasks.size();
    } else {
        metrics.average_execution_time = 0.0;
    }
    
    metrics.overload_status = _overload_protection.load();
    
    return metrics;
}

bool TaskScheduler::is_overloaded() const
{
    return _overload_protection.load();
}

void TaskScheduler::update_overload_config(const OverloadProtectionConfig& config)
{
    std::lock_guard<std::mutex> lock(_overload_config_mutex);
    _overload_config = config;
    LOG_INFO("[TaskScheduler] Overload protection config updated - max_active_tasks: {}, active_task_hard_limit: {}, overload_recovery_ratio: {}",
             config.max_active_tasks, config.active_task_hard_limit, config.overload_recovery_ratio);
}

OverloadProtectionConfig TaskScheduler::get_overload_config() const
{
    std::lock_guard<std::mutex> lock(_overload_config_mutex);
    return _overload_config;
}



bool TaskScheduler::cancel_task(const std::string& task_id)
{
    try {
        std::lock_guard<std::mutex> lock(_active_tasks_mutex);
        
        auto it = _active_tasks.find(task_id);
        if (it != _active_tasks.end()) {
            _active_tasks.erase(it);
            LOG_DEBUG("[TaskScheduler] Task {} cancelled", task_id);
            return true;
        }
        
        LOG_WARNING("[TaskScheduler] Attempted to cancel non-existent task: {}", task_id);
        return false;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[TaskScheduler] Exception while cancelling task {}: {}", task_id, e.what());
        return false;
    }
}

void TaskScheduler::cancel_user_tasks(socket_t socket)
{
    try {
        std::lock_guard<std::mutex> lock(_active_tasks_mutex);
        
        size_t cancelled_count = 0;
        
        // クライアントソケットに関連するすべてのタスクを検索して削除する
        auto it = _active_tasks.begin();
        while (it != _active_tasks.end()) {
            // ノート：実際の実装では、ソケットの関連付けを保存する必要があります
            // 現在は、単純なタスクIDのパターンマッチングを使用します
            if (it->first.find("_socket_" + std::to_string(socket) + "_") != std::string::npos) {
                it = _active_tasks.erase(it);
                ++cancelled_count;
            } else {
                ++it;
            }
        }
        
        if (cancelled_count > 0) {
            LOG_INFO("[TaskScheduler] Cancelled {} tasks for user socket: {}", cancelled_count, socket);
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("[TaskScheduler] Exception while cancelling user tasks for socket {}: {}", socket, e.what());
    }
}

uint32_t TaskScheduler::get_user_task_count(socket_t socket) const
{
    try {
        std::lock_guard<std::mutex> lock(_active_tasks_mutex);
        
        uint32_t count = 0;
        for (const auto& [task_id, info] : _active_tasks) {
            // ノート：実際の実装では、ソケットの関連付けを保存する必要があります
            if (task_id.find("_socket_" + std::to_string(socket) + "_") != std::string::npos) {
                ++count;
            }
        }
        
        return count;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[TaskScheduler] Exception while getting user task count for socket {}: {}", socket, e.what());
        return 0;
    }
}

void TaskScheduler::enable_overload_protection(bool enabled)
{
    const bool was_enabled = _overload_protection_enabled.exchange(enabled);

    if (enabled) {
        LOG_INFO("[TaskScheduler] Task overload protection enabled");

        if (!was_enabled) {
            // 定期的な過負荷チェックをスケジュールする(既に起動中なら再スケジュールしない)
            schedule_task(
                TaskPriority::LOW,
                TaskCategory::SYSTEM,
                "Overload Monitoring",
                [this]() {
                    check_system_overload();
                }
            );
        }
    } else {
        LOG_INFO("[TaskScheduler] Task overload protection disabled");
        _overload_protection.store(false);
    }
}

void TaskScheduler::update_metrics(const TaskMetadata& metadata, bool success, 
                                         std::chrono::milliseconds execution_time)
{
    try {
        if (success) {
            _total_tasks_executed++;
            LOG_TRACE("[TaskScheduler] Task {} ({}) completed successfully in {}ms", 
                         metadata.task_id, metadata.description, execution_time.count());
        } else {
            _failed_tasks++;
            LOG_WARNING("[TaskScheduler] Task {} ({}) failed", metadata.task_id, metadata.description);
        }
        
        // タイムアウトをチェック
        if (execution_time > metadata.max_execution_time) {
            _timeout_tasks++;
            LOG_WARNING("[TaskScheduler] Task {} exceeded timeout: {}ms > {}ms", 
                        metadata.task_id, execution_time.count(), 
                        metadata.max_execution_time.count());
        }
        
        // 過負荷保護を必要に応じて更新
        if (_overload_protection && execution_time > _overload_threshold) {
            LOG_WARNING("[TaskScheduler] Task execution time exceeds overload threshold");
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("[TaskScheduler] Exception while updating metrics: {}", e.what());
    }
}

// ==================== 内部ヘルパーメソッド ====================

void TaskScheduler::check_system_overload()
{
    try {
        std::lock_guard<std::mutex> lock(this->_active_tasks_mutex);
        
        size_t active_count = _active_tasks.size();
        auto current_time = std::chrono::steady_clock::now();

        // シンプルな過負荷検出: アクティブタスクが多すぎる
        const auto overload_config = get_overload_config();
        const size_t MAX_ACTIVE_TASKS = overload_config.max_active_tasks;

        if (active_count > MAX_ACTIVE_TASKS) {
            _overload_protection.store(true);
            LOG_WARNING("[TaskScheduler] System overload detected: {} active tasks", active_count);
        } else if (active_count < MAX_ACTIVE_TASKS * overload_config.overload_recovery_ratio) {
            _overload_protection.store(false);
        }
        
        // 長時間実行中のタスクをチェック(ハートビート監視など意図的に常駐する
        // タスクは、十分に大きいmax_execution_timeで登録されているため対象外になる)
        for (const auto& [task_id, info] : this->_active_tasks) {
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(current_time - info.start_time);
            if (duration > std::chrono::duration_cast<std::chrono::seconds>(info.max_execution_time)) {
                LOG_WARNING("[TaskScheduler] Long-running task detected: {} running for {}s (limit: {}s)",
                            task_id, duration.count(),
                            std::chrono::duration_cast<std::chrono::seconds>(info.max_execution_time).count());
            }
        }
        
        // 過負荷監視機能自体が有効な場合は次の過負荷チェックをスケジュールする
        // (現在の過負荷状態_overload_protectionではなく、機能の有効/無効で判定する。
        //  過負荷状態を条件にすると、負荷が正常域に戻った直後にポーリングが
        //  永久に停止してしまい、再度過負荷になっても検知できなくなるため)
        if (this->_overload_protection_enabled) {
            schedule_task(
                TaskPriority::LOW,
                TaskCategory::SYSTEM,
                "Overload Monitoring",
                [this]() {
                    std::this_thread::sleep_for(std::chrono::seconds(5)); // Wait 5 seconds
                    check_system_overload();
                }
            );
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("[TaskScheduler] Exception during overload check: {}", e.what());
    }
}

// ==================== スレッドローカルタスク管理 ====================

std::string TaskScheduler::get_current_task_id()
{
    return g_current_task_id;
}

std::string TaskScheduler::get_current_task_id_format()
{
    if (g_current_task_id.empty()) {
        return "";
    }
    return "[" + g_current_task_id + "]";
}

void TaskScheduler::set_current_task_id(const std::string& task_id)
{
    g_current_task_id = task_id;
}

void TaskScheduler::clear_current_task_id()
{
    g_current_task_id.clear();
}
