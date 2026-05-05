#include "Application/Services/ClientManagementService.hpp"

// Constructor
ClientManagementService::ClientManagementService(
    std::shared_ptr<MySQLClient> mysql_client,
    std::shared_ptr<RedisClient> redis_client,
    std::shared_ptr<ServerTime> server_time,
    std::shared_ptr<SessionHandler> session_handler,
    std::shared_ptr<ClientRepositoryHandler> repository_handler,
    std::shared_ptr<ClientCommunicationHandler> communication_handler
  )
    : _mysql_client(mysql_client),
    _redis_client(redis_client), 
    _server_time(server_time),
    _session_handler(session_handler),
    _repository(repository_handler),
    _communication(communication_handler)
{
    if (!_repository || !_communication || !_session_handler || !_mysql_client || !_redis_client || !_server_time) {
        throw std::runtime_error("Failed to initialize ClientManagementService dependencies");
    }
    
    LOG_INFO("[ClientManagementService] Service initialized successfully with injected dependencies");
}

// Destructor
ClientManagementService::~ClientManagementService() = default;

// Public API Methods

bool ClientManagementService::process_accept_new_client() {
    try {
        LOG_DEBUG("[ClientManagementService] Attempting to accept new client connection");
        
        // ビジネスルール：接続数制限チェック
        if (!can_accept_new_connection()) {
            LOG_WARNING("[ClientManagementService] Cannot accept new client - connection limit reached");
            return false;
        }

        // 通信層に委譲
        auto result = _communication->accept_new_connection();
        if (result.has_value()) {
            LOG_DEBUG("[ClientManagementService] Communication layer accepted connection - socket: {}, ip: {}", result.socket, result.client_ip);
            
            // クライアント情報をリポジトリに保存
            if (_repository->add_client(result.socket, result.ssl, result.client_addr, result.client_ip)) {
                size_t total_after_add = _repository->get_connected_count();
                LOG_INFO("[ClientManagementService] New client added to repository - socket: {}, ip: {}, total_clients: {}", 
                         result.socket, result.client_ip, total_after_add);
            } else {
                LOG_ERROR("[ClientManagementService] Failed to add new client to repository - socket: {}, ip: {}", result.socket, result.client_ip);
                _communication->close_connection(result.socket); // クリーンアップ
                return false;
            }
            return true;
        } else {
            LOG_WARNING("[ClientManagementService] Failed to accept new client connection");
            return false;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("[ClientManagementService] Exception in process_accept_new_client: {}", e.what());
        return false;
    }
}

void ClientManagementService::process_client_disconnection(socket_t socket) {
    try {
        // デバッグ：リポジトリの現在の状態を確認
        size_t total_clients = _repository->get_connected_count();
        LOG_DEBUG("[ClientManagementService] Disconnection attempt - socket: {}, total_clients_in_repo: {}", socket, total_clients);
        
        // クライアント情報取得（ビジネス検証）
        auto client = _repository->get_client(socket);
        if (client == nullptr) {
            LOG_WARNING("[ClientManagementService] Attempting to disconnect unknown client on socket: {} | Total clients: {} | Possible causes: registration failed, already cleaned up, or race condition", 
                       socket, total_clients);
            
            // 強制的に通信層の接続をクローズ（安全処理）
            try {
                _communication->close_connection(socket);
                LOG_INFO("[ClientManagementService] Forced close connection for unknown socket: {}", socket);
            } catch (const std::exception& e) {
                LOG_ERROR("[ClientManagementService] Failed to force close unknown socket {}: {}", socket, e.what());
            }
            return;
        }

        // ビジネスルール：安全な切断チェック
        if (!client->can_disconnect()) {
            LOG_WARNING("[ClientManagementService] Client {} cannot be safely disconnected at this time", client->get_id());
        }
        
        // ログ記録（切断前）
        log_connection_event(client->get_id(), "disconnecting");

        // 接続クリーンアップを通信層に委譲
        _communication->close_connection(socket);
        
        // データ削除をリポジトリ層に委譲
        _repository->remove_client(socket);
        
        LOG_INFO("[ClientManagementService] Client {} disconnected successfully", client->get_id());
        
    } catch (const std::exception& e) {
        LOG_ERROR("[ClientManagementService] Exception in process_client_disconnection for socket {}: {}", socket, e.what());
    }
}

void ClientManagementService::update_client_session(socket_t socket, const std::unique_ptr<SessionRequestMessage>& session_message) {
    try {
        auto client = _repository->get_client(socket);
        if (client == nullptr) {
            LOG_WARNING("[ClientManagementService] Session update attempted on unknown socket: {}", socket);
            return;
        }

        // セッション情報の更新
        // client->update_session(info); // Client classにセッション管理メソッドが必要
        //_repository->update_client_info(socket, session_message->get_client_id(), session_message->get_client_name(), session_message->get_client_email());
        
        log_connection_event(client->get_id(), "session_updated");
        
    } catch (const std::exception& e) {
        LOG_ERROR("[ClientManagementService] Exception in update_client_session for socket {}: {}", socket, e.what());
    }
}

void ClientManagementService::process_heartbeat(socket_t socket) {
    try {
        LOG_INFO("[ClientManagementService] Processing heartbeat for socket: {}", socket);
        auto client = _repository->get_client(socket);
        if (client == nullptr) {
            LOG_WARNING("[ClientManagementService] Heartbeat from unknown socket: {} | Client may not be registered yet or already disconnected", socket);
            return;
        }

        // 効率的な部分更新を使用 - ハートビートカウントリセットと最終活動時間の更新
        bool update_success = _repository->update_client_heartbeat(socket);
        
        if (update_success) {
            LOG_DEBUG("[ClientManagementService] Heartbeat processed for client: {} (socket: {})", client->get_id(), socket);
        } else {
            LOG_WARNING("[ClientManagementService] Failed to update heartbeat for client: {} (socket: {})", client->get_id(), socket);
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("[ClientManagementService] Exception in process_heartbeat for socket {}: {}", socket, e.what());
    }
}

void ClientManagementService::start_heartbeat_monitoring() {
    try {
        LOG_INFO("[ClientManagementService] Starting heartbeat monitoring system");
        
        // 定期的なハートビート監視をバックグラウンドで開始
        // TODO: 実際のタイマータスクまたはスレッドプールでの実装
        // 現在は手動でcleanupInactiveClients()を呼び出すことを想定
        
        LOG_INFO("[ClientManagementService] Heartbeat monitoring system started successfully");
        
    } catch (const std::exception& e) {
        LOG_ERROR("[ClientManagementService] Exception in start_heartbeat_monitoring: {}", e.what());
    }
}

void ClientManagementService::cleanup_inactive_clients() {
    try {
        LOG_DEBUG("[ClientManagementService] Starting cleanup of inactive clients");
        
        auto all_clients = _repository->get_all_clients();
        size_t cleaned_count = 0;
        auto current_time = _server_time->get_current_unix_time();
        const time_t HEARTBEAT_TIMEOUT = 30; // 30秒のタイムアウト
        
        LOG_DEBUG("[ClientManagementService] Checking {} clients for timeout (threshold: {}s)", all_clients.size(), HEARTBEAT_TIMEOUT);
        
        for (const auto& [socket, client] : all_clients) {
            if (client == nullptr) {
                LOG_WARNING("[ClientManagementService] Found null client in repository during cleanup - socket: {}", socket);
                continue;
            }
            
            // ビジネスルール：ハートビートタイムアウト検証
            time_t inactive_duration = current_time - client->get_last_activity();
            LOG_TRACE("[ClientManagementService] Client {} (socket: {}) inactive for {} seconds", 
                     client->get_id(), socket, inactive_duration);
            
            if (inactive_duration > HEARTBEAT_TIMEOUT) {
                LOG_WARNING("[ClientManagementService] Client {} (socket: {}) inactive for {} seconds - scheduling for cleanup", 
                            client->get_id(), socket, inactive_duration);
                
                log_connection_event(client->get_id(), "timeout_cleanup");
                
                // 直接切断処理を行うのではなく、クリーンアップ前に再度存在確認
                auto recheck_client = _repository->get_client(socket);
                if (recheck_client != nullptr) {
                    // 安全な切断処理
                    process_client_disconnection(socket);
                    cleaned_count++;
                } else {
                    LOG_WARNING("[ClientManagementService] Client {} already removed from repository during cleanup check", client->get_id());
                }
            }
        }
        
        if (cleaned_count > 0) {
            LOG_INFO("[ClientManagementService] Cleaned up {} inactive clients", cleaned_count);
        } else {
            LOG_DEBUG("[ClientManagementService] No inactive clients found during cleanup");
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("[ClientManagementService] Exception in cleanup_inactive_clients: {}", e.what());
    }
}

// Private Helper Methods

bool ClientManagementService::can_accept_new_connection() {
    const size_t MAX_CONNECTIONS = 1000; // 設定から読み取るべき値
    
    try {
        size_t current_count = _repository->get_connected_count();
        return current_count < MAX_CONNECTIONS;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[ClientManagementService] Exception in can_accept_new_connection: {}", e.what());
        return false; // 安全側での判定
    }
}

void ClientManagementService::log_connection_event(const std::string& client_id, const std::string& event) {
    try {
        auto timestamp = _server_time->get_current_unix_time();
        auto connected_count = _repository->get_connected_count();
        
        LOG_INFO("[ClientManagementService] [{}] Client '{}' - Event: '{}' | Total Connected: {}", 
                     timestamp, client_id.empty() ? "unknown" : client_id, event, connected_count);
                    
    } catch (const std::exception& e) {
        LOG_ERROR("[ClientManagementService] Exception in log_connection_event: {}", e.what());
    }
}

// ==================== セッション管理  ====================

SessionResult ClientManagementService::authenticate_session(const std::unique_ptr<SessionRequestMessage>& session_message, socket_t socket) {
    SessionResult result = {};
    
    try {
        LOG_INFO("[ClientManagementService] Starting session authentication for socket: {}", socket);
        // セッション検証処理をDomain層のSessionHandlerに委譲
        auto validation_result = _session_handler->validate_session(session_message, socket);
        
        // SessionValidationResultをSessionResultに変換
        result.verified = validation_result.is_valid;
        result.error_message = validation_result.error_message;
        result.unreceived_messages = validation_result.unreceived_messages;

        _repository->update_client_info(socket, validation_result.client_id, validation_result.name, validation_result.email);
        
        if (result.verified) {
            LOG_INFO("[ClientManagementService] Session authentication successful for socket: {}, client_id: {}", 
                     socket, validation_result.client_id);
        } else {
            LOG_WARNING("[ClientManagementService] Session authentication failed for socket: {}: {}", 
                        socket, result.error_message);
        }
        
        return result;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[ClientManagementService] Exception in authenticate_session for socket {}: {}", socket, e.what());
        result.verified = false;
        result.error_message = "Internal authentication error";
        return result;
    }
}

bool ClientManagementService::send_session_response(
    const std::unique_ptr<SessionRequestMessage>& session_message,
    const SessionResult& session_result,
    socket_t socket,
    const std::string& client_id
) {
    try {
        // SessionResponseMessageを生成
        auto session_response = std::make_unique<SessionResponseMessage>(
            session_message->get_common_fields(),
            client_id,
            session_result.verified,
            session_result.error_message,
            session_result.unreceived_messages
        );
        
        // シリアライズして送信
        json session_response_json = MessageBase::serialize_message(session_response);
        
        if (session_response_json.empty()) {
            LOG_ERROR("[ClientManagementService] SessionResponse message serialization failed");
            return false;
        }

        bool send_success = _communication->send_message(socket, session_response_json);
        
        if (send_success) {
            LOG_INFO("[ClientManagementService] SessionResponse sent successfully to client (socket: {}, id: {})", socket, client_id);
        } else {
            LOG_ERROR("[ClientManagementService] Failed to send SessionResponse to client (socket: {}, id: {})", socket, client_id);
        }
        
        return send_success;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[ClientManagementService] Exception in send_session_response for socket {}: {}", socket, e.what());
        return false;
    }
}

bool ClientManagementService::process_session_request(const json &session_data, socket_t socket)
{
    try{
        LOG_INFO("[ClientManagementService] Processing session request for socket: {}", socket);
        auto session_message = MessageBase::deserialize_message<SessionRequestMessage>(session_data);
        // セッションリクエストの処理
        auto session_result = authenticate_session(session_message, socket);
        
        // セッションレスポンスの送信
        bool response_sent = send_session_response(session_message, session_result, socket, _repository->get_client_id(socket));
        
        return response_sent;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[ClientManagementService] Exception in process_session_request for socket {}: {}", socket, e.what());
        return false;
    }
}
