#include "Application/Runtime/ServerBootstrapper.hpp"
#include "Application/Runtime/TaskScheduler.hpp"
#include "Application/Runtime/RequestDispatcher.hpp"
#include "Application/UseCases/UserSessionUseCase.hpp"
#include "Application/UseCases/ChatUseCase.hpp"
#include "Application/UseCases/ChatroomUseCase.hpp"
#include "Application/UseCases/SyncUseCase.hpp"
#include "Application/Runtime/SystemMonitor.hpp"

namespace {
// ハートビート監視など、意図的に常駐し続けるバックグラウンドループタスク用の
// max_execution_time。デフォルトの30秒を使うと、TaskSchedulerの
// 過負荷監視(check_system_overload)から「長時間実行タスク」として
// 常に警告されてしまうため、実質無期限とみなせる十分大きな値を設定する。
constexpr auto kPermanentTaskTimeout = std::chrono::hours(24 * 365);
}

ServerBootstrapper::ServerBootstrapper(
    std::shared_ptr<SocketHandler> socket_handler,
    std::shared_ptr<ThreadPoolHandler> thread_pool_handler,
    std::shared_ptr<TLSConnectionHandler> tlscon_handler,
    std::shared_ptr<TaskScheduler> task_scheduler,
    std::shared_ptr<RequestDispatcher> request_dispatcher,
    std::shared_ptr<UserSessionUseCase> user_session_use_case,
    std::shared_ptr<ChatUseCase> chat_use_case,
    std::shared_ptr<ChatroomUseCase> chatroom_use_case,
    std::shared_ptr<SyncUseCase> sync_use_case,
    std::shared_ptr<SystemMonitor> system_monitor,
    std::string cert_path,
    std::string key_path)
    : _socket_handler(std::move(socket_handler)),
      _thread_pool_handler(std::move(thread_pool_handler)),
      _tlscon_handler(std::move(tlscon_handler)),
      _task_scheduler(std::move(task_scheduler)),
      _request_dispatcher(std::move(request_dispatcher)),
      _user_session_use_case(std::move(user_session_use_case)),
      _chat_use_case(std::move(chat_use_case)),
      _chatroom_use_case(std::move(chatroom_use_case)),
      _sync_use_case(std::move(sync_use_case)),
      _system_monitor(std::move(system_monitor)),
      _cert_path(std::move(cert_path)),
      _key_path(std::move(key_path))
{
    if (!_socket_handler || !_thread_pool_handler || !_tlscon_handler ||
        !_task_scheduler || !_request_dispatcher || !_user_session_use_case || !_chat_use_case || !_chatroom_use_case || !_sync_use_case || !_system_monitor) {
        throw std::runtime_error("Failed to initialize ServerBootstrapper dependencies");
    }
    
    LOG_INFO("[ServerBootstrapper] ServerBootstrapper initialized with injected dependencies");
    _startup_begin_time = std::chrono::steady_clock::now();
}

ServerBootstrapper::~ServerBootstrapper()
{
    if (_system_monitor) {
        _system_monitor->stop_monitoring();
    }
    cleanup_resources();
    LOG_INFO("[ServerBootstrapper] ServerBootstrapper shutdown.");
}

// ==================== サーバー起動制御 ====================

bool ServerBootstrapper::execute_server_startup()
{
    return execute_server_startup_with_config(_startup_config);
}

