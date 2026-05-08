#ifndef TASKMANAGEMENTSERVICE_HPP
#define TASKMANAGEMENTSERVICE_HPP
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

class TaskManagementService {
private:
    std::shared_ptr<ThreadPoolHandler> _thread_pool;
    
    // タスク監視用の内部状態
    std::atomic<uint64_t> _task_counter{0};
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> _active_tasks;
    mutable std::mutex _active_tasks_mutex;
    
    // メトリクス
    std::atomic<uint64_t> _total_tasks_executed{0};
    std::atomic<uint64_t> _failed_tasks{0};
    std::atomic<uint64_t> _timeout_tasks{0};
    
    // リソース管理
    std::atomic<bool> _overload_protection{false};
    std::chrono::milliseconds _overload_threshold{100}; // タスク平均実行時間(ms)
    
public:
    explicit TaskManagementService(std::shared_ptr<ThreadPoolHandler> thread_pool);
    ~TaskManagementService();
    
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
        auto managed_task = ManagedTask(metadata, 
                                       [this, task_id, bound_task]() {
                                           execute_managed_task(task_id, bound_task);
                                       }, timeout);
        
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
    
    // ==================== スレッドローカルタスク管理 ====================
    
    // 現在のスレッドで実行中のタスクIDを取得
    static std::string get_current_task_id();
    
    // ログ出力用のタスクIDフォーマット取得（"[task_xxx]"形式）
    static std::string get_current_task_id_format();
    
private:
    // タスク実行ラッパー
    void execute_managed_task(const std::string& task_id, std::function<void()> task);
    
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

#endif // TASKMANAGEMENTSERVICE_HPP