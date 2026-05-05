#include "Domain/ClientCommunicationHandler.hpp"

ClientCommunicationHandler::AcceptResult ClientCommunicationHandler::accept_new_connection()
{
    LOG_TRACE;
    socket_t new_client_socket = INVALID_SOCKET_FD;
    struct sockaddr_in new_client_sockaddr_in = {0};
    SSL* new_client_ssl = nullptr;
    std::string client_ip;

    try {
        // 1. TCPソケット受け入れ (SocketHandlerに委譲)
        std::tie(new_client_socket, new_client_sockaddr_in) = 
            _socket_handler->accept_client_socket(sizeof(new_client_sockaddr_in));
            
        if (new_client_socket == INVALID_SOCKET_FD) {
            LOG_ERROR("Failed to accept client socket");
            return {INVALID_SOCKET_FD, nullptr, {}, ""};
        }

        LOG_DEBUG("[ClientCommunicationHandler] Successfully accepted client socket: {}", new_client_socket);

        // 2. IPアドレス抽出
        client_ip = _socket_handler->extraction_client_ip(new_client_sockaddr_in, 64);
        if (client_ip.empty()) {
            LOG_ERROR("[ClientCommunicationHandler] Failed to extract client IP address");
            _socket_handler->close_socket(new_client_socket);
            return {INVALID_SOCKET_FD, nullptr, {}, ""};
        }

        // 3. SSL/TLSハンドシェイク (ProtocolHandlerに委譲)
        // Note: SSLハンドシェイクはブロッキングモードで実行し、完了後に非ブロッキングに設定
        new_client_ssl = _protocol_handler->ssl_accept(new_client_socket);
        if (new_client_ssl == nullptr) {
            LOG_ERROR("[ClientCommunicationHandler] SSL handshake failed for socket: {}", new_client_socket);
            _socket_handler->close_socket(new_client_socket);
            return {INVALID_SOCKET_FD, nullptr, {}, ""};
        }

        // 4. ソケットをノンブロッキングモードに設定 (SSL完了後に設定)
#ifdef USE_POLL  
        int flags = fcntl(new_client_socket, F_GETFL, 0);
        if (flags == -1) {
            LOG_ERROR("[ClientCommunicationHandler] fcntl F_GETFL failed for socket: {}, error: {}", new_client_socket, strerror(errno));
            SSL_free(new_client_ssl);
            _socket_handler->close_socket(new_client_socket);
            return {INVALID_SOCKET_FD, nullptr, {}, ""};
        }
        
        LOG_DEBUG("[ClientCommunicationHandler] Socket {} set to non-blocking mode after SSL handshake", new_client_socket);
#endif

        LOG_DEBUG("[ClientCommunicationHandler] SSL handshake successful for client: {} (socket: {})", client_ip, new_client_socket);
        _protocol_handler->get_peer_cetificate(new_client_ssl);

        // 5. 新しいクライアントソケットをepollに登録
        _socket_handler->add_epoll_ctrl(new_client_socket);
        LOG_DEBUG("[ClientCommunicationHandler] Client socket {} added to epoll monitoring", new_client_socket);

        return {new_client_socket, new_client_ssl, new_client_sockaddr_in, client_ip};

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientCommunicationHandler] Exception in accept_new_connection: {}", e.what());
        if (new_client_ssl) {
            SSL_free(new_client_ssl);
        }
        if (new_client_socket != INVALID_SOCKET_FD) {
            _socket_handler->close_socket(new_client_socket);
        }
        return {INVALID_SOCKET_FD, nullptr, {}, ""};
    }
}

bool ClientCommunicationHandler::close_connection(socket_t socket)
{
    LOG_TRACE;
    try {
        std::unique_lock<std::shared_mutex> lock(_store->get_mutex());
        
        // ClientAggregateStoreから情報取得 (friend access)
        auto it = _store->get_aggregates().find(socket);
        if (it == _store->get_aggregates().end()) {
            LOG_ERROR("[ClientCommunicationHandler] Client not found for socket {}", socket);
            return false;
        }

        std::string client_ip = it->second.ipv4_address;
        SSL* ssl = it->second.ssl;

        // SSL/TLS接続終了
        if (ssl) {
            bool ssl_result = _protocol_handler->shutdown_ssl(ssl);
            if (!ssl_result) {
                LOG_WARNING("[ClientCommunicationHandler] SSL shutdown failed for socket: {}", socket);
            }
        }

        // ソケットクローズ
        _socket_handler->close_socket(socket);
        
        LOG_INFO("[ClientCommunicationHandler] Connection closed for client: {} (socket: {})", client_ip, socket);
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientCommunicationHandler] Exception in close_connection: {}", e.what());
        return false;
    }
}

bool ClientCommunicationHandler::send_message(socket_t socket, const json &message)
{
    LOG_TRACE;
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        // ClientAggregateStoreからSSL情報取得 (friend access)
        auto it = _store->get_aggregates().find(socket);
        if (it == _store->get_aggregates().end()) {
            LOG_ERROR("[ClientCommunicationHandler] Client not found for socket {}", socket);
            return false;
        }

        SSL* ssl = it->second.ssl;
        if (!ssl) {
            LOG_ERROR("[ClientCommunicationHandler] SSL connection is null for socket {}", socket);
            return false;
        }

        // ClientAggregateの同期制御
        std::lock_guard<std::mutex> aggregate_lock(_store->get_id_map_mutex());
        
        // ProtocolHandlerに委譲
        bool result = _protocol_handler->send_message_json(ssl, message);
        
        if (result) {
            LOG_DEBUG("[ClientCommunicationHandler] Message sent successfully to socket: {}", socket);
            
            // 最終活動時間更新
            it->second.last_activity = std::chrono::steady_clock::now();
        } else {
            LOG_ERROR("[ClientCommunicationHandler] Failed to send message to socket: {}", socket);
        }

        return result;

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientCommunicationHandler] Exception in send_message: {}", e.what());
        return false;
    }
}

