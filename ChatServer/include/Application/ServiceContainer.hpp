#ifndef SERVICECONTAINER_HPP
#define SERVICECONTAINER_HPP

#include <functional>
#include <mutex>
#include <memory> 

// Configuration constants
#define SERVER_CERT_FILE_PATH "/app/ChatServer/certs/server-cert.pem"
#define SERVER_PRIVATEKEY_FILE_PATH "/app/ChatServer/certs/server-key.pem"
#define SOCKET_CONFIG_FILE_PATH "/app/ChatServer/conf/socket_setting.ini"

// Forward declarations
// CommonLib
class SocketHandler;
class TLSConnectionHandler;
class ServerTime;
class ThreadPoolHandler;

// Infrastructure
class MySQLAdapter;
class RedisAdapter;

// Domain
class UserAggregateStore;
class UserRepositoryHandler;
class ChatroomRepositoryHandler;
class CommunicationHandler;
class SessionHandler;
class ChatroomSequenceHandler;

// Domain Interfaces
class IChatroomRepository;
class IMessageRepository;
class IUserRepository;

// Application
class UserManagementService;
class ChatroomManagementService;
class ChatService;
class RequestDispatcherService;
class TaskManagementService;
class StartupService;
class IMessageDeliveryService;

class ServiceContainer {
public:
    static ServiceContainer& get_instance();
    
    // 初期化とクリーンアップ
    void initialize();
    void cleanup();
    
    // 共有リソースのゲッター
    std::shared_ptr<SocketHandler> get_socket_handler();
    std::shared_ptr<RedisAdapter> get_redis_adapter();
    std::shared_ptr<TLSConnectionHandler> get_protocol_handler();
    std::shared_ptr<ServerTime> get_server_time();
    std::shared_ptr<ThreadPoolHandler> get_thread_pool_handler();
    
    // ドメイン層のゲッター
    std::shared_ptr<UserAggregateStore> get_user_store();
    std::shared_ptr<UserRepositoryHandler> get_user_repository_handler();
    std::shared_ptr<ChatroomRepositoryHandler> get_chatroom_repository_handler();
    std::shared_ptr<CommunicationHandler> get_communication_handler();
    std::shared_ptr<SessionHandler> get_session_handler();
    std::shared_ptr<ChatroomSequenceHandler> get_chatroom_sequence_handler();

    //ドメイン層（インタフェース)のゲッター
    std::shared_ptr<IChatroomRepository> get_chatroom_repository(); 
    std::shared_ptr<IMessageRepository> get_message_repository(); 
    std::shared_ptr<IUserRepository> get_user_repository();
    
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
    std::shared_ptr<MySQLAdapter> _mysql_adapter;
    std::shared_ptr<RedisAdapter> _redis_adapter;
    std::shared_ptr<TLSConnectionHandler> _tlscon_handler;
    std::shared_ptr<ServerTime> _server_time;
    std::shared_ptr<ThreadPoolHandler> _thread_pool_handler;
    
    
    // ドメイン層
    std::shared_ptr<UserAggregateStore> _user_store;
    std::shared_ptr<UserRepositoryHandler> _user_repository_handler;
    std::shared_ptr<ChatroomRepositoryHandler> _chatroom_repository_handler;
    std::shared_ptr<CommunicationHandler> _communication_handler;
    std::shared_ptr<SessionHandler> _session_handler;
    std::shared_ptr<ChatroomSequenceHandler> _chatroom_sequence_handler;

    //ドメイン層（インタフェース)
    std::shared_ptr<IChatroomRepository> _chatroom_repository;
    std::shared_ptr<IMessageRepository> _message_repository;
    std::shared_ptr<IUserRepository> _user_repository;
    
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