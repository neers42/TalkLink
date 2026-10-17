#include "Application/Services/StartupService.hpp"
#include "Application/Services/TaskManagementService.hpp"
#include "Application/Services/RequestDispatcherService.hpp"
#include "Application/Services/UserManagementService.hpp"
#include "Application/Services/ChatService.hpp"
#include "Application/Services/ChatroomManagementService.hpp"
#include "Application/Services/SyncService.hpp"

StartupService::StartupService(
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
    std::string key_path)
    : _socket_handler(std::move(socket_handler)),
      _thread_pool_handler(std::move(thread_pool_handler)),
      _tlscon_handler(std::move(protocol_handler)),
      _task_management_service(std::move(task_management_service)),
      _request_dispatcher_service(std::move(request_dispatcher_service)),
      _user_management_service(std::move(user_management_service)),
      _chat_service(std::move(chat_service)),
      _chatroom_management_service(std::move(chatroom_management_service)),
      _sync_service(std::move(sync_service)),
      _cert_path(std::move(cert_path)),
      _key_path(std::move(key_path))
{
    if (!_socket_handler || !_thread_pool_handler || !_tlscon_handler ||
        !_task_management_service || !_request_dispatcher_service || !_user_management_service || !_chat_service || !_chatroom_management_service || !_sync_service) {
        throw std::runtime_error("Failed to initialize StartupService dependencies");
    }
    
    LOG_INFO("[StartupService] StartupService initialized with injected dependencies");
    _startup_begin_time = std::chrono::steady_clock::now();
}

StartupService::~StartupService()
{
    if (_health_monitoring_active) {
        stop_health_monitoring();
    }
    cleanup_resources();
    LOG_INFO("[StartupService] StartupService shutdown.");
}

// ==================== サーバー起動制御 ====================

bool StartupService::execute_server_startup()
{
    return execute_server_startup_with_config(_startup_config);
}

bool StartupService::execute_server_startup_with_config(const ServerStartupConfig& config)
{
    std::lock_guard<std::mutex> lock(_phase_mutex);
    
    if (_current_phase != ServerStartupPhase::NOT_STARTED) {
        LOG_WARNING("[StartupService] Server startup already in progress or completed");
        return _current_phase == ServerStartupPhase::COMPLETED;
    }
    
    _startup_config = config;
    _startup_begin_time = std::chrono::steady_clock::now();
    
    LOG_INFO("[StartupService] Starting server startup sequence with timeout: {}ms", config.total_timeout.count());
    
    try {
        // フェーズ1: ソケット初期化
        set_startup_phase(ServerStartupPhase::SOCKET_INITIALIZATION);
        if (!initialize_tcp_socket()) {
            LOG_ERROR("[StartupService] TCP socket initialization failed");
            set_startup_phase(ServerStartupPhase::FAILED);
            return false;
        }
        
        if (!initialize_udp_socket()) {
            LOG_ERROR("[StartupService] UDP socket initialization failed");
            set_startup_phase(ServerStartupPhase::FAILED);
            return false;
        }
        
        // フェーズ2: バックグラウンドサービス起動
        set_startup_phase(ServerStartupPhase::BACKGROUND_SERVICES);
        if (!start_background_services()) {
            LOG_ERROR("[StartupService] Background services startup failed");
            set_startup_phase(ServerStartupPhase::FAILED);
            return false;
        }
        
        // フェーズ3: リクエストディスパッチャ設定
        set_startup_phase(ServerStartupPhase::REQUEST_CONFIGURATION);
        if (!configure_request_dispatching()) {
            LOG_ERROR("[StartupService] Request dispatcher configuration failed");
            set_startup_phase(ServerStartupPhase::FAILED);
            return false;
        }
        
        // フェーズ4: 監視開始
        set_startup_phase(ServerStartupPhase::MONITORING_START);
        if (config.enable_health_monitoring) {
            start_health_monitoring();
        }
        
        // 完了
        set_startup_phase(ServerStartupPhase::COMPLETED);
        _startup_complete_time = std::chrono::steady_clock::now();
        record_startup_metrics();
        
        LOG_INFO("[StartupService] Server startup sequence completed successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[StartupService] Exception during server startup: {}", e.what());
        set_startup_phase(ServerStartupPhase::FAILED);
        return false;
    }
}

