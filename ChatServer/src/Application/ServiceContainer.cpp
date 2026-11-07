#include "Application/ServiceContainer.hpp"
#include "EnvHelper.hpp"
#include <cmath>
#include <thread>

// Application層
#include "Application/Services/ChatService.hpp"
#include "Application/Services/SyncService.hpp"
#include "Application/Services/ChatroomManagementService.hpp"
#include "Application/Services/UserManagementService.hpp"
#include "Application/Services/RequestDispatcherService.hpp"
#include "Application/Services/TaskManagementService.hpp"
#include "Application/Services/StartupService.hpp"
#include "Application/Services/SystemMonitorService.hpp"
#include "Application/Interfaces/IMessageDeliveryService.hpp"
#include "Application/Services/MessageDeliveryService.hpp"

// Infrastructure層
#include "Infrastructure/MySQLAdapter.hpp"
#include "Infrastructure/RedisAdapter.hpp"

// Domain層
#include "Domain/UserAggregateStore.hpp"
#include "Domain/ChatroomRepositoryHandler.hpp"
#include "Domain/UserRepositoryHandler.hpp"
#include "Domain/SessionHandler.hpp"
#include "Domain/ChatroomSequenceHandler.hpp"
#include "Domain/CommunicationHandler.hpp"
#include "Domain/MessageIDGenerator.hpp"
#include "inicpp.h"

// Domain層インタフェース
#include "Domain/Interfaces/IChatroomRepository.hpp"
#include "Domain/Interfaces/IMessageRepository.hpp"
#include "Domain/Interfaces/IUserRepository.hpp"

//Util関連
#include "Utils/common.hpp"

// CommonLibrary
#include "Network/SocketHandler.hpp"
#include "Network/TLSConnectionHandler.hpp"
#include "ServerTime.hpp"
#include "ThreadPoolHandler.hpp"
// シングルトンインスタンスの取得
ServiceContainer& ServiceContainer::get_instance() {
    static ServiceContainer instance;
    return instance;
}

