#include "Domain/CommunicationHandler.hpp"

CommunicationHandler::AcceptResult CommunicationHandler::accept_new_connection()
{
    LOG_TRACE;
    socket_t new_user_socket = INVALID_SOCKET_FD;
    struct sockaddr_in new_user_sockaddr_in = {0};
    SSL* new_user_ssl = nullptr;
    std::string user_ip_addr;

    try {
        // 1. TCPソケット受け入れ (SocketHandlerに委譲)
        std::tie(new_user_socket, new_user_sockaddr_in) = 
            _socket_handler->accept_user_socket(sizeof(new_user_sockaddr_in));
            
        if (new_user_socket == INVALID_SOCKET_FD) {
            LOG_ERROR("[CommunicationHandler] Failed to accept user socket");
            return {INVALID_SOCKET_FD, nullptr, {}, ""};
        }

        LOG_DEBUG("[CommunicationHandler] Successfully accepted user socket: {}", new_user_socket);

        // 2. IPアドレス抽出
        user_ip_addr = _socket_handler->extraction_user_ip_address(new_user_sockaddr_in, 64);
        if (user_ip_addr.empty()) {
            LOG_ERROR("[CommunicationHandler] Failed to extract user IP address");
            _socket_handler->close_socket(new_user_socket);
            return {INVALID_SOCKET_FD, nullptr, {}, ""};
        }

        // 3. SSL/TLSハンドシェイク (ProtocolHandlerに委譲)
        // Note: SSLハンドシェイクはブロッキングモードで実行し、完了後に非ブロッキングに設定
        new_user_ssl = _tlscon_handler->ssl_accept(new_user_socket);
        if (new_user_ssl == nullptr) {
            LOG_ERROR("[CommunicationHandler] SSL handshake failed for socket: {}", new_user_socket);
            _socket_handler->close_socket(new_user_socket);
            return {INVALID_SOCKET_FD, nullptr, {}, ""};
        }

        // 4. ソケットをノンブロッキングモードに設定 (SSL完了後に設定)
#ifdef USE_POLL  
        int flags = fcntl(new_user_socket, F_GETFL, 0);
        if (flags == -1) {
            LOG_ERROR("[CommunicationHandler] fcntl F_GETFL failed for socket: {}, error: {}", new_user_socket, strerror(errno));
            SSL_free(new_user_ssl);
            _socket_handler->close_socket(new_user_socket);
            return {INVALID_SOCKET_FD, nullptr, {}, ""};
        }
        
        LOG_DEBUG("[CommunicationHandler] Socket {} set to non-blocking mode after SSL handshake", new_user_socket);
#endif

        LOG_DEBUG("[CommunicationHandler] SSL handshake successful for user: {} (socket: {})", user_ip_addr, new_user_socket);
        _tlscon_handler->get_peer_cetificate(new_user_ssl);

        // 5. 新しいクライアントソケットをepollに登録
        _socket_handler->add_epoll_ctrl(new_user_socket);
        LOG_DEBUG("[CommunicationHandler] User socket {} added to epoll monitoring", new_user_socket);

        return {new_user_socket, new_user_ssl, new_user_sockaddr_in, user_ip_addr};

    } catch (const std::exception& e) {
        LOG_ERROR("[CommunicationHandler] Exception in accept_new_connection: {}", e.what());
        if (new_user_ssl) {
            SSL_free(new_user_ssl);
        }
        if (new_user_socket != INVALID_SOCKET_FD) {
            _socket_handler->close_socket(new_user_socket);
        }
        return {INVALID_SOCKET_FD, nullptr, {}, ""};
    }
}