bool StartupService::shutdown_server()
{
    LOG_INFO("[StartupService] Initiating graceful server shutdown");
    _server_shutdown_requested = true;
    
    // ヘルス監視停止
    stop_health_monitoring();
    
    // リソースクリーンアップ
    cleanup_resources();
    
    LOG_INFO("[StartupService] Server shutdown completed");
    return true;
}

// ==================== 段階的起動制御 ====================

bool StartupService::initialize_tcp_socket()
{
    LOG_INFO("[StartupService] Initializing TCP listening socket");
    
    try {
        auto socket_handler = _socket_handler;
        if (!socket_handler) {
            LOG_ERROR("[StartupService] SocketHandler not available");
            return false;
        }
        this->_tcp_socket_ready = socket_handler->open_tcp_listen_socket();
        
        if (this->_tcp_socket_ready == true) {
            LOG_INFO("[StartupService] TCP listening socket initialized successfully");
        } else {
            LOG_ERROR("[StartupService] Failed to open TCP listening socket");
        }
        
        return this->_tcp_socket_ready;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[StartupService] Exception during TCP socket initialization: {}", e.what());
        _tcp_socket_ready = false;
        return this->_tcp_socket_ready;
    }
}

bool StartupService::initialize_udp_socket()
{
    LOG_INFO("[StartupService] Initializing UDP multicast socket");
    
    try {
        auto socket_handler = _socket_handler;
        if (!socket_handler) {
            LOG_ERROR("[StartupService] SocketHandler not available");
            return false;
        }
        
        _udp_socket_ready = socket_handler->open_udp_multicast_tx_socket();
        
        if (_udp_socket_ready == true) {
            LOG_INFO("[StartupService] UDP multicast socket initialized successfully");
        } else {
            LOG_ERROR("[StartupService] Failed to open UDP multicast socket");
        }
        
        return _udp_socket_ready;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[StartupService] Exception during UDP socket initialization: {}", e.what());
        _udp_socket_ready = false;
        return _udp_socket_ready;
    }
}

bool StartupService::start_background_services()
{
    LOG_INFO("[StartupService] Starting background services");
    
    try {
        auto thread_pool_handler = _thread_pool_handler;
        auto protocol_handler = _tlscon_handler;
        auto user_mgmt = _user_management_service;

        if (!thread_pool_handler || !protocol_handler || !user_mgmt) {
            LOG_ERROR("[StartupService] Required handlers not available");
            return false;
        }

        // ハートビートチェックタスクを開始(非アクティブ/未認証接続の定期クリーンアップ)
        auto task_mgmt = _task_management_service;
        if (task_mgmt) {
            task_mgmt->schedule_task(
                TaskPriority::CRITICAL,
                TaskCategory::HEARTBEAT,
                "Heartbeat monitoring task",
                [user_mgmt]() {
                    user_mgmt->start_heartbeat_monitoring();
                }
            );
        }
        
        // 証明書リロードタスクを開始
        if (task_mgmt) {
            task_mgmt->schedule_task(
                TaskPriority::LOW,
                TaskCategory::MAINTENANCE,
                "Certificate hot reload task",
                [protocol_handler, cert_path = _cert_path, key_path = _key_path]() {
                    protocol_handler->hot_reload_certificates(60, cert_path, key_path);
                }
            );
        }
        
        _background_services_running = true;
        LOG_INFO("[StartupService] Background services started successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[StartupService] Exception during background services startup: {}", e.what());
        _background_services_running = false;
        return false;
    }
}