bool ServerBootstrapper::execute_server_startup_with_config(const ServerStartupConfig& config)
{
    std::lock_guard<std::mutex> lock(_phase_mutex);
    
    if (_current_phase != ServerStartupPhase::NOT_STARTED) {
        LOG_WARNING("[ServerBootstrapper] Server startup already in progress or completed");
        return _current_phase == ServerStartupPhase::COMPLETED;
    }
    
    _startup_config = config;
    _startup_begin_time = std::chrono::steady_clock::now();
    
    LOG_INFO("[ServerBootstrapper] Starting server startup sequence with timeout: {}ms", config.total_timeout.count());
    
    try {
        // フェーズ1: ソケット初期化
        set_startup_phase(ServerStartupPhase::SOCKET_INITIALIZATION);
        auto phase_start = std::chrono::steady_clock::now();
        if (!initialize_tcp_socket()) {
            LOG_ERROR("[ServerBootstrapper] TCP socket initialization failed");
            set_startup_phase(ServerStartupPhase::FAILED);
            return false;
        }

        if (!initialize_udp_socket()) {
            LOG_ERROR("[ServerBootstrapper] UDP socket initialization failed");
            set_startup_phase(ServerStartupPhase::FAILED);
            return false;
        }

        if (!enforce_phase_timeout(phase_start, config.socket_timeout, "SOCKET_INITIALIZATION") ||
            !enforce_total_timeout(config)) {
            set_startup_phase(ServerStartupPhase::FAILED);
            return false;
        }

        // フェーズ2: バックグラウンドサービス起動
        set_startup_phase(ServerStartupPhase::BACKGROUND_SERVICES);
        phase_start = std::chrono::steady_clock::now();
        if (!start_background_services()) {
            LOG_ERROR("[ServerBootstrapper] Background services startup failed");
            set_startup_phase(ServerStartupPhase::FAILED);
            return false;
        }

        if (!enforce_phase_timeout(phase_start, config.service_init_timeout, "BACKGROUND_SERVICES") ||
            !enforce_total_timeout(config)) {
            set_startup_phase(ServerStartupPhase::FAILED);
            return false;
        }

        // フェーズ3: リクエストディスパッチャ設定
        set_startup_phase(ServerStartupPhase::REQUEST_CONFIGURATION);
        if (!configure_request_dispatching()) {
            LOG_ERROR("[ServerBootstrapper] Request dispatcher configuration failed");
            set_startup_phase(ServerStartupPhase::FAILED);
            return false;
        }

        if (!enforce_total_timeout(config)) {
            set_startup_phase(ServerStartupPhase::FAILED);
            return false;
        }

        // フェーズ4: 監視開始
        set_startup_phase(ServerStartupPhase::MONITORING_START);
        if (config.enable_health_monitoring && _system_monitor) {
            _system_monitor->start_monitoring();
        }
        if (config.enable_overload_protection && _task_scheduler) {
            _task_scheduler->enable_overload_protection(true);
        }
        
        // 完了
        set_startup_phase(ServerStartupPhase::COMPLETED);
        _startup_complete_time = std::chrono::steady_clock::now();
        record_startup_metrics();
        
        LOG_INFO("[ServerBootstrapper] Server startup sequence completed successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[ServerBootstrapper] Exception during server startup: {}", e.what());
        set_startup_phase(ServerStartupPhase::FAILED);
        return false;
    }
}

bool ServerBootstrapper::shutdown_server()
{
    LOG_INFO("[ServerBootstrapper] Initiating graceful server shutdown");
    _server_shutdown_requested = true;

    // ヘルス監視停止
    if (_system_monitor) {
        _system_monitor->stop_monitoring();
    }

    // リソースクリーンアップ
    cleanup_resources();
    
    LOG_INFO("[ServerBootstrapper] Server shutdown completed");
    return true;
}

// ==================== 段階的起動制御 ====================

bool ServerBootstrapper::initialize_tcp_socket()
{
    LOG_INFO("[ServerBootstrapper] Initializing TCP listening socket");
    
    try {
        auto socket_handler = _socket_handler;
        if (!socket_handler) {
            LOG_ERROR("[ServerBootstrapper] SocketHandler not available");
            return false;
        }
        this->_tcp_socket_ready = socket_handler->open_tcp_listen_socket();
        
        if (this->_tcp_socket_ready == true) {
            LOG_INFO("[ServerBootstrapper] TCP listening socket initialized successfully");
        } else {
            LOG_ERROR("[ServerBootstrapper] Failed to open TCP listening socket");
        }
        
        return this->_tcp_socket_ready;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[ServerBootstrapper] Exception during TCP socket initialization: {}", e.what());
        _tcp_socket_ready = false;
        return this->_tcp_socket_ready;
    }
}

bool ServerBootstrapper::initialize_udp_socket()
{
    LOG_INFO("[ServerBootstrapper] Initializing UDP multicast socket");
    
    try {
        auto socket_handler = _socket_handler;
        if (!socket_handler) {
            LOG_ERROR("[ServerBootstrapper] SocketHandler not available");
            return false;
        }
        
        _udp_socket_ready = socket_handler->open_udp_multicast_tx_socket();
        
        if (_udp_socket_ready == true) {
            LOG_INFO("[ServerBootstrapper] UDP multicast socket initialized successfully");
        } else {
            LOG_ERROR("[ServerBootstrapper] Failed to open UDP multicast socket");
        }
        
        return _udp_socket_ready;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[ServerBootstrapper] Exception during UDP socket initialization: {}", e.what());
        _udp_socket_ready = false;
        return _udp_socket_ready;
    }
}

