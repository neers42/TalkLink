#ifndef CLIENTMANAGEMENTSERVICE_HPP
#define CLIENTMANAGEMENTSERVICE_HPP
#include "Utils/common.hpp"
#include "Infrastructure/MySQLClient.hpp"
#include "Infrastructure/RedisClient.hpp"
#include "Domain/ClientCommunicationHandler.hpp"
#include "Domain/ClientRepositoryHandler.hpp"
#include "Domain/SessionHandler.hpp"
#include "Domain/Entity/Client.hpp"
#include "Domain/ClientAggregateStore.hpp"
#include "Domain/Entity/Messages/SessionRequestMessage.hpp"
#include "Domain/Entity/Messages/SessionResponseMessage.hpp"

// ChatServiceから共通のSessionResult構造体を使用
struct SessionResult {
    bool verified = false;
    std::string error_message;
    json unreceived_messages;
};

class ClientManagementService {
private:
    // データアクセス制御層のみ
    std::shared_ptr<ClientRepositoryHandler> _repository;
    std::shared_ptr<ClientCommunicationHandler> _communication;
    std::shared_ptr<SessionHandler> _session_handler;
    
    // ビジネス層のインフラのみ
    std::shared_ptr<MySQLClient> _mysql_client;  // DB操作
    std::shared_ptr<RedisClient> _redis_client;  // セッション管理・キャッシュ
    std::shared_ptr<ServerTime> _server_time;    // 時刻管理
    
    
public:
    ClientManagementService(
        std::shared_ptr<MySQLClient> mysql_client,
        std::shared_ptr<RedisClient> redis_client,
        std::shared_ptr<ServerTime> server_time,
        std::shared_ptr<SessionHandler> session_handler,
        std::shared_ptr<ClientRepositoryHandler> repository_handler,
        std::shared_ptr<ClientCommunicationHandler> communication_handler
        );
    ~ClientManagementService();


    // High-level business operations（通信の詳細を知らない）
    bool process_accept_new_client();
    void process_client_disconnection(socket_t socket);
    void update_client_session(socket_t socket, const std::unique_ptr<SessionRequestMessage>& session_message);
    void process_heartbeat(socket_t socket);
    void start_heartbeat_monitoring();
    void cleanup_inactive_clients();
    
    // Session management operations (ChatServiceから移行)
    SessionResult authenticate_session(const std::unique_ptr<SessionRequestMessage>& session_message, socket_t socket);
    bool send_session_response(
        const std::unique_ptr<SessionRequestMessage>& session_message,
        const SessionResult& session_result,
        socket_t socket,
        const std::string& client_id
    );

    bool process_session_request(const json& session_data, socket_t socket);
    
private:
    bool can_accept_new_connection();  // ビジネスルールのみ
    void log_connection_event(const std::string& client_id, const std::string& event);
    

};
#endif // CLIENTMANAGEMENTSERVICE_HPP