bool StartupService::configure_request_dispatching()
{
    LOG_INFO("[StartupService] Configuring request dispatching");
    
    try {
        auto dispatcher = _request_dispatcher_service;
        auto user_mgmt = _user_management_service;
        
        if (!dispatcher || !user_mgmt) {
            LOG_ERROR("[StartupService] Required services not available");
            return false;
        }
        
        // Accept callback設定
        // accept()/SSL_accept()はブロッキング呼び出しのため、ディスパッチャースレッド上で
        // 直接実行すると、ハンドシェイクが滞留した1接続が全体の処理を止めてしまう。
        // スレッドプール側のタスクとして実行することで、他の接続への影響を防ぐ。
        auto task_mgmt = _task_management_service;
        dispatcher->set_accept_callback([task_mgmt, user_mgmt]() {
            task_mgmt->schedule_task(
                TaskPriority::HIGH,
                TaskCategory::USER_MANAGEMENT,
                "Accept new connection",
                [user_mgmt]() {
                    user_mgmt->process_accept_new_user();
                }
            );
        });
        
        // Message handle callback設定 - TaskManagementService経由でマルチスレッド処理
        dispatcher->set_message_handle_callback([this](const MessageDispatchResult& result) {
            LOG_DEBUG("[RequestDispatcherService] Message dispatch callback triggered - socket: {}, type: {}, success: {}", 
                     result.socket, static_cast<int>(result.type), result.success);
            
            // メッセージタイプに応じて適切な優先度でタスクスケジューリング
            switch (result.type) {
                case MessageDispatchResult::Type::HEARTBEAT:
                    _task_management_service->schedule_task(
                        TaskPriority::CRITICAL,
                        TaskCategory::HEARTBEAT,
                        "Process user heartbeat",
                        [this, socket = result.socket]() {
                            _user_management_service->process_heartbeat(socket);
                        }
                    );
                    break;
                    
                case MessageDispatchResult::Type::SESSION_REQUEST:
                    _task_management_service->schedule_task(
                        TaskPriority::HIGH,
                        TaskCategory::AUTHENTICATION,
                        "Process session request",
                        [this, message_data = result.message_data, socket = result.socket]() {
                            _user_management_service->process_session_request(message_data, socket);
                        }
                    );
                    break;
                    
                case MessageDispatchResult::Type::CHAT_MESSAGE:
                    _task_management_service->schedule_task(
                        TaskPriority::NORMAL,
                        TaskCategory::CHAT_PROCESSING,
                        "Process chat message",
                        [this, message_data = result.message_data, socket = result.socket]() {
                            _chat_service->process_chat_message(message_data, socket);
                        }
                    );
                    break;
                    
                case MessageDispatchResult::Type::CHATROOM_CONTROL:
                    _task_management_service->schedule_task(
                        TaskPriority::HIGH,
                        TaskCategory::CHAT_PROCESSING,
                        "Process chatroom control",
                        [this, message_data = result.message_data, socket = result.socket]() {
                            _chatroom_management_service->process_chatroom_operation(message_data);
                        }
                        
                    );
                    break;
                    
                case MessageDispatchResult::Type::DISCONNECT:
                    _task_management_service->schedule_task(
                        TaskPriority::CRITICAL,
                        TaskCategory::USER_MANAGEMENT,
                        "Process user disconnection",
                        [this, socket = result.socket]() {
                            _user_management_service->process_user_disconnection(socket);
                        }
                    );
                    break;

                case MessageDispatchResult::Type::USER_SETTING_CONTROL:
                    //ToDo: USER_SETTING_CONTROLの処理を実装
                    break;

                case MessageDispatchResult::Type::SYNC_REQUEST:
                    _task_management_service->schedule_task(
                        TaskPriority::NORMAL,
                        TaskCategory::CHAT_PROCESSING,
                        "Process sync request",
                        [this, message_data = result.message_data, socket = result.socket]() {
                            _sync_service->process_sync_request(message_data, socket);
                        }
                    );
                    break;

                case MessageDispatchResult::Type::USER_SEARCH_REQUEST:
                    _task_management_service->schedule_task(
                        TaskPriority::NORMAL,
                        TaskCategory::USER_MANAGEMENT,
                        "Process user search request",
                        [this, message_data = result.message_data, socket = result.socket]() {
                            _user_management_service->process_user_search(message_data, socket);
                        }
                    );
                    break;
                    
                default:
                    LOG_ERROR("[RequestDispatcherService] Unknown message type ({}) or error occurred for socket: {}", 
                             static_cast<int>(result.type), result.socket);
                    if (!result.success) {
                        LOG_ERROR("[RequestDispatcherService] Error message: {}", result.error_message);
                    }
                    break;
            }
        });
        
        _request_dispatcher_configured = true;
        LOG_INFO("[StartupService] Request dispatcher configured successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[StartupService] Exception during request dispatcher configuration: {}", e.what());
        _request_dispatcher_configured = false;
        return false;
    }
}

