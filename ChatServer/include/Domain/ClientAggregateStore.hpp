#ifndef CLIENTAGGREGATESTORE_HPP
#define CLIENTAGGREGATESTORE_HPP

// ネットワーク関連のヘッダーを最初に読み込み
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <unordered_map>
#include <shared_mutex>

#include "Utils/common.hpp"
#include "Domain/Entity/Client.hpp"
class ClientAggregateStore {
public:
    struct ClientAggregate {
        std::shared_ptr<Client> client;
        socket_t socket;
        SSL* ssl;
        struct sockaddr_in address;
        std::string ipv4_address;
        std::chrono::steady_clock::time_point last_activity;
        
        ClientAggregate(std::shared_ptr<Client> client_ptr,
                        socket_t sock, SSL* ssl_ptr, const sockaddr_in& addr, 
                        const std::string& ip)
            : client(client_ptr), socket(sock), ssl(ssl_ptr), 
              address(addr), ipv4_address(ip),
              last_activity(std::chrono::steady_clock::now()) {}
        
        ClientAggregate(socket_t sock, SSL* ssl_ptr, const sockaddr_in& addr, 
                        const std::string& ip)
            : client(std::make_shared<Client>()), socket(sock), ssl(ssl_ptr), 
              address(addr), ipv4_address(ip),
              last_activity(std::chrono::steady_clock::now()) {}
    };

private:
    std::unordered_map<socket_t, ClientAggregate> _aggregates;
    std::unordered_map<std::string, socket_t> _client_id_to_socket;
    mutable std::shared_mutex _shared_mutex;
    mutable std::mutex _id_map_mutex; // IDマッピング用の個別ミューテックス
    
    // ClientRepositoryHandlerとClientCommunicationHandlerのみフレンド
    friend class ClientRepositoryHandler;
    friend class ClientCommunicationHandler;
    
public:
    ClientAggregateStore() = default;
    ~ClientAggregateStore() = default;
    
    // 外部からは基本的な統計情報のみ公開
    size_t size() const;
    bool empty() const;
    
private:
    // 内部操作（フレンドクラスのみアクセス可能）
    bool insert(socket_t socket, ClientAggregate&& aggregate);
    bool erase(socket_t socket);
    ClientAggregate* find(socket_t socket);
    const ClientAggregate* find(socket_t socket) const;
    
    std::shared_mutex& get_mutex() { return _shared_mutex; }
    auto& get_aggregates() { return _aggregates; }
    auto& get_id_to_socket_map() { return _client_id_to_socket; }
    std::mutex& get_id_map_mutex() { return _id_map_mutex; }
};
#endif // CLIENTAGGREGATESTORE_HPP