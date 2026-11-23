#ifndef TASKSCHEDULER_HPP
#define TASKSCHEDULER_HPP
#include "Utils/common.hpp"

// Task priority levels
enum class TaskPriority {
    CRITICAL = 0,    // システムクリティカル（ハートビート、認証）
    HIGH = 1,        // ユーザーインタラクション（メッセージ送信）
    NORMAL = 2,      // バックグラウンド処理（ログ書き込み）
    LOW = 3          // メンテナンス（クリーンアップ、監視）
};

// Task category for business workflow management
enum class TaskCategory {
    AUTHENTICATION,  // ログイン/ログアウト処理
    USER_MANAGEMENT, // セッション管理、接続処理
    CHAT_PROCESSING, // メッセージルーティング、配信
    HEARTBEAT,      // ヘルスモニタリング
    MAINTENANCE,    // クリーンアップ、証明書リロード
    SYSTEM         // 一般的なシステムタスク
};

// タスクメタデータ構造
struct TaskMetadata {
    std::string task_id;    // 一意のタスク識別子
    TaskPriority priority;  // タスクの優先度
    TaskCategory category;  // タスクのカテゴリ
    std::chrono::steady_clock::time_point created_at; // 作成時刻
    std::chrono::steady_clock::time_point scheduled_at; // スケジュール時刻
    std::string description; // タスクの説明
    socket_t associated_socket{INVALID_SOCKET_FD}; // ユーザー関連タスク用
    std::chrono::milliseconds max_execution_time{30000}; // 最大実行時間
    
    TaskMetadata(const std::string& id, TaskPriority prio, TaskCategory cat, const std::string& desc,
                 std::chrono::milliseconds timeout = std::chrono::milliseconds(30000))
        : task_id(id), priority(prio), category(cat), 
          created_at(std::chrono::steady_clock::now()),
          scheduled_at(created_at), description(desc), max_execution_time(timeout) {}
};

// タスク実行ラッパー
struct ManagedTask {
    TaskMetadata metadata;
    std::function<void()> task_function;
    std::chrono::milliseconds max_execution_time{30000}; // 30秒のデフォルトタイムアウト
    
    ManagedTask(const TaskMetadata& meta, std::function<void()> func, 
                std::chrono::milliseconds timeout = std::chrono::milliseconds(30000))
        : metadata(meta), task_function(std::move(func)), max_execution_time(timeout) {}
};

// アクティブタスク1件分の監視情報
struct ActiveTaskInfo {
    std::chrono::steady_clock::time_point start_time;
    std::chrono::milliseconds max_execution_time;
};

// 過負荷保護の設定パラメータ
struct OverloadProtectionConfig {
    size_t max_active_tasks{100};        // この値を超えたアクティブタスク数で過負荷と判定する
    size_t active_task_hard_limit{150};  // 過負荷中にこの値を超えたら新規タスクの実行をスキップする
    double overload_recovery_ratio{0.7}; // max_active_tasks×この比率を下回ったら過負荷状態を解除する
};

class TaskScheduler {
private:
    // カテゴリごとに物理的に分離したスレッドプール。
    // 1つのプールを優先度で並び替えるより、用途ごとにプールを分けた方が
    // 「他の処理の渋滞でハートビートが止まる」といった飢餓状態を避けやすい。
    std::shared_ptr<ThreadPoolHandler> _heartbeat_pool;  // HEARTBEAT
    std::shared_ptr<ThreadPoolHandler> _realtime_pool;   // AUTHENTICATION, USER_MANAGEMENT, CHAT_PROCESSING
    std::shared_ptr<ThreadPoolHandler> _background_pool; // MAINTENANCE, SYSTEM

    std::shared_ptr<ThreadPoolHandler>& select_pool_for_category(TaskCategory category);

    // タスク監視用の内部状態
    std::atomic<uint64_t> _task_counter{0};
    std::unordered_map<std::string, ActiveTaskInfo> _active_tasks;
    mutable std::mutex _active_tasks_mutex;
    
    // メトリクス
    std::atomic<uint64_t> _total_tasks_executed{0};
    std::atomic<uint64_t> _failed_tasks{0};
    std::atomic<uint64_t> _timeout_tasks{0};
    
