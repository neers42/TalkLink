#include "Application/Services/UserManagementService.hpp"

// Constructor
UserManagementService::UserManagementService(
    std::shared_ptr<MySQLClient> mysql_client,
    std::shared_ptr<RedisClient> redis_client,
    std::shared_ptr<ServerTime> server_time,
    std::shared_ptr<SessionHandler> session_handler,
    std::shared_ptr<UserRepositoryHandler> repository_handler,
    std::shared_ptr<CommunicationHandler> communication_handler
  )
    : _mysql_client(mysql_client),
    _redis_client(redis_client), 
    _server_time(server_time),
    _session_handler(session_handler),
    _repository(repository_handler),
    _communication(communication_handler)
{
    if (!_repository || !_communication || !_session_handler || !_mysql_client || !_redis_client || !_server_time) {
        throw std::runtime_error("Failed to initialize UserManagementService dependencies");
    }
    
    LOG_INFO("[UserManagementService] Service initialized successfully with injected dependencies");
}

// Destructor
UserManagementService::~UserManagementService() = default;

// Public API Methods

bool UserManagementService::process_accept_new_user() {
    try {
        LOG_DEBUG("[UserManagementService] Attempting to accept new user connection");
        
        // ビジネスルール：接続数制限チェック
        if (!can_accept_new_connection()) {
            LOG_WARNING("[UserManagementService] Cannot accept new user - connection limit reached");
            return false;
        }

        // 通信層に委譲
        auto result = _communication->accept_new_connection();
        if (result.has_value()) {
            LOG_DEBUG("[UserManagementService] Communication layer accepted connection - socket: {}, ip: {}", result.socket, result.user_ip_addr);
            
            // クライアント情報をリポジトリに保存
            if (_repository->add_user(result.socket, result.ssl, result.user_addr, result.user_ip_addr)) {
                size_t total_after_add = _repository->get_connected_user_count();
                LOG_INFO("[UserManagementService] New user added to repository - socket: {}, ip: {}, total_users: {}", 
                         result.socket, result.user_ip_addr, total_after_add);
            } else {
                LOG_ERROR("[UserManagementService] Failed to add new user to repository - socket: {}, ip: {}", result.socket, result.user_ip_addr);
                _communication->close_connection(result.socket); // クリーンアップ
                return false;
            }
            return true;
        } else {
            LOG_WARNING("[UserManagementService] Failed to accept new user connection");
            return false;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("[UserManagementService] Exception in process_accept_new_user: {}", e.what());
        return false;
    }
}

void UserManagementService::process_user_disconnection(socket_t socket) {
    try {
        LOG_INFO("[UserManagementService] Processing disconnection for socket: {}", socket);
        // デバッグ：リポジトリの現在の状態を確認
        size_t total_users = _repository->get_connected_user_count();
        LOG_DEBUG("[UserManagementService] Disconnection attempt - socket: {}, total_users_in_repo: {}", socket, total_users);
        
        // ユーザー情報取得（ビジネス検証）
        auto user = _repository->get_user(socket);
        if (user == nullptr) {
            LOG_WARNING("[UserManagementService] Attempting to disconnect unknown user on socket: {} | Total users: {} | Possible causes: registration failed, already cleaned up, or race condition", 
                       socket, total_users);
            
            // 強制的に通信層の接続をクローズ（安全処理）
            try {
                _communication->close_connection(socket);
                LOG_INFO("[UserManagementService] Forced close connection for unknown socket: {}", socket);
            } catch (const std::exception& e) {
                LOG_ERROR("[UserManagementService] Failed to force close unknown socket {}: {}", socket, e.what());
            }
            return;
        }

        // ビジネスルール：安全な切断チェック
        if (!user->can_disconnect()) {
            LOG_WARNING("[UserManagementService] User {} cannot be safely disconnected at this time", user->get_id());
        }
        
        // ログ記録（切断前）
        log_connection_event(user->get_id(), "disconnecting");

        // 接続クリーンアップを通信層に委譲
        _communication->close_connection(socket);
        
        // データ削除をリポジトリ層に委譲
        _repository->remove_user(socket);
        
        LOG_INFO("[UserManagementService] User {} disconnected successfully", user->get_id());
        
    } catch (const std::exception& e) {
        LOG_ERROR("[UserManagementService] Exception in process_user_disconnection for socket {}: {}", socket, e.what());
    }
}

void UserManagementService::update_user_session(socket_t socket, const std::unique_ptr<SessionRequestMessage>& session_message) {
    try {
        auto user = _repository->get_user(socket);
        if (user == nullptr) {
            LOG_WARNING("[UserManagementService] Session update attempted on unknown socket: {}", socket);
            return;
        }

        log_connection_event(user->get_id(), "session_updated");
        
    } catch (const std::exception& e) {
        LOG_ERROR("[UserManagementService] Exception in update_user_session for socket {}: {}", socket, e.what());
    }
}

void UserManagementService::process_heartbeat(socket_t socket) {
    try {
        LOG_INFO("[UserManagementService] Processing heartbeat for socket: {}", socket);
        auto user = _repository->get_user(socket);
        if (user == nullptr) {
            LOG_WARNING("[UserManagementService] Heartbeat from unknown socket: {} | User may not be registered yet or already disconnected", socket);
            return;
        }

        // 効率的な部分更新を使用 - ハートビートカウントリセットと最終活動時間の更新
        bool update_success = _repository->update_user_heartbeat(socket);
        
        if (update_success) {
            LOG_DEBUG("[UserManagementService] Heartbeat processed for user: {} (socket: {})", user->get_id(), socket);
        } else {
            LOG_WARNING("[UserManagementService] Failed to update heartbeat for user: {} (socket: {})", user->get_id(), socket);
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("[UserManagementService] Exception in process_heartbeat for socket {}: {}", socket, e.what());
    }
}

void UserManagementService::start_heartbeat_monitoring() {
    try {
        LOG_INFO("[UserManagementService] Starting heartbeat monitoring system");
        
        // 定期的なハートビート監視をバックグラウンドで開始
        // TODO: 実際のタイマータスクまたはスレッドプールでの実装
        // 現在は手動でcleanupInactiveClients()を呼び出すことを想定
        
        LOG_INFO("[UserManagementService] Heartbeat monitoring system started successfully");
        
    } catch (const std::exception& e) {
        LOG_ERROR("[UserManagementService] Exception in start_heartbeat_monitoring: {}", e.what());
    }
}

void UserManagementService::cleanup_inactive_users() {
    try {
        LOG_DEBUG("[UserManagementService] Starting cleanup of inactive users");
        
        auto all_users = _repository->get_all_users();
        size_t cleaned_count = 0;
        auto current_time = _server_time->get_current_unix_time();
        const time_t HEARTBEAT_TIMEOUT = 30; // 30秒のタイムアウト
        
        LOG_DEBUG("[UserManagementService] Checking {} users for timeout (threshold: {}s)", all_users.size(), HEARTBEAT_TIMEOUT);
        
        for (const auto& [socket, user] : all_users) {
            if (user == nullptr) {
                LOG_WARNING("[UserManagementService] Found null user in repository during cleanup - socket: {}", socket);
                continue;
            }
            
            // ビジネスルール：ハートビートタイムアウト検証
            time_t inactive_duration = current_time - user->get_last_activity();
            LOG_TRACE("[UserManagementService] User {} (socket: {}) inactive for {} seconds", 
                     user->get_id(), socket, inactive_duration);
            
            if (inactive_duration > HEARTBEAT_TIMEOUT) {
                LOG_WARNING("[UserManagementService] User {} (socket: {}) inactive for {} seconds - scheduling for cleanup", 
                            user->get_id(), socket, inactive_duration);
                
                log_connection_event(user->get_id(), "timeout_cleanup");
                
                // 直接切断処理を行うのではなく、クリーンアップ前に再度存在確認
                auto recheck_user = _repository->get_user(socket);
                if (recheck_user != nullptr) {
                    // 安全な切断処理
                    process_user_disconnection(socket);
                    cleaned_count++;
                } else {
                    LOG_WARNING("[UserManagementService] User {} already removed from repository during cleanup check", user->get_id());
                }
            }
        }
        
        if (cleaned_count > 0) {
            LOG_INFO("[UserManagementService] Cleaned up {} inactive users", cleaned_count);
        } else {
            LOG_DEBUG("[UserManagementService] No inactive users found during cleanup");
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("[UserManagementService] Exception in cleanup_inactive_users: {}", e.what());
    }
}

// Private Helper Methods

bool UserManagementService::can_accept_new_connection() {
    const size_t MAX_CONNECTIONS = 1000; // 設定から読み取るべき値
    
    try {
        size_t current_count = _repository->get_connected_user_count();
        return current_count < MAX_CONNECTIONS;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[UserManagementService] Exception in can_accept_new_connection: {}", e.what());
        return false; // 安全側での判定
    }
}

void UserManagementService::log_connection_event(const std::string& uid, const std::string& event) {
    try {
        auto timestamp = _server_time->get_current_unix_time();
        auto connected_count = _repository->get_connected_user_count();
        
        LOG_INFO("[UserManagementService] User '{}' - Event: '{}' | Total Connected: {}", 
                     uid.empty() ? "unknown" : uid, event, connected_count);
                    
    } catch (const std::exception& e) {
        LOG_ERROR("[UserManagementService] Exception in log_connection_event: {}", e.what());
    }
}

// ==================== セッション管理  ====================

SessionResult UserManagementService::authenticate_session(const std::unique_ptr<SessionRequestMessage>& session_message, socket_t socket) {
    SessionResult result = {};
    
    try {
        LOG_INFO("[UserManagementService] Starting session authentication for socket: {}", socket);
        // セッション検証処理をDomain層のSessionHandlerに委譲
        auto validation_result = _session_handler->validate_session(session_message, socket);
        
        // SessionValidationResultをSessionResultに変換
        result.verified = validation_result.is_valid;
        result.error_message = validation_result.error_message;
        result.unreceived_messages = validation_result.unreceived_messages;

        _repository->update_user_info(socket, validation_result.uid, validation_result.name, validation_result.email);
        
        if (result.verified) {
            LOG_INFO("[UserManagementService] Session authentication successful for socket: {}, uid: {}", 
                     socket, validation_result.uid);
        } else {
            LOG_WARNING("[UserManagementService] Session authentication failed for socket: {}: {}", 
                        socket, result.error_message);
        }
        
        return result;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[UserManagementService] Exception in authenticate_session for socket {}: {}", socket, e.what());
        result.verified = false;
        result.error_message = "Internal authentication error";
        return result;
    }
}

bool UserManagementService::send_session_response(
    const std::unique_ptr<SessionRequestMessage>& session_message,
    const SessionResult& session_result,
    socket_t socket,
    const std::string& uid
) {
    try {
        // SessionResponseMessageを生成
        auto session_response = std::make_unique<SessionResponseMessage>(
            session_message->get_common_fields(),
            uid,
            session_result.verified,
            session_result.error_message,
            session_result.unreceived_messages
        );
        
        // シリアライズして送信
        json session_response_json = MessageBase::serialize_message(session_response);
        
        if (session_response_json.empty()) {
            LOG_ERROR("[UserManagementService] SessionResponse message serialization failed");
            return false;
        }

        bool send_success = _communication->send_message(socket, session_response_json);
        
        if (send_success) {
            LOG_INFO("[UserManagementService] SessionResponse sent successfully to user (socket: {}, id: {})", socket, uid);
        } else {
            LOG_ERROR("[UserManagementService] Failed to send SessionResponse to user (socket: {}, id: {})", socket, uid);
        }
        
        return send_success;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[UserManagementService] Exception in send_session_response for socket {}: {}", socket, e.what());
        return false;
    }
}

bool UserManagementService::process_session_request(const json &session_data, socket_t socket)
{
    try{
        LOG_INFO("[UserManagementService] Processing session request for socket: {}", socket);
        auto session_message = MessageBase::deserialize_message<SessionRequestMessage>(session_data);
        // セッションリクエストの処理
        if(session_message == nullptr) {
            throw std::runtime_error("SessionRequestMessage deserialization failed. Message is null.");
        }
        auto session_result = authenticate_session(session_message, socket);
        
        // セッションレスポンスの送信
        bool response_sent = send_session_response(session_message, session_result, socket, _repository->get_uid_by_socket(socket));
        
        return response_sent;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[UserManagementService] Exception in process_session_request for socket {}: {}", socket, e.what());
        return false;
    }
}