bool CommunicationHandler::close_connection(socket_t socket)
{
    LOG_TRACE;
    try {
        std::unique_lock<std::shared_mutex> lock(_store->get_mutex());
        
        // UserAggregateStoreから情報取得 (friend access)
        auto it = _store->get_aggregates().find(socket);
        if (it == _store->get_aggregates().end()) {
            LOG_ERROR("[CommunicationHandler] User not found for socket {}", socket);
            return false;
        }

        std::string user_ip_addr = it->second.ipv4_address;
        SSL* ssl = it->second.ssl;

        // SSL/TLS接続終了
        if (ssl) {
            bool ssl_result = _tlscon_handler->shutdown_ssl(ssl);
            if (!ssl_result) {
                LOG_WARNING("[CommunicationHandler] SSL shutdown failed for socket: {}", socket);
            }
        }

        // ソケットクローズ
        _socket_handler->close_socket(socket);
        
        LOG_INFO("[CommunicationHandler] Connection closed for user: {} (socket: {})", user_ip_addr, socket);
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("[CommunicationHandler] Exception in close_connection: {}", e.what());
        return false;
    }
}

bool CommunicationHandler::send_message(socket_t socket, const json &message)
{
    LOG_TRACE;
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        // UserAggregateStoreからSSL情報取得 (friend access)
        auto it = _store->get_aggregates().find(socket);
        if (it == _store->get_aggregates().end()) {
            LOG_ERROR("[CommunicationHandler] User not found for socket {}", socket);
            return false;
        }

        SSL* ssl = it->second.ssl;
        if (!ssl) {
            LOG_ERROR("[CommunicationHandler] SSL connection is null for socket {}", socket);
            return false;
        }

        // UserAggregateの同期制御
        std::lock_guard<std::mutex> aggregate_lock(_store->get_id_map_mutex());
        
        // ProtocolHandlerに委譲
        bool result = _tlscon_handler->send_message_json(ssl, message);
        
        if (result) {
            LOG_DEBUG("[CommunicationHandler] Message sent successfully to user: {} (socket: {})", it->second.ipv4_address, socket);
            
            // 最終活動時間更新
            it->second.last_activity = std::chrono::steady_clock::now();
        } else {
            LOG_ERROR("[CommunicationHandler] Failed to send message to user: {} (socket: {})", it->second.ipv4_address, socket);
        }

        return result;

    } catch (const std::exception& e) {
        LOG_ERROR("[CommunicationHandler] Exception in send_message: {}", e.what());
        return false;
    }
}

std::tuple<json, bool, bool> CommunicationHandler::receive_message(socket_t socket)
{
    LOG_TRACE;
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        // UserAggregateStoreからSSL情報取得 (friend access)
        auto it = _store->get_aggregates().find(socket);
        if (it == _store->get_aggregates().end()) {
            LOG_ERROR("[CommunicationHandler] User not found for socket {}", socket);
            return {json{}, true, false}; // disconnect flag = true
        }

        SSL* ssl = it->second.ssl;
        if (!ssl) {
            LOG_ERROR("[CommunicationHandler] SSL connection is null for socket {}", socket);
            return {json{}, true, false};
        }

        // UserAggregateの同期制御
        std::lock_guard<std::mutex> aggregate_lock(_store->get_id_map_mutex());
        
        // ProtocolHandlerに委譲
        auto [recv_json, disconnect_flag, recv_flag] = _tlscon_handler->receive_message_json(ssl);
    
        if (recv_flag && !disconnect_flag) {
            // 最終活動時間更新
            it->second.last_activity = std::chrono::steady_clock::now();
            LOG_DEBUG("[CommunicationHandler] Message received successfully from user: {} (socket: {})", it->second.ipv4_address, socket);
        } else if (disconnect_flag) {
            LOG_INFO("[CommunicationHandler] User disconnection detected for user: {} (socket: {})", it->second.ipv4_address, socket);
        } else {
            LOG_WARNING("[CommunicationHandler] Unexpected state for user: {} (socket: {}) - recv_flag={}, disconnect_flag={}", 
                       it->second.ipv4_address, socket, recv_flag, disconnect_flag);
        }
        return {recv_json, disconnect_flag, recv_flag};

    } catch (const std::exception& e) {
        LOG_ERROR("[CommunicationHandler] Exception in receive_message: {}", e.what());
        return {json{}, true, false};
    }
}