// サービスコンテナの初期化
void ServiceContainer::initialize() {
    std::lock_guard<std::mutex> lock(_mutex);
    
    if (_initialized) {
        return;
    }
    
    try {
        //Phase 0 : 環境変数の読み取り
        const std::string cert_path = get_env("TLS_CERT_PATH");
        const std::string key_path  = get_env("TLS_KEY_PATH");
        // CHAT_SERVER_MACHINE_IDの設定不備を起動時に検出する(実際の値はgenerate_message_id内で遅延キャッシュされる)
        MessageIDGenerator::resolve_machine_id();

        // chat.confの読み込み
        ini::IniFile chat_config;
        chat_config.load(CHAT_CONFIG_FILE_PATH);
        const int max_resend_count = chat_config["ChatSettings"]["max_resend_count"].as<int>();
        const int max_user_num_in_chatroom = chat_config["ChatSettings"]["max_user_num_in_chatroom"].as<int>();
        const int max_connections = chat_config["ChatSettings"]["max_connections"].as<int>();
        const int handshake_timeout_sec = chat_config["ChatSettings"]["handshake_timeout_sec"].as<int>();
        const int heartbeat_timeout_sec = chat_config["ChatSettings"]["heartbeat_timeout_sec"].as<int>();
        const int heartbeat_check_interval_sec = chat_config["ChatSettings"]["heartbeat_check_interval_sec"].as<int>();

        // StartupSettingsの読み込み(StartupServiceの起動設定パラメータ)
        ServerStartupConfig startup_config;
        startup_config.socket_timeout = std::chrono::milliseconds(chat_config["StartupSettings"]["socket_timeout_ms"].as<int>());
        startup_config.service_init_timeout = std::chrono::milliseconds(chat_config["StartupSettings"]["service_init_timeout_ms"].as<int>());
        startup_config.total_timeout = std::chrono::milliseconds(chat_config["StartupSettings"]["total_timeout_ms"].as<int>());
        startup_config.enable_overload_protection = chat_config["StartupSettings"]["enable_overload_protection"].as<bool>();
        startup_config.enable_health_monitoring = chat_config["StartupSettings"]["enable_health_monitoring"].as<bool>();

        // SystemMonitorServiceの監視設定(同じStartupSettingsセクションから読み込む)
        SystemMonitorConfig monitor_config;
        monitor_config.max_concurrent_connections = static_cast<uint32_t>(chat_config["StartupSettings"]["max_concurrent_connections"].as<int>());
        monitor_config.check_interval = std::chrono::seconds(chat_config["StartupSettings"]["health_check_interval_sec"].as<int>());
        monitor_config.high_usage_warning_threshold_percent = chat_config["StartupSettings"]["high_usage_warning_threshold_percent"].as<double>();

        // ThreadPoolSettingsの読み込み(カテゴリ別スレッドプールのサイズ)
        // 比率(0.0〜1.0)×実コア数で算出することで、本番/開発などコア数が異なる
        // 環境でも同じ設定値のままバランスの取れたスレッド数になる(最低1スレッド)。
        const double heartbeat_pool_ratio = chat_config["ThreadPoolSettings"]["heartbeat_pool_ratio"].as<double>();
        const double realtime_pool_ratio = chat_config["ThreadPoolSettings"]["realtime_pool_ratio"].as<double>();
        const double background_pool_ratio = chat_config["ThreadPoolSettings"]["background_pool_ratio"].as<double>();

        const unsigned int hardware_concurrency = std::max(1u, std::thread::hardware_concurrency());
        const int heartbeat_pool_threads = std::max(1, static_cast<int>(std::round(heartbeat_pool_ratio * hardware_concurrency)));
        const int realtime_pool_threads = std::max(1, static_cast<int>(std::round(realtime_pool_ratio * hardware_concurrency)));
        const int background_pool_threads = std::max(1, static_cast<int>(std::round(background_pool_ratio * hardware_concurrency)));

        LOG_INFO("[ServiceContainer] Thread pool sizing - hardware_concurrency: {}, heartbeat: {}, realtime: {}, background: {}",
                 hardware_concurrency, heartbeat_pool_threads, realtime_pool_threads, background_pool_threads);

        // OverloadProtectionSettingsの読み込み(TaskManagementServiceの過負荷保護しきい値)
        OverloadProtectionConfig overload_protection_config;
        overload_protection_config.max_active_tasks = static_cast<size_t>(chat_config["OverloadProtectionSettings"]["max_active_tasks"].as<int>());
        overload_protection_config.active_task_hard_limit = static_cast<size_t>(chat_config["OverloadProtectionSettings"]["active_task_hard_limit"].as<int>());
        overload_protection_config.overload_recovery_ratio = chat_config["OverloadProtectionSettings"]["overload_recovery_ratio"].as<double>();

        // Phase 1: インフラストラクチャ層の共有リソースを初期化
        _server_time = std::make_shared<ServerTime>();
        _socket_handler = std::make_shared<SocketHandler>(SOCKET_CONFIG_FILE_PATH);
        _mysql_adapter = std::make_shared<MySQLAdapter>();
        _redis_adapter = std::make_shared<RedisAdapter>();
        _tlscon_handler = std::make_shared<TLSConnectionHandler>(cert_path, key_path);
        // カテゴリ別にスレッドプールを分離する(他の処理の渋滞でハートビートが
        // 止まる、といった飢餓状態を避けるため、1プールを優先度で並び替えるのではなく
        // 物理的に別プールにする)
        _heartbeat_pool = std::make_shared<ThreadPoolHandler>(heartbeat_pool_threads);
        _realtime_pool = std::make_shared<ThreadPoolHandler>(realtime_pool_threads);
        _background_pool = std::make_shared<ThreadPoolHandler>(background_pool_threads);

        // Interface binding
        _chatroom_repository = _mysql_adapter;
        _message_repository = _mysql_adapter;
        _user_repository = _mysql_adapter;

        // Phase 2: タスク管理層を初期化
        _task_management_service = std::make_shared<TaskManagementService>(
            _heartbeat_pool, _realtime_pool, _background_pool);
        _task_management_service->update_overload_config(overload_protection_config);
        
        // Phase 3: データストレージを初期化
        _user_store = std::make_shared<UserAggregateStore>();
        _chatroom_repository_handler = std::make_shared<ChatroomRepositoryHandler>(_chatroom_repository, _user_repository, _message_repository, max_user_num_in_chatroom);
        _user_repository_handler = std::make_shared<UserRepositoryHandler>(_user_store, _user_repository, _server_time);
        _communication_handler = std::make_shared<CommunicationHandler>(_user_store, _socket_handler, _tlscon_handler, handshake_timeout_sec);
        
        // Phase 3.5: ドメインハンドラーを初期化
        _session_handler = std::make_shared<SessionHandler>(_user_repository_handler, _redis_adapter, _user_repository, _message_repository,_server_time);
        _chatroom_sequence_handler = std::make_shared<ChatroomSequenceHandler>(_redis_adapter);
        
        // Phase 4: メッセージ配信サービス
        _message_delivery_service = std::make_shared<MessageDeliveryService>(_communication_handler,_chatroom_repository_handler ,_user_repository_handler); 
        
        // Phase 5: ビジネスサービスを初期化
        _user_management_service = std::make_shared<UserManagementService>(
            _server_time, _session_handler, _user_repository_handler, _communication_handler,
            max_connections, heartbeat_timeout_sec, heartbeat_check_interval_sec);
            
        _chatroom_management_service = std::make_shared<ChatroomManagementService>(
            _message_delivery_service, _chatroom_repository_handler, _chatroom_sequence_handler, _user_repository_handler);
            
        _chat_service = std::make_shared<ChatService>(
            _message_delivery_service, _user_repository_handler, _chatroom_repository_handler, _chatroom_sequence_handler, max_resend_count);

        _sync_service = std::make_shared<SyncService>(
            _message_delivery_service, _chatroom_repository_handler, _user_repository_handler);

        _request_dispatcher_service = std::make_shared<RequestDispatcherService>(
            _socket_handler, _communication_handler);

        // Phase 5.5: システム監視サービスを初期化
        _system_monitor_service = std::make_shared<SystemMonitorService>(
            _task_management_service, _user_management_service);
        _system_monitor_service->update_monitor_config(monitor_config);

        // Phase 6: スタートアップサービスを初期化（すべてのサービスが準備された後）
        _startup_service = std::make_shared<StartupService>(
            _socket_handler,
            _realtime_pool,
            _tlscon_handler,
            _task_management_service,
            _request_dispatcher_service,
            _user_management_service,
            _chat_service,
            _chatroom_management_service,
            _sync_service,
            _system_monitor_service,
            cert_path,
            key_path
        );
        _startup_service->update_startup_config(startup_config);

        _initialized = true;
        
        LOG_INFO("ServiceContainer initialized successfully with new service architecture");
    }
    catch (const std::exception& e) {
        LOG_ERROR("ServiceContainer initialization failed: " + std::string(e.what()));
        throw;
    }
}