bool ServerBootstrapper::start_background_services()
{
    LOG_INFO("[ServerBootstrapper] Starting background services");
    
    try {
        auto thread_pool_handler = _thread_pool_handler;
        auto tlscon_handler = _tlscon_handler;
        auto user_mgmt = _user_session_use_case;

        if (!thread_pool_handler || !tlscon_handler || !user_mgmt) {
            LOG_ERROR("[ServerBootstrapper] Required handlers not available");
            return false;
        }

        // ハートビートチェックタスクを開始(非アクティブ/未認証接続の定期クリーンアップ)
        auto task_mgmt = _task_scheduler;
        if (task_mgmt) {
            task_mgmt->schedule_task_with_timeout(
                TaskPriority::CRITICAL,
                TaskCategory::HEARTBEAT,
                "Heartbeat monitoring task",
                kPermanentTaskTimeout,
                [user_mgmt]() {
                    user_mgmt->start_heartbeat_monitoring();
                }
            );
        }

        // 証明書リロードタスクを開始
        if (task_mgmt) {
            task_mgmt->schedule_task_with_timeout(
                TaskPriority::LOW,
                TaskCategory::MAINTENANCE,
                "Certificate hot reload task",
                kPermanentTaskTimeout,
                [tlscon_handler, cert_path = _cert_path, key_path = _key_path]() {
                    tlscon_handler->hot_reload_certificates(60, cert_path, key_path);
                }
            );
        }
        
        _background_services_running = true;
        LOG_INFO("[ServerBootstrapper] Background services started successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[ServerBootstrapper] Exception during background services startup: {}", e.what());
        _background_services_running = false;
        return false;
    }
}

bool ServerBootstrapper::configure_request_dispatching()
{
    LOG_INFO("[ServerBootstrapper] Configuring request dispatching");
    
    try {
        auto dispatcher = _request_dispatcher;
        auto user_mgmt = _user_session_use_case;
        
        if (!dispatcher || !user_mgmt) {
            LOG_ERROR("[ServerBootstrapper] Required services not available");
            return false;
        }
        
        // Accept callback設定
        // accept()/SSL_accept()はブロッキング呼び出しのため、ディスパッチャースレッド上で
        // 直接実行すると、ハンドシェイクが滞留した1接続が全体の処理を止めてしまう。
        // スレッドプール側のタスクとして実行することで、他の接続への影響を防ぐ。
        auto task_mgmt = _task_scheduler;
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
        
        // Message handle callback設定 - TaskScheduler経由でマルチスレッド処理
        dispatcher->set_message_handle_callback([this](const MessageDispatchResult& result) {
            LOG_DEBUG("[ServerBootstrapper] Message dispatch callback triggered - socket: {}, type: {}, success: {}",
                     result.socket, to_string(result.type), result.success);

            // 認証が必要なメッセージ種別は、未認証のソケットからの場合は処理せず破棄する
            // (SESSION_REQUEST/HEARTBEAT/DISCONNECTはSIGNIN前の接続でも必要なため対象外)
            switch (result.type) {
                case MessageDispatchResult::Type::CHAT_MESSAGE:
                case MessageDispatchResult::Type::CHATROOM_CONTROL:
                case MessageDispatchResult::Type::SYNC_REQUEST:
                case MessageDispatchResult::Type::USER_SEARCH_REQUEST:
                    if (!_user_session_use_case->is_socket_authenticated(result.socket)) {
                        LOG_WARNING("[ServerBootstrapper] Rejecting message type {} from unauthenticated socket: {}",
                                    static_cast<int>(result.type), result.socket);
                        return;
                    }
                    break;
                default:
                    break;
            }

            // メッセージタイプに応じて適切な優先度でタスクスケジューリング
            switch (result.type) {
                case MessageDispatchResult::Type::HEARTBEAT:
                    _task_scheduler->schedule_task(
                        TaskPriority::CRITICAL,
                        TaskCategory::HEARTBEAT,
                        "Process user heartbeat",
                        [this, socket = result.socket]() {
                            _user_session_use_case->process_heartbeat(socket);
                        }
                    );
                    break;
                    
                case MessageDispatchResult::Type::SESSION_REQUEST:
                    _task_scheduler->schedule_task(
                        TaskPriority::HIGH,
                        TaskCategory::AUTHENTICATION,
                        "Process session request",
                        [this, message_data = result.message_data, socket = result.socket]() {
                            _user_session_use_case->process_session_request(message_data, socket);
                        }
                    );
                    break;
                    
                case MessageDispatchResult::Type::CHAT_MESSAGE:
                    _task_scheduler->schedule_task(
                        TaskPriority::NORMAL,
                        TaskCategory::CHAT_PROCESSING,
                        "Process chat message",
                        [this, message_data = result.message_data, socket = result.socket]() {
                            _chat_use_case->process_chat_message(message_data, socket);
                        }
                    );
                    break;
                    
                case MessageDispatchResult::Type::CHATROOM_CONTROL:
                    _task_scheduler->schedule_task(
                        TaskPriority::HIGH,
                        TaskCategory::CHAT_PROCESSING,
                        "Process chatroom control",
                        [this, message_data = result.message_data, socket = result.socket]() {
                            _chatroom_use_case->process_chatroom_operation(message_data, socket);
                        }
                        
                    );
                    break;
                    
                case MessageDispatchResult::Type::DISCONNECT:
                    _task_scheduler->schedule_task(
                        TaskPriority::CRITICAL,
                        TaskCategory::USER_MANAGEMENT,
                        "Process user disconnection",
                        [this, socket = result.socket]() {
                            _user_session_use_case->process_user_disconnection(socket);
                        }
                    );
                    break;

                case MessageDispatchResult::Type::USER_SETTING_CONTROL:
                    //ToDo: USER_SETTING_CONTROLの処理を実装
                    break;

                case MessageDispatchResult::Type::SYNC_REQUEST:
                    _task_scheduler->schedule_task(
                        TaskPriority::NORMAL,
                        TaskCategory::CHAT_PROCESSING,
                        "Process sync request",
                        [this, message_data = result.message_data, socket = result.socket]() {
                            _sync_use_case->process_sync_request(message_data, socket);
                        }
                    );
                    break;

                case MessageDispatchResult::Type::USER_SEARCH_REQUEST:
                    _task_scheduler->schedule_task(
                        TaskPriority::NORMAL,
                        TaskCategory::USER_MANAGEMENT,
                        "Process user search request",
                        [this, message_data = result.message_data, socket = result.socket]() {
                            _user_session_use_case->process_user_search(message_data, socket);
                        }
                    );
                    break;
                    
                default:
                    LOG_ERROR("[ServerBootstrapper] Unknown message type ({}) or error occurred for socket: {}",
                             static_cast<int>(result.type), result.socket);
                    if (!result.success) {
                        LOG_ERROR("[ServerBootstrapper] Error message: {}", result.error_message);
                    }
                    break;
            }
        });
        
        _request_dispatcher_configured = true;
        LOG_INFO("[ServerBootstrapper] Request dispatcher configured successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[ServerBootstrapper] Exception during request dispatcher configuration: {}", e.what());
        _request_dispatcher_configured = false;
        return false;
    }
}

