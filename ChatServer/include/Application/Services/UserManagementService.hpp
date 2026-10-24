#ifndef USERMANAGEMENTSERVICE_HPP
#define USERMANAGEMENTSERVICE_HPP
#include "Utils/common.hpp"
#include "Domain/DTO/UserInfo.hpp"

class CommunicationHandler;
class UserRepositoryHandler;
class SessionHandler;
class MySQLAdapter;
class RedisAdapter;
class ServerTime;

class SessionRequestMessage;
class UserSearchRequestMessage;

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
    std::shared_ptr<MySQLAdapter> _mysql_adapter;  // DB操作
    std::shared_ptr<RedisAdapter> _redis_adapter;  // セッション管理・キャッシュ
    std::shared_ptr<ServerTime> _server_time;    // 時刻管理

    // 設定(chat.confから取得)
    int _max_connections;
    int _heartbeat_timeout_sec;
    int _heartbeat_check_interval_sec;

    // ハートビート監視ループの制御
    std::atomic<bool> _running{true};
    std::mutex _heartbeat_mutex;
    std::condition_variable _heartbeat_cv;

public:
    UserManagementService(
        std::shared_ptr<ServerTime> server_time,
        std::shared_ptr<SessionHandler> session_handler,
        std::shared_ptr<UserRepositoryHandler> repository_handler,
        std::shared_ptr<CommunicationHandler> communication_handler,
        int max_connections,
        int heartbeat_timeout_sec,
        int heartbeat_check_interval_sec
        );
    ~UserManagementService();


    // High-level business operations（通信の詳細を知らない）
    bool process_accept_new_user();
    void process_user_disconnection(socket_t socket);
    void update_user_session(socket_t socket, const std::unique_ptr<SessionRequestMessage>& session_message);
    void process_heartbeat(socket_t socket);
    // バックグラウンドタスクとして呼び出す、ハートビート監視の常駐ループ
    void start_heartbeat_monitoring();
    // 常駐ループを停止する(デストラクタから呼び出される)
    void stop_heartbeat_monitoring();
    void cleanup_inactive_users();
    
    // メッセージ処理
    bool process_session_request(const json& session_data, socket_t socket);
    bool process_user_search(const json& query_data, socket_t socket);

    // 認証ゲート用: このソケットが現在認証済みかどうか
    bool is_socket_authenticated(socket_t socket);

    // システム監視用: 現在接続中のソケット数(未認証のハンドシェイク中も含む)
    size_t get_connected_user_count() const;

private:
    bool can_accept_new_connection();  // ビジネスルールのみ
    void log_connection_event(const std::string& uid, const std::string& event);

    // Session management operations (ChatServiceから移行)
    SessionResult authenticate_session(const std::unique_ptr<SessionRequestMessage>& session_message, socket_t socket);
    bool send_session_response(
        const std::unique_ptr<SessionRequestMessage>& session_message,
        const SessionResult& session_result,
        socket_t socket,
        const std::string& uid
    );

    bool send_user_search_response(
        const std::unique_ptr<UserSearchRequestMessage>& user_search_request,
        const std::vector<UserInfo>& user_infos,
        const std::string& error_message,
        socket_t socket
    );

    

};
#endif // USERMANAGEMENTSERVICE_HPP