#ifndef USERREPOSITORYHANDLER_HPP
#define USERREPOSITORYHANDLER_HPP
#include "Utils/common.hpp"
#include "Domain/UserAggregateStore.hpp"
#include "Domain/Entity/User.hpp"

class UserRepositoryHandler
{
private:
    std::shared_ptr<UserAggregateStore> _store;
    
public:
    explicit UserRepositoryHandler(std::shared_ptr<UserAggregateStore> store) 
        : _store(store) {}
    
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