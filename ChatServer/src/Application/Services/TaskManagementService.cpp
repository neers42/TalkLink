#include "Application/Services/TaskManagementService.hpp"

// CommonLibのスレッドローカル変数を使用
extern thread_local std::string g_current_task_id;

TaskManagementService::TaskManagementService(std::shared_ptr<ThreadPoolHandler> thread_pool)
    : _thread_pool(std::move(thread_pool))
{
    if (!_thread_pool) {
        throw std::runtime_error("ThreadPoolHandler is null");
    }
    
    LOG_INFO("[TaskManagementService] TaskManagementService initialized as infrastructure layer (no service dependencies)");
}

TaskManagementService::~TaskManagementService() {
    // シャットダウン前にすべてのタスクをクリアする
    {
        std::lock_guard<std::mutex> lock(_active_tasks_mutex);
        _active_tasks.clear();
    }
    
    LOG_INFO("[TaskManagementService] TaskManagementService shutting down - Total tasks executed: {}, Failed: {}", 
                _total_tasks_executed.load(), _failed_tasks.load());
}

// ==================== タスク実行 & モニタリング ====================

void TaskManagementService::execute_managed_task(const std::string& task_id, std::function<void()> task,
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
        if (_overload_protection && _active_tasks.size() > 150) {
            LOG_WARNING("[TaskManagementService] Task {} skipped due to overload protection", task_id);
            clear_current_task_id();
            return;
        }
        
        // 実際のタスクを実行
        task();
        success = true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[TaskManagementService] Task {} execution failed: {}", task_id, e.what());
        _failed_tasks++;
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // メトリクスを更新
    _total_tasks_executed++;
    if (success) {
        LOG_TRACE("[TaskManagementService] Task {} completed in {}ms", task_id, execution_time.count());
    }
    
    // アクティブタスクから削除
    {
        std::lock_guard<std::mutex> lock(_active_tasks_mutex);
        _active_tasks.erase(task_id);
    }
    
    // スレッドローカルタスクIDをクリア
    clear_current_task_id();
}

void TaskManagementService::enqueue_by_priority(ManagedTask managed_task)
{
    // マネージドタスクを実行するラッパーを作成
    _thread_pool->enqueue_task([this, task = std::move(managed_task)]() mutable {
        execute_managed_task(task.metadata.task_id, task.task_function, task.max_execution_time);
    });
}

// ==================== ユーティリティメソッド ====================

std::string TaskManagementService::generate_task_id()
{
    auto counter = ++_task_counter;
    auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return "task_" + std::to_string(timestamp) + "_" + std::to_string(counter);
}

TaskManagementService::TaskMetrics TaskManagementService::get_task_metrics() const
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

bool TaskManagementService::is_overloaded() const
{
    return _overload_protection.load();
}



bool TaskManagementService::cancel_task(const std::string& task_id)
{
    try {
        std::lock_guard<std::mutex> lock(_active_tasks_mutex);
        
        auto it = _active_tasks.find(task_id);
        if (it != _active_tasks.end()) {
            _active_tasks.erase(it);
            LOG_DEBUG("[TaskManagementService] Task {} cancelled", task_id);
            return true;
        }
        
        LOG_WARNING("[TaskManagementService] Attempted to cancel non-existent task: {}", task_id);
        return false;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[TaskManagementService] Exception while cancelling task {}: {}", task_id, e.what());
        return false;
    }
}

void TaskManagementService::cancel_user_tasks(socket_t socket)
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
            LOG_INFO("[TaskManagementService] Cancelled {} tasks for user socket: {}", cancelled_count, socket);
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("[TaskManagementService] Exception while cancelling user tasks for socket {}: {}", socket, e.what());
    }
}

uint32_t TaskManagementService::get_user_task_count(socket_t socket) const
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
        LOG_ERROR("[TaskManagementService] Exception while getting user task count for socket {}: {}", socket, e.what());
        return 0;
    }
}

void TaskManagementService::enable_overload_protection(bool enabled)
{
    const bool was_enabled = _overload_protection_enabled.exchange(enabled);

    if (enabled) {
        LOG_INFO("[TaskManagementService] Task overload protection enabled");

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
        LOG_INFO("[TaskManagementService] Task overload protection disabled");
        _overload_protection.store(false);
    }
}

void TaskManagementService::update_metrics(const TaskMetadata& metadata, bool success, 
                                         std::chrono::milliseconds execution_time)
{
    try {
        if (success) {
            _total_tasks_executed++;
            LOG_TRACE("[TaskManagementService] Task {} ({}) completed successfully in {}ms", 
                         metadata.task_id, metadata.description, execution_time.count());
        } else {
            _failed_tasks++;
            LOG_WARNING("[TaskManagementService] Task {} ({}) failed", metadata.task_id, metadata.description);
        }
        
        // タイムアウトをチェック
        if (execution_time > metadata.max_execution_time) {
            _timeout_tasks++;
            LOG_WARNING("[TaskManagementService] Task {} exceeded timeout: {}ms > {}ms", 
                        metadata.task_id, execution_time.count(), 
                        metadata.max_execution_time.count());
        }
        
        // 過負荷保護を必要に応じて更新
        if (_overload_protection && execution_time > _overload_threshold) {
            LOG_WARNING("[TaskManagementService] Task execution time exceeds overload threshold");
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("[TaskManagementService] Exception while updating metrics: {}", e.what());
    }
}

// ==================== 内部ヘルパーメソッド ====================

void TaskManagementService::check_system_overload()
{
    try {
        std::lock_guard<std::mutex> lock(this->_active_tasks_mutex);
        
        size_t active_count = _active_tasks.size();
        auto current_time = std::chrono::steady_clock::now();
        
        // シンプルな過負荷検出: アクティブタスクが多すぎる
        const size_t MAX_ACTIVE_TASKS = 100;
        
        if (active_count > MAX_ACTIVE_TASKS) {
            _overload_protection.store(true);
            LOG_WARNING("[TaskManagementService] System overload detected: {} active tasks", active_count);
        } else if (active_count < MAX_ACTIVE_TASKS * 0.7) { // 70% 以下で過負荷解除
            _overload_protection.store(false);
        }
        
        // 長時間実行中のタスクをチェック(ハートビート監視など意図的に常駐する
        // タスクは、十分に大きいmax_execution_timeで登録されているため対象外になる)
        for (const auto& [task_id, info] : this->_active_tasks) {
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(current_time - info.start_time);
            if (duration > std::chrono::duration_cast<std::chrono::seconds>(info.max_execution_time)) {
                LOG_WARNING("[TaskManagementService] Long-running task detected: {} running for {}s (limit: {}s)",
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
        LOG_ERROR("[TaskManagementService] Exception during overload check: {}", e.what());
    }
}

// ==================== スレッドローカルタスク管理 ====================

std::string TaskManagementService::get_current_task_id()
{
    return g_current_task_id;
}

std::string TaskManagementService::get_current_task_id_format()
{
    if (g_current_task_id.empty()) {
        return "";
    }
    return "[" + g_current_task_id + "]";
}

void TaskManagementService::set_current_task_id(const std::string& task_id)
{
    g_current_task_id = task_id;
}

void TaskManagementService::clear_current_task_id()
{
    g_current_task_id.clear();
}
