#include "Application/UseCases/UserSessionUseCase.hpp"
#include "Domain/CommunicationHandler.hpp"
#include "Domain/UserRepositoryHandler.hpp"
#include "Domain/SessionHandler.hpp"
#include "Domain/Entity/User.hpp"
#include "Domain/UserAggregateStore.hpp"
#include "Domain/Messages/SessionRequestMessage.hpp"
#include "Domain/Messages/SessionResponseMessage.hpp"
#include "Domain/Messages/UserSearchRequestMessage.hpp"
#include "Domain/Messages/UserSearchResponseMessage.hpp"

// Constructor
UserSessionUseCase::UserSessionUseCase(
    std::shared_ptr<ServerTime> server_time,
    std::shared_ptr<SessionHandler> session_handler,
    std::shared_ptr<UserRepositoryHandler> repository_handler,
    std::shared_ptr<CommunicationHandler> communication_handler,
    int max_connections,
    int heartbeat_timeout_sec,
    int heartbeat_check_interval_sec
  )
    : _server_time(std::move(server_time)),
    _session_handler(std::move(session_handler)),
    _repository(std::move(repository_handler)),
    _communication(std::move(communication_handler)),
    _max_connections(max_connections),
    _heartbeat_timeout_sec(heartbeat_timeout_sec),
    _heartbeat_check_interval_sec(heartbeat_check_interval_sec)
{
    if (!_repository || !_communication || !_session_handler || !_server_time) {
        throw std::runtime_error("Failed to initialize UserSessionUseCase dependencies");
    }

    LOG_INFO("[UserSessionUseCase] initialized successfully with injected dependencies");
}

// Destructor
UserSessionUseCase::~UserSessionUseCase() {
    stop_heartbeat_monitoring();
}

// Public API Methods

