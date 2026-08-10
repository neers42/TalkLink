#ifndef USERREPOSITORYHANDLER_HPP
#define USERREPOSITORYHANDLER_HPP
#include "Utils/common.hpp"
#include "Domain/UserAggregateStore.hpp"
#include "Domain/Entity/User.hpp"
#include "Infrastructure/MySQLAdapter.hpp"

struct SearchResult {
    std::vector<UserInfo> user_infos;
    std::string error_message;

    SearchResult() : error_message("") {}
};
class UserRepositoryHandler
{
private:
    std::shared_ptr<UserAggregateStore> _store;
    std::shared_ptr<MySQLAdapter> _mysql_adapter; // DBアクセス用アダプター
    std::shared_ptr<ServerTime> _server_time; // 時刻管理用ユーティリティ
    
public:
    explicit UserRepositoryHandler(std::shared_ptr<UserAggregateStore> store, std::shared_ptr<MySQLAdapter> mysql_adapter, std::shared_ptr<ServerTime> server_time) 
        : _store(store), _mysql_adapter(mysql_adapter), _server_time(server_time) {}
    
    // クライアント管理操作
    bool add_user(socket_t socket, SSL* ssl, const sockaddr_in& addr,
                   std::shared_ptr<User> user, const std::string& ip);
    bool add_user(socket_t socket, SSL* ssl, const sockaddr_in& addr,
                   const std::string& ip); // クライアント情報なしのオーバーロード
    bool remove_user(socket_t socket);
    bool update_user_info(socket_t socket, const std::string& id, 
                          const std::string& name, const std::string& email);
    
    // 部分更新メソッド（効率的な更新）
    bool update_user_activity(socket_t socket, time_t last_activity);
    bool update_user_heartbeat(socket_t socket);
    bool update_user_auth_status(socket_t socket, bool auth_status);
    
    // クエリ操作 (UserManagementServiceに公開)
    std::shared_ptr<User> get_user(socket_t socket);
    std::shared_ptr<User> get_user_by_id(const std::string& uid);
    socket_t get_socket_by_uid(const std::string& uid);
    std::shared_ptr<User> get_user_by_socket(socket_t socket);
    std::string get_uid_by_socket(socket_t socket);
    std::string get_uid_by_socket(const std::shared_ptr<User>& user);

    //クエリ操作(DB)
    SearchResult search_users(const std::string& query);
    
    // 統計情報
    size_t get_connected_user_count() const;
    std::vector<socket_t> get_all_sockets();
    std::vector<socket_t> get_inactive_users(std::chrono::seconds timeout);
    std::map<socket_t, std::shared_ptr<User>> get_all_users() const;
    
    // その他の操作
    void update_all_last_activity();
    void clear_all_heartbeats();
    
private:
    // Store内部操作のヘルパー
    UserAggregateStore::UserAggregate* get_aggregate(socket_t socket);
    bool validate_user_data(const std::shared_ptr<User>& user);

};
#endif // USERREPOSITORYHANDLER_HPP