// ==================== システム監視 ====================

ServerStartupPhase ServerBootstrapper::get_startup_phase() const
{
    return _current_phase.load();
}

bool ServerBootstrapper::is_server_ready() const
{
    return _current_phase.load() == ServerStartupPhase::COMPLETED;
}

bool ServerBootstrapper::is_system_healthy() const
{
    return is_server_ready() && 
           _tcp_socket_ready.load() && 
           _udp_socket_ready.load() && 
           _background_services_running.load() && 
           _request_dispatcher_configured.load() &&
           !_server_shutdown_requested.load();
}

// ==================== 設定管理 ====================

void ServerBootstrapper::update_startup_config(const ServerStartupConfig& config)
{
    std::lock_guard<std::mutex> lock(_config_mutex);
    _startup_config = config;
    LOG_INFO("[ServerBootstrapper] Startup configuration updated");
}

const ServerStartupConfig& ServerBootstrapper::get_startup_config() const
{
    std::lock_guard<std::mutex> lock(_config_mutex);
    return _startup_config;
}

// ==================== 内部ヘルパーメソッド ====================

bool ServerBootstrapper::enforce_phase_timeout(
    const std::chrono::steady_clock::time_point& phase_start,
    std::chrono::milliseconds budget,
    const std::string& phase_name)
{
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - phase_start);
    if (elapsed > budget) {
        LOG_ERROR("[ServerBootstrapper] Phase '{}' exceeded its timeout budget ({}ms > {}ms)",
                   phase_name, elapsed.count(), budget.count());
        return false;
    }
    return true;
}

bool ServerBootstrapper::enforce_total_timeout(const ServerStartupConfig& config)
{
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - _startup_begin_time);
    if (elapsed > config.total_timeout) {
        LOG_ERROR("[ServerBootstrapper] Server startup exceeded total timeout budget ({}ms > {}ms)",
                   elapsed.count(), config.total_timeout.count());
        return false;
    }
    return true;
}

void ServerBootstrapper::set_startup_phase(ServerStartupPhase new_phase)
{
    _current_phase = new_phase;
    LOG_DEBUG("[ServerBootstrapper] Startup phase changed to: {}", static_cast<int>(new_phase));
}

void ServerBootstrapper::record_startup_metrics()
{
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        _startup_complete_time - _startup_begin_time);
    LOG_INFO("[ServerBootstrapper] Server startup completed in {}ms", duration.count());
}

void ServerBootstrapper::cleanup_resources()
{
    LOG_DEBUG("[ServerBootstrapper] Cleaning up resources");
    _tcp_socket_ready = false;
    _udp_socket_ready = false;
    _background_services_running = false;
    _request_dispatcher_configured = false;
}