bool UserSessionUseCase::process_accept_new_user() {
    try {
        LOG_DEBUG("[UserSessionUseCase] Attempting to accept new user connection");
        
        // ビジネスルール：接続数制限チェック
        if (!can_accept_new_connection()) {
            LOG_WARNING("[UserSessionUseCase] Cannot accept new user - connection limit reached");
            return false;
        }

        // 通信層に委譲
        auto result = _communication->accept_new_connection();
        if (result.has_value()) {
            LOG_DEBUG("[UserSessionUseCase] Communication layer accepted connection - socket: {}, ip: {}", result.socket, result.user_ip_addr);
            
            // クライアント情報をリポジトリに保存
            if (_repository->add_user(result.socket, result.ssl, result.user_addr, result.user_ip_addr)) {
                size_t total_after_add = _repository->get_connected_user_count();
                LOG_INFO("[UserSessionUseCase] New user added to repository - socket: {}, ip: {}, total_users: {}", 
                         result.socket, result.user_ip_addr, total_after_add);
            } else {
                LOG_ERROR("[UserSessionUseCase] Failed to add new user to repository - socket: {}, ip: {}", result.socket, result.user_ip_addr);
                _communication->close_connection(result.socket); // クリーンアップ
                return false;
            }
            return true;
        } else {
            LOG_WARNING("[UserSessionUseCase] Failed to accept new user connection");
            return false;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("[UserSessionUseCase] Exception in process_accept_new_user: {}", e.what());
        return false;
    }
}

void UserSessionUseCase::process_user_disconnection(socket_t socket) {
    try {
        LOG_INFO("[UserSessionUseCase] Processing disconnection for socket: {}", socket);
        // デバッグ：リポジトリの現在の状態を確認
        size_t total_users = _repository->get_connected_user_count();
        LOG_DEBUG("[UserSessionUseCase] Disconnection attempt - socket: {}, total_users_in_repo: {}", socket, total_users);
        
        // ユーザー情報取得（ビジネス検証）
        auto user = _repository->get_user(socket);
        if (user == nullptr) {
            LOG_WARNING("[UserSessionUseCase] Attempting to disconnect unknown user on socket: {} | Total users: {} | Possible causes: registration failed, already cleaned up, or race condition", 
                       socket, total_users);
            
            // 強制的に通信層の接続をクローズ（安全処理）
            try {
                _communication->close_connection(socket);
                LOG_INFO("[UserSessionUseCase] Forced close connection for unknown socket: {}", socket);
            } catch (const std::exception& e) {
                LOG_ERROR("[UserSessionUseCase] Failed to force close unknown socket {}: {}", socket, e.what());
            }
            return;
        }

        // ビジネスルール：安全な切断チェック
        if (!user->can_disconnect()) {
            LOG_WARNING("[UserSessionUseCase] User {} cannot be safely disconnected at this time", user->get_id());
        }
        
        // ログ記録（切断前）
        log_connection_event(user->get_id(), "disconnecting");

        // 接続クリーンアップを通信層に委譲
        _communication->close_connection(socket);
        
        // データ削除をリポジトリ層に委譲
        _repository->remove_user(socket);
        
        LOG_INFO("[UserSessionUseCase] User {} disconnected successfully", user->get_id());
        
    } catch (const std::exception& e) {
        LOG_ERROR("[UserSessionUseCase] Exception in process_user_disconnection for socket {}: {}", socket, e.what());
    }
}

void UserSessionUseCase::update_user_session(socket_t socket, const std::unique_ptr<SessionRequestMessage>& session_message) {
    try {
        auto user = _repository->get_user(socket);
        if (user == nullptr) {
            LOG_WARNING("[UserSessionUseCase] Session update attempted on unknown socket: {}", socket);
            return;
        }

        log_connection_event(user->get_id(), "session_updated");
        
    } catch (const std::exception& e) {
        LOG_ERROR("[UserSessionUseCase] Exception in update_user_session for socket {}: {}", socket, e.what());
    }
}

void UserSessionUseCase::process_heartbeat(socket_t socket) {
    try {
        LOG_INFO("[UserSessionUseCase] Processing heartbeat for socket: {}", socket);
        auto user = _repository->get_user(socket);
        if (user == nullptr) {
            LOG_WARNING("[UserSessionUseCase] Heartbeat from unknown socket: {} | User may not be registered yet or already disconnected", socket);
            return;
        }

        // 効率的な部分更新を使用 - ハートビートカウントリセットと最終活動時間の更新
        bool update_success = _repository->update_user_heartbeat(socket);
        
        if (update_success) {
            LOG_DEBUG("[UserSessionUseCase] Heartbeat processed for user: {} (socket: {})", user->get_id(), socket);
        } else {
            LOG_WARNING("[UserSessionUseCase] Failed to update heartbeat for user: {} (socket: {})", user->get_id(), socket);
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("[UserSessionUseCase] Exception in process_heartbeat for socket {}: {}", socket, e.what());
    }
}

bool UserSessionUseCase::is_socket_authenticated(socket_t socket) {
    try {
        auto user = _repository->get_user(socket);
        if (user == nullptr) {
            return false;
        }
        return user->is_authenticated();
    } catch (const std::exception& e) {
        LOG_ERROR("[UserSessionUseCase] Exception in is_socket_authenticated for socket {}: {}", socket, e.what());
        return false;
    }
}

size_t UserSessionUseCase::get_connected_user_count() const {
    return _repository->get_connected_user_count();
}

void UserSessionUseCase::start_heartbeat_monitoring() {
    LOG_INFO("[UserSessionUseCase] Starting heartbeat monitoring loop (interval: {}s, timeout: {}s)",
             _heartbeat_check_interval_sec, _heartbeat_timeout_sec);

    while (_running) {
        std::unique_lock<std::mutex> lock(_heartbeat_mutex);
        bool stopped = _heartbeat_cv.wait_for(
            lock,
            std::chrono::seconds(_heartbeat_check_interval_sec),
            [this]() { return !_running.load(); });
        if (stopped) {
            break;
        }
        lock.unlock();

        try {
            cleanup_inactive_users();
        } catch (const std::exception& e) {
            LOG_ERROR("[UserSessionUseCase] Exception during heartbeat cleanup: {}", e.what());
        }
    }

    LOG_INFO("[UserSessionUseCase] Heartbeat monitoring loop stopped");
}

void UserSessionUseCase::stop_heartbeat_monitoring() {
    _running = false;
    _heartbeat_cv.notify_all();
}

void UserSessionUseCase::cleanup_inactive_users() {
    try {
        LOG_DEBUG("[UserSessionUseCase] Starting cleanup of inactive users");
        
        auto all_users = _repository->get_all_users();
        size_t cleaned_count = 0;
        auto current_time = _server_time->get_current_unix_time();
        const time_t HEARTBEAT_TIMEOUT = _heartbeat_timeout_sec;

        LOG_DEBUG("[UserSessionUseCase] Checking {} users for timeout (threshold: {}s)", all_users.size(), HEARTBEAT_TIMEOUT);
        
        for (const auto& [socket, user] : all_users) {
            if (user == nullptr) {
                LOG_WARNING("[UserSessionUseCase] Found null user in repository during cleanup - socket: {}", socket);
                continue;
            }
            
            // ビジネスルール：ハートビートタイムアウト検証
            time_t inactive_duration = current_time - user->get_last_activity();
            LOG_TRACE("[UserSessionUseCase] User {} (socket: {}) inactive for {} seconds", 
                     user->get_id(), socket, inactive_duration);
            
            if (inactive_duration > HEARTBEAT_TIMEOUT) {
                LOG_WARNING("[UserSessionUseCase] User {} (socket: {}) inactive for {} seconds - scheduling for cleanup", 
                            user->get_id(), socket, inactive_duration);
                
                log_connection_event(user->get_id(), "timeout_cleanup");
                
                // 直接切断処理を行うのではなく、クリーンアップ前に再度存在確認
                auto recheck_user = _repository->get_user(socket);
                if (recheck_user != nullptr) {
                    // 安全な切断処理
                    process_user_disconnection(socket);
                    cleaned_count++;
                } else {
                    LOG_WARNING("[UserSessionUseCase] User {} already removed from repository during cleanup check", user->get_id());
                }
            }
        }
        
        if (cleaned_count > 0) {
            LOG_INFO("[UserSessionUseCase] Cleaned up {} inactive users", cleaned_count);
        } else {
            LOG_DEBUG("[UserSessionUseCase] No inactive users found during cleanup");
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("[UserSessionUseCase] Exception in cleanup_inactive_users: {}", e.what());
    }
}

// Private Helper Methods

bool UserSessionUseCase::can_accept_new_connection() {
    try {
        size_t current_count = _repository->get_connected_user_count();
        return current_count < static_cast<size_t>(_max_connections);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[UserSessionUseCase] Exception in can_accept_new_connection: {}", e.what());
        return false; // 安全側での判定
    }
}

void UserSessionUseCase::log_connection_event(const std::string& uid, const std::string& event) {
    try {
        auto timestamp = _server_time->get_current_unix_time();
        auto connected_count = _repository->get_connected_user_count();
        
        LOG_INFO("[UserSessionUseCase] User '{}' - Event: '{}' | Total Connected: {}", 
                     uid.empty() ? "unknown" : uid, event, connected_count);
                    
    } catch (const std::exception& e) {
        LOG_ERROR("[UserSessionUseCase] Exception in log_connection_event: {}", e.what());
    }
}

// ==================== セッション管理  ====================

SessionResult UserSessionUseCase::authenticate_session(const std::unique_ptr<SessionRequestMessage>& session_message, socket_t socket) {
    SessionResult result = {};
    
    try {
        LOG_INFO("[UserSessionUseCase] Starting session authentication for socket: {}", socket);
        // セッション検証処理をDomain層のSessionHandlerに委譲
        auto validation_result = _session_handler->validate_session(session_message, socket);
        
        // SessionValidationResultをSessionResultに変換
        result.verified = validation_result.is_valid;
        result.error_message = validation_result.error_message;
        result.unreceived_messages = validation_result.unreceived_messages;

        if (result.verified) {
            // 検証に失敗した接続にはuidを紐付けない(未登録ユーザ宛ての配信先にさせないため)
            _repository->update_user_info(socket, validation_result.uid, validation_result.name, validation_result.email);
            // SIGNOUTは認証解除、SIGNIN/SIGNUPは認証成立として反映する
            const bool new_auth_status = session_message->get_sign_type() != SessionRequestMessage::SignType::SIGNOUT;
            _repository->update_user_auth_status(socket, new_auth_status);
            LOG_INFO("[UserSessionUseCase] Session authentication successful for socket: {}, uid: {}",
                     socket, validation_result.uid);
        } else {
            LOG_WARNING("[UserSessionUseCase] Session authentication failed for socket: {}: {}", 
                        socket, result.error_message);
        }
        
        return result;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[UserSessionUseCase] Exception in authenticate_session for socket {}: {}", socket, e.what());
        result.verified = false;
        result.error_message = "Internal authentication error";
        return result;
    }
}

bool UserSessionUseCase::send_session_response(
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
            LOG_ERROR("[UserSessionUseCase] SessionResponse message serialization failed");
            return false;
        }

        bool send_success = _communication->send_message(socket, session_response_json);
        
        if (send_success) {
            LOG_INFO("[UserSessionUseCase] SessionResponse sent successfully to user (socket: {}, id: {})", socket, uid);
        } else {
            LOG_ERROR("[UserSessionUseCase] Failed to send SessionResponse to user (socket: {}, id: {})", socket, uid);
        }
        
        return send_success;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[UserSessionUseCase] Exception in send_session_response for socket {}: {}", socket, e.what());
        return false;
    }
}

bool UserSessionUseCase::send_user_search_response(
    const std::unique_ptr<UserSearchRequestMessage>& user_search_request,
    const std::vector<UserInfo> &user_infos,
    const std::string &error_message,
    socket_t socket)
{
    try {
        if (user_search_request == nullptr) {
            LOG_ERROR("[UserSessionUseCase] UserSearchRequestMessage is null");
            return false;
        }

        json search_results = json::array();
        for (const auto& info : user_infos) {
            json entry;
            entry["id"] = info.uid;
            entry["name"] = info.name;
            entry["icon_url"] = info.icon_url;
            search_results.push_back(entry);
        }

        // UserSearchResponseMessageを生成
        auto user_search_response = std::make_unique<UserSearchResponseMessage>(
            user_search_request->get_common_fields(),
            search_results,
            error_message
        );
        
        // シリアライズして送信
        json user_search_response_json = MessageBase::serialize_message(user_search_response);
        
        if (user_search_response_json.empty()) {
            LOG_ERROR("[UserSessionUseCase] UserSearchResponse message serialization failed");
            return false;
        }

        bool send_success = _communication->send_message(socket, user_search_response_json);
        
        if (send_success) {
            LOG_INFO("[UserSessionUseCase] UserSearchResponse sent successfully to socket: {}", socket);
            return true;
        } else {
            LOG_ERROR("[UserSessionUseCase] Failed to send UserSearchResponse to socket: {}", socket);
            return false;
        }
    }catch (const std::exception& e) {
        LOG_ERROR("[UserSessionUseCase] Exception in send_user_search_response for socket {}: {}", socket, e.what());
        return false;
    }
}

bool UserSessionUseCase::process_session_request(const json &session_data, socket_t socket)
{
    try{
        LOG_INFO("[UserSessionUseCase] Processing session request for socket: {}", socket);
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
        LOG_ERROR("[UserSessionUseCase] Exception in process_session_request for socket {}: {}", socket, e.what());
        return false;
    }
}

bool UserSessionUseCase::process_user_search(const json& query_data, socket_t socket)
{
    try
    {
        LOG_INFO("[UserSessionUseCase] Processing user search for socket: {}", socket);
        
        //user search requestメッセージの作成
        auto user_search_request = MessageBase::deserialize_message<UserSearchRequestMessage>(query_data);
        if(user_search_request == nullptr) {
            throw std::runtime_error("UserSearchRequestMessage deserialization failed. Message is null.");
        }

        //DBからユーザーの情報を検索
        auto user_infos = _repository->search_users(user_search_request->get_query());

        // レスポンスの送信
        bool response_sent = send_user_search_response(user_search_request, user_infos.user_infos, user_infos.error_message, socket);
        return response_sent;
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("[UserSessionUseCase] Exception in process_user_search for socket {}: {}", socket, e.what());
    }
    return false;
}
