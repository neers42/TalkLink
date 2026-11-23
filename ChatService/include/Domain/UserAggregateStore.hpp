#ifndef USERAGGREGATESTORE_HPP
#define USERAGGREGATESTORE_HPP

// ネットワーク関連のヘッダーを最初に読み込み
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <unordered_map>
#include <shared_mutex>

#include "Utils/common.hpp"
#include "Domain/Entity/User.hpp"
class UserAggregateStore {
public:
    struct UserAggregate {
        std::shared_ptr<User> user;
        socket_t socket;
        SSL* ssl;
        struct sockaddr_in address;
        std::string ipv4_address;
        std::chrono::steady_clock::time_point last_activity;
        
        UserAggregate(std::shared_ptr<User> user_ptr,
                        socket_t sock, SSL* ssl_ptr, const sockaddr_in& addr, 
                        const std::string& ip)
            : user(user_ptr), socket(sock), ssl(ssl_ptr), 
              address(addr), ipv4_address(ip),
              last_activity(std::chrono::steady_clock::now()) {}
        
        UserAggregate(socket_t sock, SSL* ssl_ptr, const sockaddr_in& addr, 
                        const std::string& ip)
            : user(std::make_shared<User>()), socket(sock), ssl(ssl_ptr), 
              address(addr), ipv4_address(ip),
              last_activity(std::chrono::steady_clock::now()) {}
    };

private:
    std::unordered_map<socket_t, UserAggregate> _aggregates;
    std::unordered_map<std::string, socket_t> _user_id_to_socket;
    mutable std::shared_mutex _shared_mutex;
    mutable std::mutex _id_map_mutex; // IDマッピング用の個別ミューテックス
    
    // UserRepositoryHandlerとCommunicationHandlerのみフレンド
    friend class UserRepositoryHandler;
    friend class CommunicationHandler;
    
public:
    UserAggregateStore() = default;
    ~UserAggregateStore() = default;
    
    // 外部からは基本的な統計情報のみ公開
    size_t size() const;
    bool empty() const;
    
private:
    // 内部操作（フレンドクラスのみアクセス可能）
    bool insert(socket_t socket, UserAggregate&& aggregate);
    bool erase(socket_t socket);
    UserAggregate* find(socket_t socket);
    const UserAggregate* find(socket_t socket) const;
    
    std::shared_mutex& get_mutex() { return _shared_mutex; }
    auto& get_aggregates() { return _aggregates; }
    auto& get_id_to_socket_map() { return _user_id_to_socket; }
    std::mutex& get_id_map_mutex() { return _id_map_mutex; }
};
#endif // USERAGGREGATESTORE_HPP