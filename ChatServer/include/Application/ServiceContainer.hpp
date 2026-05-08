#ifndef SERVICECONTAINER_HPP
#define SERVICECONTAINER_HPP
#include "Utils/common.hpp"
#include "Application/Services/ChatService.hpp"
#include "Application/Services/ChatroomManagementService.hpp"
#include "Application/Services/UserManagementService.hpp"
#include "Application/Services/RequestDispatcherService.hpp"
#include "Application/Services/TaskManagementService.hpp"
#include "Application/Services/StartupService.hpp"
#include "Application/Services/MessageDeliveryService.hpp"
#include "Domain/CommunicationHandler.hpp"
#include "Domain/UserRepositoryHandler.hpp"
#include "Domain/ChatroomRepositoryHandler.hpp"
#include "Domain/SessionHandler.hpp"
#include "Domain/UserAggregateStore.hpp"

// Forward declarations
class SocketHandler;
class MySQLClient;
class RedisClient;
class ProtocolHandler;
class ServerTime;
class ThreadPoolHandler;
class IMessageDeliveryService;
class SessionHandler;

class ServiceContainer {
public:
    static ServiceContainer& get_instance();
    
    // 初期化とクリーンアップ
    void initialize();
    void cleanup();
    
    // 共有リソースのゲッター
    std::shared_ptr<SocketHandler> get_socket_handler();
    std::shared_ptr<MySQLClient> get_mysql_client();
    std::shared_ptr<RedisClient> get_redis_client();
    std::shared_ptr<ProtocolHandler> get_protocol_handler();
    std::shared_ptr<ServerTime> get_server_time();
    std::shared_ptr<ThreadPoolHandler> get_thread_pool_handler();
    
    // ドメイン層のゲッター
    std::shared_ptr<UserAggregateStore> get_user_store();
    std::shared_ptr<UserRepositoryHandler> get_user_repository_handler();
    std::shared_ptr<ChatroomRepositoryHandler> get_chatroom_repository_handler();
    std::shared_ptr<CommunicationHandler> get_communication_handler();
    std::shared_ptr<SessionHandler> get_session_handler();
    
    // アプリケーションサービスのゲッター
    std::shared_ptr<UserManagementService> get_user_management_service();
    std::shared_ptr<ChatroomManagementService> get_chatroom_management_service();
    std::shared_ptr<ChatService> get_chat_service();
    std::shared_ptr<RequestDispatcherService> get_request_dispatcher_service();
    std::shared_ptr<TaskManagementService> get_task_management_service();
    std::shared_ptr<StartupService> get_startup_service();
    std::shared_ptr<IMessageDeliveryService> get_message_delivery_service();

private:
    ServiceContainer() = default;
    ~ServiceContainer() = default;
    ServiceContainer(const ServiceContainer&) = delete;
    ServiceContainer& operator=(const ServiceContainer&) = delete;
    
    // 共有リソース (インフラストラクチャ層)
    std::shared_ptr<SocketHandler> _socket_handler;
    std::shared_ptr<MySQLClient> _mysql_client;
    std::shared_ptr<RedisClient> _redis_client;
    std::shared_ptr<ProtocolHandler> _protocol_handler;
    std::shared_ptr<ServerTime> _server_time;
    std::shared_ptr<ThreadPoolHandler> _thread_pool_handler;
    
    
    // ドメイン層
    std::shared_ptr<UserAggregateStore> _user_store;
    std::shared_ptr<UserRepositoryHandler> _user_repository_handler;
    std::shared_ptr<ChatroomRepositoryHandler> _chatroom_repository_handler;
    std::shared_ptr<CommunicationHandler> _communication_handler;
    std::shared_ptr<SessionHandler> _session_handler;
    
    // アプリケーションサービス層(サービス)
    std::shared_ptr<UserManagementService> _user_management_service;
    std::shared_ptr<ChatroomManagementService> _chatroom_management_service;
    std::shared_ptr<ChatService> _chat_service;
    std::shared_ptr<RequestDispatcherService> _request_dispatcher_service;
    std::shared_ptr<TaskManagementService> _task_management_service;
    std::shared_ptr<StartupService> _startup_service;
    std::shared_ptr<IMessageDeliveryService> _message_delivery_service;

    std::mutex _mutex;
    bool _initialized = false;
};
#endif // SERVICECONTAINER_HPP