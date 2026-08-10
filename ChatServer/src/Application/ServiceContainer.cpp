#include "Application/ServiceContainer.hpp"

// Core Services
#include "Application/Services/ChatService.hpp"
#include "Application/Services/ChatroomManagementService.hpp"
#include "Application/Services/UserManagementService.hpp"
#include "Application/Services/RequestDispatcherService.hpp"
#include "Application/Services/TaskManagementService.hpp"
#include "Application/Services/StartupService.hpp"
#include "Application/Services/IMessageDeliveryService.hpp"

// Infrastructure
#include "Infrastructure/MySQLAdapter.hpp"
#include "Infrastructure/RedisAdapter.hpp"
#include "SocketHandler.hpp"
#include "ProtocolHandler.hpp"
#include "ServerTime.hpp"
#include "ThreadPoolHandler.hpp"
#include "Domain/UserAggregateStore.hpp"
#include "Domain/ChatroomRepositoryHandler.hpp"
#include "Domain/UserRepositoryHandler.hpp"
#include "Domain/SessionHandler.hpp"
#include "Domain/ChatroomSequenceHandler.hpp"

#include <functional>
#include <mutex>

// Configuration constants
#define SERVER_CERT_FILE_PATH "/app/ChatServer/certs/server-cert.pem"
#define SERVER_PRIVATEKEY_FILE_PATH "/app/ChatServer/certs/server-key.pem"
#define SOCKET_CONFIG_FILE_PATH "/app/ChatServer/conf/socket_setting.ini"

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
        // Phase 1: インフラストラクチャ層の共有リソースを初期化
        _server_time = std::make_shared<ServerTime>();
        _socket_handler = std::make_shared<SocketHandler>(SOCKET_CONFIG_FILE_PATH);
        _mysql_adapter = std::make_shared<MySQLAdapter>();
        _redis_adapter = std::make_shared<RedisAdapter>();
        _protocol_handler = std::make_shared<ProtocolHandler>(SERVER_CERT_FILE_PATH, SERVER_PRIVATEKEY_FILE_PATH);
        _thread_pool_handler = std::make_shared<ThreadPoolHandler>(10);
        
        // Phase 2: タスク管理層を初期化
        _task_management_service = std::make_shared<TaskManagementService>(_thread_pool_handler);
        
        // Phase 3: データストレージを初期化
        _user_store = std::make_shared<UserAggregateStore>();
        _chatroom_repository_handler = std::make_shared<ChatroomRepositoryHandler>(_mysql_adapter);
        _user_repository_handler = std::make_shared<UserRepositoryHandler>(_user_store, _mysql_adapter, _server_time);
        _communication_handler = std::make_shared<CommunicationHandler>(_user_store, _socket_handler, _protocol_handler);
        
        // Phase 3.5: ドメインハンドラーを初期化
        _session_handler = std::make_shared<SessionHandler>(_user_repository_handler, _redis_adapter, _mysql_adapter, _server_time);
        _chatroom_sequence_handler = std::make_shared<ChatroomSequenceHandler>(_redis_adapter);
        
        // Phase 4: メッセージ配信サービス（仮実装）
        _message_delivery_service = std::make_shared<MessageDeliveryService>(_communication_handler,_chatroom_repository_handler ,_user_repository_handler); 
        
        // Phase 5: ビジネスサービスを初期化
        _user_management_service = std::make_shared<UserManagementService>(
            _server_time, _session_handler, _user_repository_handler, _communication_handler);
            
        _chatroom_management_service = std::make_shared<ChatroomManagementService>(
            _message_delivery_service, _chatroom_repository_handler, _chatroom_sequence_handler);
            
        _chat_service = std::make_shared<ChatService>(
            _message_delivery_service, _user_repository_handler, _chatroom_repository_handler, _chatroom_sequence_handler);
        
        _request_dispatcher_service = std::make_shared<RequestDispatcherService>(
            _socket_handler, _thread_pool_handler, _communication_handler);
            
        // Phase 6: スタートアップサービスを初期化（すべてのサービスが準備された後）
        _startup_service = std::make_shared<StartupService>(
            _socket_handler,
            _thread_pool_handler,
            _protocol_handler,
            _task_management_service,
            _request_dispatcher_service,
            _user_management_service,
            _chat_service,
            _chatroom_management_service
        );
        
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

std::shared_ptr<MySQLAdapter> ServiceContainer::get_mysql_adapter() {
    if (!_initialized) {
        throw std::runtime_error("ServiceContainer not initialized");
    }
    return this->_mysql_adapter;
}

std::shared_ptr<RedisAdapter> ServiceContainer::get_redis_adapter() {
    if (!_initialized) {
        throw std::runtime_error("ServiceContainer not initialized");
    }
    return this->_redis_adapter;
}

std::shared_ptr<ProtocolHandler> ServiceContainer::get_protocol_handler() {
    if (!_initialized) {
        throw std::runtime_error("ServiceContainer not initialized");
    }
    return this->_protocol_handler;
}

std::shared_ptr<ServerTime> ServiceContainer::get_server_time() {
    if (!_initialized) {
        throw std::runtime_error("ServiceContainer not initialized");
    }
    return this->_server_time;
}

std::shared_ptr<ThreadPoolHandler> ServiceContainer::get_thread_pool_handler() {
    if (!_initialized) {
        throw std::runtime_error("ServiceContainer not initialized");
    }
    return this->_thread_pool_handler;
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
        _request_dispatcher_service.reset();
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
        _thread_pool_handler.reset();
        _protocol_handler.reset();
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