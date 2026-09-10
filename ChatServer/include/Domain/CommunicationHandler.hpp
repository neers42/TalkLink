#ifndef COMMUNICATIONHANDLER_HPP
#define COMMUNICATIONHANDLER_HPP
#include "Utils/common.hpp"
#include "Domain/UserAggregateStore.hpp"

class CommunicationHandler {

    struct AcceptResult {
        socket_t socket;
        SSL* ssl;
        struct sockaddr_in user_addr;
        std::string user_ip_addr;
        
        bool has_value() const {
            return socket != INVALID_SOCKET_FD && ssl != nullptr && !user_ip_addr.empty();
        }
    };
private:
    std::shared_ptr<UserAggregateStore> _store;
    
    // 通信関連のインフラを全て管理
    std::shared_ptr<SocketHandler> _socket_handler;      // ソケット操作
    std::shared_ptr<TLSConnectionHandler> _tlscon_handler;  // SSL/プロトコル
    
public:
    explicit CommunicationHandler(
        std::shared_ptr<UserAggregateStore> store,
        std::shared_ptr<SocketHandler> socket_handler,
        std::shared_ptr<TLSConnectionHandler> tlscon_handler) 
        : _store(store), 
          _socket_handler(socket_handler),
          _tlscon_handler(tlscon_handler) {}
    
    // 低レベルな通信操作（UserRepositoryHandlerから呼び出される）
    AcceptResult accept_new_connection();
    bool close_connection(socket_t socket);
    
    // メッセージ操作
    bool send_message(socket_t socket, const json& message);
    std::tuple<json, bool, bool> receive_message(socket_t socket);
    
    // SSL操作
    bool shutdown_ssl(socket_t socket);
    
    // ネットワーク情報の取得
    std::string get_ip_address(socket_t socket);
    bool is_connection_active(socket_t socket);
    
    // サーバー操作
    bool open_listen_socket();
    socket_t get_listen_socket();
    bool close_listen_socket();
};
#endif // COMMUNICATIONHANDLER_HPP