// 共有リソースのゲッター
std::shared_ptr<SocketHandler> ServiceContainer::get_socket_handler() {
    if (!_initialized) {
        throw std::runtime_error("ServiceContainer not initialized");
    }
    return this->_socket_handler;
}

std::shared_ptr<RedisAdapter> ServiceContainer::get_redis_adapter() {
    if (!_initialized) {
        throw std::runtime_error("ServiceContainer not initialized");
    }
    return this->_redis_adapter;
}

std::shared_ptr<TLSConnectionHandler> ServiceContainer::get_protocol_handler() {
    if (!_initialized) {
        throw std::runtime_error("ServiceContainer not initialized");
    }
    return this->_tlscon_handler;
}

std::shared_ptr<ServerTime> ServiceContainer::get_server_time() {
    if (!_initialized) {
        throw std::runtime_error("ServiceContainer not initialized");
    }
    return this->_server_time;
}

std::shared_ptr<ThreadPoolHandler> ServiceContainer::get_heartbeat_thread_pool() {
    if (!_initialized) {
        throw std::runtime_error("ServiceContainer not initialized");
    }
    return this->_heartbeat_pool;
}

std::shared_ptr<ThreadPoolHandler> ServiceContainer::get_realtime_thread_pool() {
    if (!_initialized) {
        throw std::runtime_error("ServiceContainer not initialized");
    }
    return this->_realtime_pool;
}

std::shared_ptr<ThreadPoolHandler> ServiceContainer::get_background_thread_pool() {
    if (!_initialized) {
        throw std::runtime_error("ServiceContainer not initialized");
    }
    return this->_background_pool;
}

// ドメイン層のゲッター
std::shared_ptr<UserAggregateStore> ServiceContainer::get_user_store() {
    if (!_initialized) {
        throw std::runtime_error("ServiceContainer not initialized");
    }
    return _user_store;
}

std::shared_ptr<UserRepositoryHandler> ServiceContainer::get_user_repository_handler() {
    if (!_initialized) {
        throw std::runtime_error("ServiceContainer not initialized");
    }
    return _user_repository_handler;
}

std::shared_ptr<ChatroomRepositoryHandler> ServiceContainer::get_chatroom_repository_handler() {
    if (!_initialized) {
        throw std::runtime_error("ServiceContainer not initialized");
    }
    return _chatroom_repository_handler;
}

std::shared_ptr<CommunicationHandler> ServiceContainer::get_communication_handler()
{
    if (!_initialized || !_communication_handler) {
        throw std::runtime_error("ServiceContainer not initialized or CommunicationHandler not available");
    }
    return _communication_handler;
} 

std::shared_ptr<SessionHandler> ServiceContainer::get_session_handler() {
    if (!_initialized) {
        throw std::runtime_error("ServiceContainer not initialized");
    }
    return _session_handler;
} 

std::shared_ptr<ChatroomSequenceHandler> ServiceContainer::get_chatroom_sequence_handler() {
    if (!_initialized || !_chatroom_sequence_handler) {
        throw std::runtime_error("ServiceContainer not initialized or ChatroomSequenceHandler not available");
    }
    return _chatroom_sequence_handler;
}