    // リソース管理
    std::atomic<bool> _overload_protection{false};        // 現在過負荷状態かどうか
    std::atomic<bool> _overload_protection_enabled{false}; // 過負荷監視機能自体の有効/無効
    std::chrono::milliseconds _overload_threshold{100}; // タスク平均実行時間(ms)
    OverloadProtectionConfig _overload_config;
    mutable std::mutex _overload_config_mutex;

public:
    TaskScheduler(
        std::shared_ptr<ThreadPoolHandler> heartbeat_pool,
        std::shared_ptr<ThreadPoolHandler> realtime_pool,
        std::shared_ptr<ThreadPoolHandler> background_pool
    );
    ~TaskScheduler();
    
    // ==================== 優先度ベースのタスク実行 ====================
    
    // タスクを優先度とメタデータでスケジュールする
    template<typename F, typename... Args>
    std::string schedule_task(TaskPriority priority, TaskCategory category, 
                             const std::string& description, F&& func, Args&&... args) {
        return schedule_task_with_timeout(priority, category, description, 
                                        std::chrono::milliseconds(30000), 
                                        std::forward<F>(func), std::forward<Args>(args)...);
    }
    
    // カスタムタイムアウトでタスクをスケジュールする
    template<typename F, typename... Args>
    std::string schedule_task_with_timeout(TaskPriority priority, TaskCategory category,
                                          const std::string& description,
                                          std::chrono::milliseconds timeout,
                                          F&& func, Args&&... args) {
        auto task_id = generate_task_id();
        auto metadata = TaskMetadata(task_id, priority, category, description);
        
        auto bound_task = std::bind(std::forward<F>(func), std::forward<Args>(args)...);
        // task_functionはそのままbound_taskを持たせる(enqueue_by_priorityが
        // execute_managed_taskを呼ぶため、ここでさらにラップすると
        // execute_managed_taskが二重に呼ばれ、メトリクスが二重計上されてしまう)
        auto managed_task = ManagedTask(metadata, bound_task, timeout);

        enqueue_by_priority(std::move(managed_task));
        return task_id;
    }
    
    // ==================== リソース & モニタリング制御 ====================
    
    // 特定のタスクをキャンセル（まだ実行されていない場合）
    bool cancel_task(const std::string& task_id);
    
    // タスク実行メトリクスを取得
    struct TaskMetrics {
        uint64_t total_executed;    // 総実行タスク数
        uint64_t failed_tasks;   // 失敗したタスク数
        uint64_t timeout_tasks;  // タイムアウトしたタスク数
        uint64_t active_count;  // 現在アクティブなタスク数
        double average_execution_time;  // 平均実行時間(ms)
        bool overload_status;   // 過負荷保護の状態
    };
    TaskMetrics get_task_metrics() const;
    
    // ユーザー固有のタスク管理
    void cancel_user_tasks(socket_t socket);
    uint32_t get_user_task_count(socket_t socket) const;
    
    // システムヘルスモニタリングと過負荷保護
    void enable_overload_protection(bool enabled = true);
    bool is_overloaded() const;

    // 過負荷保護の設定(しきい値等)を更新/取得する
    void update_overload_config(const OverloadProtectionConfig& config);
    OverloadProtectionConfig get_overload_config() const;
    
    // ==================== スレッドローカルタスク管理 ====================
    
    // 現在のスレッドで実行中のタスクIDを取得
    static std::string get_current_task_id();
    
    // ログ出力用のタスクIDフォーマット取得（"[task_xxx]"形式）
    static std::string get_current_task_id_format();
    
private:
    // タスク実行ラッパー
    void execute_managed_task(const std::string& task_id, std::function<void()> task,
                               std::chrono::milliseconds max_execution_time);
    
    // 優先度キュー管理
    void enqueue_by_priority(ManagedTask managed_task);
    
    // ユーティリティ関数
    std::string generate_task_id();
    void update_metrics(const TaskMetadata& metadata, bool success, 
                       std::chrono::milliseconds execution_time);
    
    // システムヘルスモニタリングと過負荷保護
    void check_system_overload();
    
    // スレッドローカルタスク管理
    static void set_current_task_id(const std::string& task_id);
    static void clear_current_task_id();
};

#endif // TASKSCHEDULER_HPP