bool CommunicationHandler::shutdown_ssl(socket_t socket)
{
    LOG_TRACE;
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        // UserAggregateStoreからSSL情報取得 (friend access)
        auto it = _store->get_aggregates().find(socket);
        if (it == _store->get_aggregates().end()) {
            LOG_ERROR("[CommunicationHandler] User not found for socket {}", socket);
            return false;
        }

        SSL* ssl = it->second.ssl;
        if (!ssl) {
            LOG_WARNING("[CommunicationHandler] SSL connection is already null for socket {}", socket);
            return true; // already closed
        }

        // ProtocolHandlerに委譲
        bool result = _tlscon_handler->shutdown_ssl(ssl);
        
        if (result) {
            LOG_INFO("[CommunicationHandler] SSL shutdown successful for socket: {}", socket);
        } else {
            LOG_WARNING("[CommunicationHandler] SSL shutdown failed for socket: {}", socket);
        }

        return result;

    } catch (const std::exception& e) {
        LOG_ERROR("[CommunicationHandler] Exception in shutdown_ssl: {}", e.what());
        return false;
    }
}

// Network info
std::string CommunicationHandler::get_ip_address(socket_t socket)
{
    LOG_TRACE;
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        // UserAggregateStoreからIP情報取得 (friend access)
        auto it = _store->get_aggregates().find(socket);
        if (it == _store->get_aggregates().end()) {
            LOG_ERROR("[CommunicationHandler] User not found for socket {}", socket);
            return "";
        }

        return it->second.ipv4_address;

    } catch (const std::exception& e) {
        LOG_ERROR("[CommunicationHandler] Exception in get_ip_address: {}", e.what());
        return "";
    }
}

bool CommunicationHandler::is_connection_active(socket_t socket)
{
    LOG_TRACE;
    try {
        // SocketHandlerでソケット状態確認
        bool socket_active = !_socket_handler->is_socket_closed(socket);
        
        if (!socket_active) {
            return false;
        }

        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        // UserAggregateStoreで存在確認 (friend access)
        auto it = _store->get_aggregates().find(socket);
        if (it == _store->get_aggregates().end()) {
            return false;
        }

        // クライアントの接続状態確認
        return it->second.user && 
               it->second.user->is_active() && 
               it->second.ssl != nullptr;

    } catch (const std::exception& e) {
        LOG_ERROR("[CommunicationHandler] Exception in is_connection_active: {}", e.what());
        return false;
    }
}

bool CommunicationHandler::open_listen_socket()
{
    LOG_TRACE;
    try {
        // SocketHandlerに委譲
        bool result = _socket_handler->open_tcp_listen_socket();
        
        if (result) {
            LOG_INFO("[CommunicationHandler] TCP listen socket opened successfully");
        } else {
            LOG_ERROR("[CommunicationHandler] Failed to open TCP listen socket");
        }

        return result;

    } catch (const std::exception& e) {
        LOG_ERROR("[CommunicationHandler] Exception in open_listen_socket: {}", e.what());
        return false;
    }
}

socket_t CommunicationHandler::get_listen_socket()
{
    LOG_TRACE;
    try {
        // SocketHandlerから取得
        return _socket_handler->get_listen_socket();

    } catch (const std::exception& e) {
        LOG_ERROR("[CommunicationHandler] Exception in get_listen_socket: {}", e.what());
        return INVALID_SOCKET_FD;
    }
}

bool CommunicationHandler::close_listen_socket()
{
    LOG_TRACE;
    try {
        socket_t listen_socket = _socket_handler->get_listen_socket();
        if (listen_socket == INVALID_SOCKET_FD) {
            LOG_WARNING("[CommunicationHandler] Listen socket is already closed");
            return true;
        }

        // SocketHandlerに委譲
        _socket_handler->close_socket(listen_socket);
        LOG_INFO("[CommunicationHandler] Listen socket closed successfully");
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("[CommunicationHandler] Exception in close_listen_socket: {}", e.what());
        return false;
    }
}