std::shared_ptr<IChatroomRepository> ServiceContainer::get_chatroom_repository()
{
    if(!_initialized || !_chatroom_repository){
        throw std::runtime_error("ServiceContainer not initialized or ChatroomRepository not available");
    }
    return _chatroom_repository;
}

std::shared_ptr<IMessageRepository> ServiceContainer::get_message_repository()
{
    if(!_initialized || !_message_repository){
        throw std::runtime_error("ServiceContainer not initialized or MessageRepository not available");
    }
    return _message_repository;
}

std::shared_ptr<IUserRepository> ServiceContainer::get_user_repository()
{
    if(!_initialized || !_user_repository){
        throw std::runtime_error("ServiceContainer not initialized or UserRepository not available");
    }
    return _user_repository;
}

// アプリケーションサービスのゲッター
std::shared_ptr<UserManagementService> ServiceContainer::get_user_management_service() {
    if (!_initialized || !_user_management_service) {
        throw std::runtime_error("ServiceContainer not initialized or UserManagementService not available");
    }
    return _user_management_service;
}

std::shared_ptr<ChatroomManagementService> ServiceContainer::get_chatroom_management_service() {
    if (!_initialized || !_chatroom_management_service) {
        throw std::runtime_error("ServiceContainer not initialized or ChatroomManagementService not available");
    }
    return _chatroom_management_service;
}

std::shared_ptr<ChatService> ServiceContainer::get_chat_service() {
    if (!_initialized || !_chat_service) {
        throw std::runtime_error("ServiceContainer not initialized or ChatService not available");
    }
    return _chat_service;
}

std::shared_ptr<SyncService> ServiceContainer::get_sync_service() {
    if (!_initialized || !_sync_service) {
        throw std::runtime_error("ServiceContainer not initialized or SyncService not available");
    }
    return _sync_service;
}

std::shared_ptr<RequestDispatcherService> ServiceContainer::get_request_dispatcher_service() {
    if (!_initialized || !_request_dispatcher_service) {
        throw std::runtime_error("ServiceContainer not initialized or RequestDispatcherService not available");
    }
    return _request_dispatcher_service;
}

std::shared_ptr<TaskManagementService> ServiceContainer::get_task_management_service() {
    if (!_initialized || !_task_management_service) {
        throw std::runtime_error("ServiceContainer not initialized or TaskManagementService not available");
    }
    return _task_management_service;
}

std::shared_ptr<StartupService> ServiceContainer::get_startup_service() {
    if (!_initialized || !_startup_service) {
        throw std::runtime_error("ServiceContainer not initialized or StartupService not available");
    }
    return _startup_service;
}

std::shared_ptr<SystemMonitorService> ServiceContainer::get_system_monitor_service() {
    if (!_initialized || !_system_monitor_service) {
        throw std::runtime_error("ServiceContainer not initialized or SystemMonitorService not available");
    }
    return _system_monitor_service;
}

std::shared_ptr<IMessageDeliveryService> ServiceContainer::get_message_delivery_service() {
    if (!_initialized) {
        throw std::runtime_error("ServiceContainer not initialized");
    }
    return _message_delivery_service;
}

// クリーンアップ
void ServiceContainer::cleanup() {
    std::lock_guard<std::mutex> lock(_mutex);
    
    try {
        // サービスをクリーンアップ (依存関係の逆順で実施)
        _startup_service.reset();
        _system_monitor_service.reset();
        _request_dispatcher_service.reset();
        _sync_service.reset();
        _chat_service.reset();
        _chatroom_management_service.reset();
        _user_management_service.reset();
        
        // データストレージをクリーンアップ
        _chatroom_repository_handler.reset();
        _user_repository_handler.reset();
        _user_store.reset();
        _communication_handler.reset();
        _session_handler.reset();
        _chatroom_sequence_handler.reset();
        
        // メッセージ配信サービスをクリーンアップ
        _message_delivery_service.reset();
        
        // タスク管理サービスをクリーンアップ
        _task_management_service.reset();
        
        // インフラストラクチャをクリーンアップ (初期化の逆順で実施)
        _heartbeat_pool.reset();
        _realtime_pool.reset();
        _background_pool.reset();
        _tlscon_handler.reset();
        _redis_adapter.reset();
        _mysql_adapter.reset();
        _socket_handler.reset();
        _server_time.reset();
        
        _initialized = false;
        
        LOG_INFO("ServiceContainer cleaned up successfully");
    }
    catch (const std::exception& e) {
        LOG_ERROR("ServiceContainer cleanup failed: " + std::string(e.what()));
    }
}

