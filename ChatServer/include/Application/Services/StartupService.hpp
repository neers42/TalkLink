#ifndef STARTUPSERVICE_HPP
#define STARTUPSERVICE_HPP

#include "Utils/common.hpp"
#include <condition_variable>
#define SOCKET_CONFIG_FILE_PATH "/app/ChatServer/conf/socket_setting.conf"

// Forward declarations
class SocketHandler;
class ThreadPoolHandler;
class TLSConnectionHandler;
class TaskManagementService;
class RequestDispatcherService;
class UserManagementService;
class ChatService;
class ChatroomManagementService;
class SyncService;

// サーバー起動フェーズ定義
enum class ServerStartupPhase {
    NOT_STARTED,
    SOCKET_INITIALIZATION,
    BACKGROUND_SERVICES,
    REQUEST_CONFIGURATION,
    MONITORING_START,
    COMPLETED,
    FAILED
};

// システムリソース状態
struct SystemResourceStatus {
    bool tcp_socket_ready;
    bool udp_socket_ready;
    bool background_tasks_running;
    bool request_dispatcher_configured;
    uint32_t active_connections;
    double cpu_usage_percent;
    double memory_usage_percent;
    std::chrono::steady_clock::time_point startup_time;
    std::chrono::milliseconds total_startup_duration;
};

// 起動設定パラメータ
struct ServerStartupConfig {
    std::chrono::milliseconds socket_timeout{5000};       // 5秒
    std::chrono::milliseconds service_init_timeout{10000}; // 10秒
    std::chrono::milliseconds total_timeout{30000};       // 30秒
    bool enable_overload_protection{true};
    bool enable_health_monitoring{true};
    uint32_t max_concurrent_connections{1000};
    std::chrono::seconds health_check_interval{30};
};

/**
 * @brief サーバーのスタートアップシーケンスを管理するサービス
 */
class StartupService : public std::enable_shared_from_this<StartupService> {
public:
    StartupService(
        std::shared_ptr<SocketHandler> socket_handler,
        std::shared_ptr<ThreadPoolHandler> thread_pool_handler,
        std::shared_ptr<TLSConnectionHandler> protocol_handler,
        std::shared_ptr<TaskManagementService> task_management_service,
        std::shared_ptr<RequestDispatcherService> request_dispatcher_service,
        std::shared_ptr<UserManagementService> user_management_service,
        std::shared_ptr<ChatService> chat_service,
        std::shared_ptr<ChatroomManagementService> chatroom_management_service,
        std::shared_ptr<SyncService> sync_service,
        std::string cert_path,
        std::string key_path
    );
    ~StartupService();

    // ==================== サーバー起動制御 ====================
    
    /**
     * @brief サーバー全体のスタートアップシーケンス実行
     * @return 起動成功の可否
     */
    bool execute_server_startup();
    
    /**
     * @brief カスタム設定でサーバー起動
     * @param config 起動設定パラメータ
     * @return 起動成功の可否
     */
    bool execute_server_startup_with_config(const ServerStartupConfig& config);
    
    /**
     * @brief サーバーのグレースフルシャットダウン
     * @return シャットダウン成功の可否
     */
    bool shutdown_server();

    // ==================== 段階的起動制御 ====================
    
    /**
     * @brief TCPリスニングソケットの初期化
     * @return 初期化成功の可否
     */
    bool initialize_tcp_socket();
    
    /**
     * @brief UDPマルチキャストソケットの初期化
     * @return 初期化成功の可否
     */
    bool initialize_udp_socket();
    
    /**
     * @brief バックグラウンドサービスの起動
     * @return 起動成功の可否
     */
    bool start_background_services();
    
    /**
     * @brief リクエストディスパッチャの設定
     * @return 設定成功の可否
     */
    bool configure_request_dispatching();

    // ==================== システム監視 ====================
    
    /**
     * @brief 現在の起動フェーズを取得
     * @return 現在のフェーズ
     */
    ServerStartupPhase get_startup_phase() const;
    
    /**
     * @brief システムリソース状態を取得
     * @return リソース状態の詳細
     */
    SystemResourceStatus get_system_status() const;
    
    /**
     * @brief サーバーが完全に起動済みかチェック
     * @return 起動完了の可否
     */
    bool is_server_ready() const;
    
    /**
     * @brief システムヘルス状態をチェック
     * @return ヘルス状態（正常：true、異常：false）
     */
    bool is_system_healthy() const;

    // ==================== 設定管理 ====================
    
    /**
     * @brief 起動設定を更新
     * @param config 新しい設定
     */
    void update_startup_config(const ServerStartupConfig& config);
    
    /**
     * @brief 現在の起動設定を取得
     * @return 現在の設定
     */
    const ServerStartupConfig& get_startup_config() const;

private:
    // Injected dependencies
    std::shared_ptr<SocketHandler> _socket_handler;
    std::shared_ptr<ThreadPoolHandler> _thread_pool_handler;
    std::shared_ptr<TLSConnectionHandler> _tlscon_handler;
    std::shared_ptr<TaskManagementService> _task_management_service;
    std::shared_ptr<RequestDispatcherService> _request_dispatcher_service;
    std::shared_ptr<UserManagementService> _user_management_service;
    std::shared_ptr<ChatService> _chat_service;
    std::shared_ptr<ChatroomManagementService> _chatroom_management_service;
    std::shared_ptr<SyncService> _sync_service;
    std::string _cert_path; // 環境変数TLS_CERT_PATHから取得した証明書パス
    std::string _key_path;  // 環境変数TLS_KEY_PATHから取得した秘密鍵パス
    // 起動状態管理
    std::atomic<ServerStartupPhase> _current_phase{ServerStartupPhase::NOT_STARTED};
    mutable std::mutex _phase_mutex;
    
    // 起動設定
    ServerStartupConfig _startup_config;
    mutable std::mutex _config_mutex;
    
    // システム状態
    std::atomic<bool> _tcp_socket_ready{false};
    std::atomic<bool> _udp_socket_ready{false};
    std::atomic<bool> _background_services_running{false};
    std::atomic<bool> _request_dispatcher_configured{false};
    std::atomic<bool> _server_shutdown_requested{false};
    
    // タイムスタンプ管理
    std::chrono::steady_clock::time_point _startup_begin_time;
    std::chrono::steady_clock::time_point _startup_complete_time;
    
    // 内部ヘルパーメソッド
    void set_startup_phase(ServerStartupPhase new_phase);
    bool wait_for_phase_completion(ServerStartupPhase phase, std::chrono::milliseconds timeout);
    void record_startup_metrics();
    void cleanup_resources();

    // ヘルス監視
    void start_health_monitoring();
    void stop_health_monitoring();
    void run_health_monitoring_loop();
    std::atomic<bool> _health_monitoring_active{false};
    std::mutex _health_monitor_mutex;
    std::condition_variable _health_monitor_cv;

    // システムリソース計測(Linux /proc ベース)
    double calculate_cpu_usage_percent() const;
    double calculate_memory_usage_percent() const;
    mutable std::mutex _cpu_sample_mutex;
    mutable uint64_t _prev_cpu_total_jiffies{0};
    mutable uint64_t _prev_cpu_idle_jiffies{0};
    mutable bool _has_cpu_sample{false};
};

#endif // STARTUPSERVICE_HPP