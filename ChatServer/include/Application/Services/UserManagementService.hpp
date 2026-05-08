#ifndef USERMANAGEMENTSERVICE_HPP
#define USERMANAGEMENTSERVICE_HPP
#include "Utils/common.hpp"
#include "Infrastructure/MySQLClient.hpp"
#include "Infrastructure/RedisClient.hpp"
#include "Domain/CommunicationHandler.hpp"
#include "Domain/UserRepositoryHandler.hpp"
#include "Domain/SessionHandler.hpp"
#include "Domain/Entity/User.hpp"
#include "Domain/UserAggregateStore.hpp"
#include "Domain/Entity/Messages/SessionRequestMessage.hpp"
#include "Domain/Entity/Messages/SessionResponseMessage.hpp"

// ChatServiceから共通のSessionResult構造体を使用
struct SessionResult {
    bool verified = false;
    std::string error_message;
    json unreceived_messages;
};

class UserManagementService {
private:
    // データアクセス制御層のみ
    std::shared_ptr<UserRepositoryHandler> _repository;
    std::shared_ptr<CommunicationHandler> _communication;
    std::shared_ptr<SessionHandler> _session_handler;
    
    // ビジネス層のインフラのみ
    std::shared_ptr<MySQLClient> _mysql_client;  // DB操作
    std::shared_ptr<RedisClient> _redis_client;  // セッション管理・キャッシュ
    std::shared_ptr<ServerTime> _server_time;    // 時刻管理
    
    
public:
    UserManagementService(
        std::shared_ptr<MySQLClient> mysql_client,
        std::shared_ptr<RedisClient> redis_client,
        std::shared_ptr<ServerTime> server_time,
        std::shared_ptr<SessionHandler> session_handler,
        std::shared_ptr<UserRepositoryHandler> repository_handler,
        std::shared_ptr<CommunicationHandler> communication_handler
        );
    ~UserManagementService();


    // High-level business operations（通信の詳細を知らない）
    bool process_accept_new_user();
    void process_user_disconnection(socket_t socket);
    void update_user_session(socket_t socket, const std::unique_ptr<SessionRequestMessage>& session_message);
    void process_heartbeat(socket_t socket);
    void start_heartbeat_monitoring();
    void cleanup_inactive_users();
    
    // Session management operations (ChatServiceから移行)
    SessionResult authenticate_session(const std::unique_ptr<SessionRequestMessage>& session_message, socket_t socket);
    bool send_session_response(
        const std::unique_ptr<SessionRequestMessage>& session_message,
        const SessionResult& session_result,
        socket_t socket,
        const std::string& uid
    );

    bool process_session_request(const json& session_data, socket_t socket);
    
private:
    bool can_accept_new_connection();  // ビジネスルールのみ
    void log_connection_event(const std::string& uid, const std::string& event);
    

};
#endif // USERMANAGEMENTSERVICE_HPP