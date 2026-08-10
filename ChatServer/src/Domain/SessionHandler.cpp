#include "Domain/SessionHandler.hpp"
#include "Domain/UserRepositoryHandler.hpp"
#include "Infrastructure/RedisAdapter.hpp"
#include "Infrastructure/MySQLAdapter.hpp"
#include "ServerTime.hpp"

SessionHandler::SessionHandler(
    std::shared_ptr<UserRepositoryHandler> repository,
    std::shared_ptr<RedisAdapter> redis_adapter,
    std::shared_ptr<MySQLAdapter> mysql_adapter,
    std::shared_ptr<ServerTime> server_time)
    : _repository(std::move(repository)), 
    _redis_adapter(std::move(redis_adapter)), 
    _mysql_adapter(std::move(mysql_adapter)), 
    _server_time(std::move(server_time))
{
    if (!_repository || !_redis_adapter || !_mysql_adapter || !_server_time) {
        throw std::runtime_error("Failed to initialize SessionHandler dependencies");
    }
    
    LOG_INFO("[SessionHandler] Handler initialized successfully");
}

SessionHandler::~SessionHandler() = default;

SessionValidationResult SessionHandler::validate_session(
    const std::unique_ptr<SessionRequestMessage>& session_message, 
    socket_t socket)
{
    SessionValidationResult result = {};
    
    try {
        // メッセージフォーマット検証
        if (!validate_session_message(session_message)) {
            result.error_message = "Invalid session message format";
            log_session_event("unknown", "invalid_message_format", socket);
            return result;
        }

        // セッションタイプ別処理
        result = process_session_request_by_type(session_message, socket);
        
        return result;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[SessionHandler] Exception in validate_session for socket {}: {}", socket, e.what());
        result.error_message = "Internal session validation error";
        return result;
    }
}

std::string SessionHandler::extract_uid_from_jwt(const std::string& jwt) {
    try {
        auto decoded_token = jwt::decode(jwt);
        if(decoded_token.has_subject() == false) {
            throw std::runtime_error("JWT does not contain user_id claim");
        }
        std::string uid = decoded_token.get_subject();
        if (uid.empty()) {
            throw std::runtime_error("Missing user_id claim in JWT");
        }
        
        return uid;
    
    } catch (const jwt::error::invalid_json_exception& e) {
        LOG_ERROR("[SessionHandler] Invalid JWT format: {}", e.what());
        throw;
    }
}

bool SessionHandler::verify_session_in_redis(const std::string& uid) {
    try {
        // Redis セッションキーの存在確認
        std::string session_key = "session:" + uid;
        bool session_exists = _redis_adapter->is_user_session_key(session_key);
        
        if (session_exists) {
            LOG_DEBUG("[SessionHandler] Session found in Redis for user: {}", uid);
        } else {
            LOG_WARNING("[SessionHandler] Session not found in Redis for user: {}", uid);
        }
        
        return session_exists;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[SessionHandler] Redis session verification failed for user {}: {}", uid, e.what());
        return false;
    }
}

void SessionHandler::resolve_user_profile(
    const std::string& uid,
    const std::unique_ptr<SessionRequestMessage>& session_message,
    std::string& out_name,
    std::string& out_email)
{
    out_name = session_message ? session_message->get_user_name() : "";
    out_email = session_message ? session_message->get_user_mail_addr() : "";

    if (!out_name.empty() && !out_email.empty()) {
        return;
    }

    if (uid.empty()) {
        LOG_WARNING("[SessionHandler] Cannot resolve profile from DB because uid is empty");
        return;
    }

    try {
        std::string db_name;
        std::string db_email;
        std::string db_icon;
        auto user_info = _mysql_adapter->get_user_info(uid);
        if (out_name.empty() && !user_info.name.empty()) {
            out_name = user_info.name;
        }
        if (out_email.empty() && !user_info.mail_address.empty()) {
            out_email = user_info.mail_address;
        }

        LOG_INFO(
            "[SessionHandler] Profile resolved for uid={} (payload_name_len={}, payload_email_len={}, resolved_name_len={}, resolved_email_len={})",
            uid,
            session_message ? session_message->get_user_name().size() : 0,
            session_message ? session_message->get_user_mail_addr().size() : 0,
            out_name.size(),
            out_email.size());
    } catch (const std::exception& e) {
        LOG_ERROR("[SessionHandler] Failed to resolve profile from DB for uid {}: {}", uid, e.what());
    }
}

json SessionHandler::get_unreceived_messages(const std::string& uid, uint64_t last_received_id) {
    json unreceived_messages = json::array();
    
    try {
        // MySQL/Redisから未受信メッセージを取得する実装
        // 現在は簡易実装
        LOG_DEBUG("[SessionHandler] Getting unreceived messages for user: {}, last_received_id: {}", 
                 uid, last_received_id);
        
        // TODO: 実際のメッセージ取得ロジックを実装
        
    } catch (const std::exception& e) {
        LOG_ERROR("[SessionHandler] Failed to get unreceived messages for user {}: {}", uid, e.what());
    }
    
    return unreceived_messages;
}

bool SessionHandler::validate_session_message(const std::unique_ptr<SessionRequestMessage>& session_message) {
    if (!session_message) {
        LOG_WARNING("[SessionHandler] Session message is null");
        return false;
    }

    std::string jwt = session_message->get_jwt();
    if (jwt.empty()) {
        LOG_WARNING("[SessionHandler] JWT token is empty in session message");
        return false;
    }
    return true;
}

