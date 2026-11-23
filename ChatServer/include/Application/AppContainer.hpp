#ifndef APPCONTAINER_HPP
#define APPCONTAINER_HPP

#include <functional>
#include <mutex>
#include <memory> 

// Configuration constants
#define SOCKET_CONFIG_FILE_PATH "/app/ChatServer/conf/socket_setting.conf"
#define CHAT_CONFIG_FILE_PATH "/app/ChatServer/conf/chat.conf"

// Forward declarations
// CommonLib
class SocketHandler;
class TLSConnectionHandler;
class ServerTime;
class ThreadPoolHandler;

// Infrastructure
class MySQLAdapter;
class RedisAdapter;
class GrpcSessionRepository;

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
class UserSessionUseCase;
class ChatroomUseCase;
class ChatUseCase;
class RequestDispatcher;
class TaskScheduler;
class ServerBootstrapper;
class IMessageSender;
class SyncUseCase;
class SystemMonitor;

class AppContainer {
public:
    static AppContainer& get_instance();
    
    // 初期化とクリーンアップ
    void initialize();
    void cleanup();

    // 共有リソースのゲッター
    std::shared_ptr<SocketHandler> get_socket_handler();
    std::shared_ptr<RedisAdapter> get_redis_adapter();
    std::shared_ptr<TLSConnectionHandler> get_protocol_handler();
    std::shared_ptr<ServerTime> get_server_time();
    std::shared_ptr<ThreadPoolHandler> get_heartbeat_thread_pool();
    std::shared_ptr<ThreadPoolHandler> get_realtime_thread_pool();
    std::shared_ptr<ThreadPoolHandler> get_background_thread_pool();
    
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
    std::shared_ptr<UserSessionUseCase> get_user_session_use_case();
    std::shared_ptr<ChatroomUseCase> get_chatroom_use_case();
    std::shared_ptr<ChatUseCase> get_chat_use_case();
    std::shared_ptr<RequestDispatcher> get_request_dispatcher();
    std::shared_ptr<TaskScheduler> get_task_scheduler();
    std::shared_ptr<ServerBootstrapper> get_server_bootstrapper();
    std::shared_ptr<IMessageSender> get_message_sender();
    std::shared_ptr<SyncUseCase> get_sync_use_case();
    std::shared_ptr<SystemMonitor> get_system_monitor();

private:
    AppContainer() = default;
    ~AppContainer() = default;
    AppContainer(const AppContainer&) = delete;
    AppContainer& operator=(const AppContainer&) = delete;
    
    // 共有リソース (インフラストラクチャ層)
    std::shared_ptr<SocketHandler> _socket_handler;
    std::shared_ptr<MySQLAdapter> _mysql_adapter;
    std::shared_ptr<RedisAdapter> _redis_adapter;
    std::shared_ptr<GrpcSessionRepository> _grpc_session_repository; // AuthenticationServer経由のセッション検証/削除(ISessionRepository)
    std::shared_ptr<TLSConnectionHandler> _tlscon_handler;
    std::shared_ptr<ServerTime> _server_time;
    std::shared_ptr<ThreadPoolHandler> _heartbeat_pool;
    std::shared_ptr<ThreadPoolHandler> _realtime_pool;
    std::shared_ptr<ThreadPoolHandler> _background_pool;
    
    
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
    std::shared_ptr<UserSessionUseCase> _user_session_use_case;
    std::shared_ptr<ChatroomUseCase> _chatroom_use_case;
    std::shared_ptr<ChatUseCase> _chat_use_case;
    std::shared_ptr<RequestDispatcher> _request_dispatcher;
    std::shared_ptr<TaskScheduler> _task_scheduler;
    std::shared_ptr<ServerBootstrapper> _server_bootstrapper;
    std::shared_ptr<IMessageSender> _message_sender;
    std::shared_ptr<SyncUseCase> _sync_use_case;
    std::shared_ptr<SystemMonitor> _system_monitor;

    std::mutex _mutex;
    bool _initialized = false;
};
#endif // APPCONTAINER_HPP