// ==================== システム監視 ====================

ServerStartupPhase StartupService::get_startup_phase() const
{
    return _current_phase.load();
}

SystemResourceStatus StartupService::get_system_status() const
{
    SystemResourceStatus status;
    status.tcp_socket_ready = _tcp_socket_ready.load();
    status.udp_socket_ready = _udp_socket_ready.load();
    status.background_tasks_running = _background_services_running.load();
    status.request_dispatcher_configured = _request_dispatcher_configured.load();
    status.active_connections = 0; // TODO: UserManagementServiceから取得
    status.cpu_usage_percent = 0.0; // TODO: システム監視実装
    status.memory_usage_percent = 0.0; // TODO: システム監視実装
    status.startup_time = _startup_begin_time;
    
    if (_current_phase == ServerStartupPhase::COMPLETED) {
        status.total_startup_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            _startup_complete_time - _startup_begin_time);
    } else {
        auto now = std::chrono::steady_clock::now();
        status.total_startup_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - _startup_begin_time);
    }
    
    return status;
}

bool StartupService::is_server_ready() const
{
    return _current_phase.load() == ServerStartupPhase::COMPLETED;
}

bool StartupService::is_system_healthy() const
{
    return is_server_ready() && 
           _tcp_socket_ready.load() && 
           _udp_socket_ready.load() && 
           _background_services_running.load() && 
           _request_dispatcher_configured.load() &&
           !_server_shutdown_requested.load();
}

// ==================== 設定管理 ====================

void StartupService::update_startup_config(const ServerStartupConfig& config)
{
    std::lock_guard<std::mutex> lock(_config_mutex);
    _startup_config = config;
    LOG_INFO("[StartupService] Startup configuration updated");
}

const ServerStartupConfig& StartupService::get_startup_config() const
{
    std::lock_guard<std::mutex> lock(_config_mutex);
    return _startup_config;
}

// ==================== 内部ヘルパーメソッド ====================

void StartupService::set_startup_phase(ServerStartupPhase new_phase)
{
    _current_phase = new_phase;
    LOG_DEBUG("[StartupService] Startup phase changed to: {}", static_cast<int>(new_phase));
}

void StartupService::record_startup_metrics()
{
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        _startup_complete_time - _startup_begin_time);
    LOG_INFO("[StartupService] Server startup completed in {}ms", duration.count());
}

void StartupService::cleanup_resources()
{
    LOG_DEBUG("[StartupService] Cleaning up resources");
    _tcp_socket_ready = false;
    _udp_socket_ready = false;
    _background_services_running = false;
    _request_dispatcher_configured = false;
}

void StartupService::start_health_monitoring()
{
    if (!_health_monitoring_active.exchange(true)) {
        LOG_INFO("[StartupService] Health monitoring started");
        // TODO: ヘルス監視タスクの実装
    }
}

void StartupService::stop_health_monitoring()
{
    if (_health_monitoring_active.exchange(false)) {
        LOG_INFO("[StartupService] Health monitoring stopped");
    }
}