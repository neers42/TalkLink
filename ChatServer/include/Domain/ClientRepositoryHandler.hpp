#ifndef CLIENTREPOSITORYHANDLER_HPP
#define CLIENTREPOSITORYHANDLER_HPP
#include "Utils/common.hpp"
#include "Domain/ClientAggregateStore.hpp"
#include "Domain/Entity/Client.hpp"

class ClientRepositoryHandler
{
private:
    std::shared_ptr<ClientAggregateStore> _store;
    
public:
    explicit ClientRepositoryHandler(std::shared_ptr<ClientAggregateStore> store) 
        : _store(store) {}
    
    // クライアント管理操作
    bool add_client(socket_t socket, SSL* ssl, const sockaddr_in& addr,
                   std::shared_ptr<Client> client, const std::string& ip);
    bool add_client(socket_t socket, SSL* ssl, const sockaddr_in& addr,
                   const std::string& ip); // クライアント情報なしのオーバーロード
    bool remove_client(socket_t socket);
    bool update_client_info(socket_t socket, const std::string& id, 
                          const std::string& name, const std::string& email);
    
    // 部分更新メソッド（効率的な更新）
    bool update_client_activity(socket_t socket, time_t last_activity);
    bool update_client_heartbeat(socket_t socket);
    bool update_client_auth_status(socket_t socket, bool auth_status);
    
    // クエリ操作 (ClientManagementServiceに公開)
    std::shared_ptr<Client> get_client(socket_t socket);
    std::shared_ptr<Client> get_client_by_id(const std::string& client_id);
    socket_t get_socket_by_client_id(const std::string& client_id);
    std::shared_ptr<Client> get_client_by_socket(socket_t socket);
    std::string get_client_id(socket_t socket);
    std::string get_client_id(const std::shared_ptr<Client>& client);
    
    // 統計情報
    size_t get_connected_count() const;
    std::vector<socket_t> get_all_sockets();
    std::vector<socket_t> get_inactive_clients(std::chrono::seconds timeout);
    std::map<socket_t, std::shared_ptr<Client>> get_all_clients() const;
    
    // その他の操作
    void update_all_last_activity();
    void clear_all_heartbeats();
    
private:
    // Store内部操作のヘルパー
    ClientAggregateStore::ClientAggregate* get_aggregate(socket_t socket);
    bool validate_client_data(const std::shared_ptr<Client>& client);

};
#endif // CLIENTREPOSITORYHANDLER_HPP