SessionValidationResult SessionHandler::process_session_request_by_type(
    const std::unique_ptr<SessionRequestMessage>& session_message,
    socket_t socket)
{
    auto sign_type = session_message->get_sign_type();
    
    switch (sign_type) {
        case SessionRequestMessage::SignType::SIGNIN:
            return handle_signin_request(session_message, socket);
            
        case SessionRequestMessage::SignType::SIGNUP:
            return handle_signup_request(session_message, socket);
            
        case SessionRequestMessage::SignType::SIGNOUT:
            return handle_signout_request(session_message, socket);
            
        default:
            SessionValidationResult result = {};
            result.error_message = "Unknown session request type";
            log_session_event("unknown", "unknown_request_type", socket);
            return result;
    }
}

SessionValidationResult SessionHandler::handle_signin_request(
    const std::unique_ptr<SessionRequestMessage>& session_message,
    socket_t socket)
{
    SessionValidationResult result = {};
    
    try {
        if(!session_message) {
            result.error_message = "Session message is null";
            log_session_event("unknown", "null_session_message", socket);
            return result;
        }
        // JWTからクライアントIDを抽出
        result.uid = extract_uid_from_jwt(session_message->get_jwt());
        
        // Redisでセッション存在確認
        if (!verify_session_in_redis(result.uid)) {
            result.error_message = "Session not found or expired";
            log_session_event(result.uid, "session_not_found", socket);
            return result;
        }
        
        // payload値を優先し、空ならDBから補完
        resolve_user_profile(result.uid, session_message, result.name, result.email);
        
        // 未受信メッセージ取得
        result.unreceived_messages = get_unreceived_messages(
            result.uid, 
            session_message->get_last_received_message_id()
        );
        
        result.is_valid = true;
        result.error_message = "Session validation successful";
        
        log_session_event(result.uid, "signin_successful", socket);
        LOG_INFO("[SessionHandler] SIGNIN request validated for user: {}", result.uid);
        
    } catch (const std::exception& e) {
        result.error_message = "JWT processing failed: " + std::string(e.what());
        log_session_event("unknown", "signin_jwt_error", socket);
        LOG_ERROR("[SessionHandler] SIGNIN request failed: {}", e.what());
    }
    
    return result;
}

SessionValidationResult SessionHandler::handle_signup_request(
    const std::unique_ptr<SessionRequestMessage>& session_message,
    socket_t socket)
{
    SessionValidationResult result = {};
    
    try {
        if(!session_message) {
            result.error_message = "Session message is null";
            log_session_event("unknown", "null_session_message", socket);
            return result;
        }
        // SIGNUP時もJWTから認証情報を確認
        result.uid = extract_uid_from_jwt(session_message->get_jwt());

        // Redisでセッション存在確認
        if (!verify_session_in_redis(result.uid)) {
            result.error_message = "Session not found or expired";
            log_session_event(result.uid, "session_not_found", socket);
            return result;
        }
        // ユーザの情報をDBに登録
        auto insert_db_result = _mysql_adapter->insert_user_info_data(
            result.uid, 
            session_message->get_user_name(), 
            session_message->get_user_mail_addr(),
            "",
            _server_time->get_current_time_to_seconds(),
            ""
        );
        if(!insert_db_result) {
            result.error_message = "Failed to create user in database";
            log_session_event(result.uid, "signup_db_error", socket);
            result.is_valid = false;
            result.error_message = "Failed to insert user data in database";
            return result;
        }
        result.is_valid = true;
        result.error_message = "Signup session validation successful";
        result.unreceived_messages = json::array(); // 新規ユーザーなので空
        
        log_session_event(result.uid, "signup_successful", socket);
        LOG_INFO("[SessionHandler] SIGNUP request validated for user: {}", result.uid);
        
    } catch (const std::exception& e) {
        result.error_message = "SIGNUP JWT processing failed: " + std::string(e.what());
        log_session_event("unknown", "signup_jwt_error", socket);
        LOG_ERROR("[SessionHandler] SIGNUP request failed: {}", e.what());
    }
    
    return result;
}

SessionValidationResult SessionHandler::handle_signout_request(
    const std::unique_ptr<SessionRequestMessage>& session_message,
    socket_t socket)
{
    SessionValidationResult result = {};
    
    try {
        if(!session_message) {
            result.error_message = "Session message is null";
            log_session_event("unknown", "null_session_message", socket);
            return result;
        }
        // SIGNOUT時はセッション無効化処理
        result.uid = extract_uid_from_jwt(session_message->get_jwt());
        
        // TODO: Redisからセッション削除処理を実装
        // _redis_adapter->delete_session(result.uid);
        
        result.is_valid = true;
        result.error_message = "Signout successful";
        result.unreceived_messages = json::array();
        
        log_session_event(result.uid, "signout_successful", socket);
        LOG_INFO("[SessionHandler] SIGNOUT request processed for user: {}", result.uid);
        
    } catch (const std::exception& e) {
        result.error_message = "SIGNOUT processing failed: " + std::string(e.what());
        log_session_event("unknown", "signout_error", socket);
        LOG_ERROR("[SessionHandler] SIGNOUT request failed: {}", e.what());
    }
    
    return result;
}

void SessionHandler::log_session_event(const std::string& uid, const std::string& event, socket_t socket) {
    try {
        LOG_INFO("[SessionHandler] User {} {} on socket {}", uid, event, socket);
        // TODO: Additional audit logging if needed
    } catch (const std::exception& e) {
        LOG_ERROR("[SessionHandler] Failed to log session event: {}", e.what());
    }
}