std::tuple<json, bool, bool> ClientCommunicationHandler::receive_message(socket_t socket)
{
    LOG_TRACE;
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        // ClientAggregateStoreからSSL情報取得 (friend access)
        auto it = _store->get_aggregates().find(socket);
        if (it == _store->get_aggregates().end()) {
            LOG_ERROR("[ClientCommunicationHandler] Client not found for socket {}", socket);
            return {json{}, true, false}; // disconnect flag = true
        }

        SSL* ssl = it->second.ssl;
        if (!ssl) {
            LOG_ERROR("[ClientCommunicationHandler] SSL connection is null for socket {}", socket);
            return {json{}, true, false};
        }

        // ClientAggregateの同期制御
        std::lock_guard<std::mutex> aggregate_lock(_store->get_id_map_mutex());
        
        // ProtocolHandlerに委譲
        auto [recv_json, disconnect_flag, recv_flag] = _protocol_handler->receive_message_json(ssl);
    
        if (recv_flag && !disconnect_flag) {
            // 最終活動時間更新
            it->second.last_activity = std::chrono::steady_clock::now();
            LOG_DEBUG("[ClientCommunicationHandler] Message received successfully from socket: {}", socket);
        } else if (disconnect_flag) {
            LOG_INFO("[ClientCommunicationHandler] Client disconnection detected on socket: {}", socket);
        } else {
            LOG_WARNING("[ClientCommunicationHandler] Unexpected state for socket: {} - recv_flag={}, disconnect_flag={}", 
                       socket, recv_flag, disconnect_flag);
        }
        return {recv_json, disconnect_flag, recv_flag};

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientCommunicationHandler] Exception in receive_message: {}", e.what());
        return {json{}, true, false};
    }
}

bool ClientCommunicationHandler::shutdown_ssl(socket_t socket)
{
    LOG_TRACE;
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        // ClientAggregateStoreからSSL情報取得 (friend access)
        auto it = _store->get_aggregates().find(socket);
        if (it == _store->get_aggregates().end()) {
            LOG_ERROR("[ClientCommunicationHandler] Client not found for socket {}", socket);
            return false;
        }

        SSL* ssl = it->second.ssl;
        if (!ssl) {
            LOG_WARNING("[ClientCommunicationHandler] SSL connection is already null for socket {}", socket);
            return true; // already closed
        }

        // ProtocolHandlerに委譲
        bool result = _protocol_handler->shutdown_ssl(ssl);
        
        if (result) {
            LOG_INFO("[ClientCommunicationHandler] SSL shutdown successful for socket: {}", socket);
        } else {
            LOG_WARNING("[ClientCommunicationHandler] SSL shutdown failed for socket: {}", socket);
        }

        return result;

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientCommunicationHandler] Exception in shutdown_ssl: {}", e.what());
        return false;
    }
}

// Network info
std::string ClientCommunicationHandler::get_ip_address(socket_t socket)
{
    LOG_TRACE;
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        // ClientAggregateStoreからIP情報取得 (friend access)
        auto it = _store->get_aggregates().find(socket);
        if (it == _store->get_aggregates().end()) {
            LOG_ERROR("[ClientCommunicationHandler] Client not found for socket {}", socket);
            return "";
        }

        return it->second.ipv4_address;

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientCommunicationHandler] Exception in get_ip_address: {}", e.what());
        return "";
    }
}

bool ClientCommunicationHandler::is_connection_active(socket_t socket)
{
    LOG_TRACE;
    try {
        // SocketHandlerでソケット状態確認
        bool socket_active = !_socket_handler->is_socket_closed(socket);
        
        if (!socket_active) {
            return false;
        }

        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        // ClientAggregateStoreで存在確認 (friend access)
        auto it = _store->get_aggregates().find(socket);
        if (it == _store->get_aggregates().end()) {
            return false;
        }

        // クライアントの接続状態確認
        return it->second.client && 
               it->second.client->is_active() && 
               it->second.ssl != nullptr;

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientCommunicationHandler] Exception in is_connection_active: {}", e.what());
        return false;
    }
}

bool ClientCommunicationHandler::open_listen_socket()
{
    LOG_TRACE;
    try {
        // SocketHandlerに委譲
        bool result = _socket_handler->open_tcp_listen_socket();
        
        if (result) {
            LOG_INFO("[ClientCommunicationHandler] TCP listen socket opened successfully");
        } else {
            LOG_ERROR("[ClientCommunicationHandler] Failed to open TCP listen socket");
        }

        return result;

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientCommunicationHandler] Exception in open_listen_socket: {}", e.what());
        return false;
    }
}

socket_t ClientCommunicationHandler::get_listen_socket()
{
    LOG_TRACE;
    try {
        // SocketHandlerから取得
        return _socket_handler->get_listen_socket();

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientCommunicationHandler] Exception in get_listen_socket: {}", e.what());
        return INVALID_SOCKET_FD;
    }
}

bool ClientCommunicationHandler::close_listen_socket()
{
    LOG_TRACE;
    try {
        socket_t listen_socket = _socket_handler->get_listen_socket();
        if (listen_socket == INVALID_SOCKET_FD) {
            LOG_WARNING("[ClientCommunicationHandler] Listen socket is already closed");
            return true;
        }

        // SocketHandlerに委譲
        _socket_handler->close_socket(listen_socket);
        LOG_INFO("[ClientCommunicationHandler] Listen socket closed successfully");
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientCommunicationHandler] Exception in close_listen_socket: {}", e.what());
        return false;
    }
}
