#include "Domain/SessionHandler.hpp"
#include "Domain/ClientRepositoryHandler.hpp"
#include "Infrastructure/RedisClient.hpp"
#include "Infrastructure/MySQLClient.hpp"
#include "ServerTime.hpp"

SessionHandler::SessionHandler(
    std::shared_ptr<ClientRepositoryHandler> repository,
    std::shared_ptr<RedisClient> redis_client,
    std::shared_ptr<MySQLClient> mysql_client,
    std::shared_ptr<ServerTime> server_time)
    : _repository(repository), _redis_client(redis_client), _mysql_client(mysql_client), _server_time(server_time)
{
    if (!_repository || !_redis_client || !_mysql_client || !_server_time) {
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

std::string SessionHandler::extract_client_id_from_jwt(const std::string& jwt) {
    try {
        auto decoded_token = jwt::decode(jwt);
        if(decoded_token.has_subject() == false) {
            throw std::runtime_error("JWT does not contain user_id claim");
        }
        std::string client_id = decoded_token.get_subject();
        if (client_id.empty()) {
            throw std::runtime_error("Missing user_id claim in JWT");
        }
        
        return client_id;
    
    } catch (const jwt::error::invalid_json_exception& e) {
        LOG_ERROR("[SessionHandler] Invalid JWT format: {}", e.what());
        throw;
    }
}

bool SessionHandler::verify_session_in_redis(const std::string& client_id) {
    try {
        // Redis セッションキーの存在確認
        std::string session_key = "session:" + client_id;
        bool session_exists = _redis_client->is_client_session_key(session_key);
        
        if (session_exists) {
            LOG_DEBUG("[SessionHandler] Session found in Redis for client: {}", client_id);
        } else {
            LOG_WARNING("[SessionHandler] Session not found in Redis for client: {}", client_id);
        }
        
        return session_exists;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[SessionHandler] Redis session verification failed for client {}: {}", client_id, e.what());
        return false;
    }
}

json SessionHandler::get_unreceived_messages(const std::string& client_id, uint64_t last_received_id) {
    json unreceived_messages = json::array();
    
    try {
        // MySQL/Redisから未受信メッセージを取得する実装
        // 現在は簡易実装
        LOG_DEBUG("[SessionHandler] Getting unreceived messages for client: {}, last_received_id: {}", 
                 client_id, last_received_id);
        
        // TODO: 実際のメッセージ取得ロジックを実装
        // unreceived_messages = _mysql_client->get_messages_after(client_id, last_received_id);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[SessionHandler] Failed to get unreceived messages for client {}: {}", client_id, e.what());
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
        result.client_id = extract_client_id_from_jwt(session_message->get_jwt());
        
        // Redisでセッション存在確認
        if (!verify_session_in_redis(result.client_id)) {
            result.error_message = "Session not found or expired";
            log_session_event(result.client_id, "session_not_found", socket);
            return result;
        }
        
        //emailとnameをセット
        result.email = session_message->get_client_mail_addr();
        result.name = session_message->get_client_name();
        
        // 未受信メッセージ取得
        result.unreceived_messages = get_unreceived_messages(
            result.client_id, 
            session_message->get_last_received_message_id()
        );
        
        result.is_valid = true;
        result.error_message = "Session validation successful";
        
        log_session_event(result.client_id, "signin_successful", socket);
        LOG_INFO("[SessionHandler] SIGNIN request validated for client: {}", result.client_id);
        
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
        result.client_id = extract_client_id_from_jwt(session_message->get_jwt());
        
        // Redisでセッション存在確認
        if (!verify_session_in_redis(result.client_id)) {
            result.error_message = "Authentication required for signup";
            log_session_event(result.client_id, "signup_session_not_found", socket);
            return result;
        }
        
        //emailとnameをセット
        result.email = session_message->get_client_mail_addr();
        result.name = session_message->get_client_name();
        
        result.is_valid = true;
        result.error_message = "Signup session validation successful";
        result.unreceived_messages = json::array(); // 新規ユーザーなので空
        
        log_session_event(result.client_id, "signup_successful", socket);
        LOG_INFO("[SessionHandler] SIGNUP request validated for client: {}", result.client_id);
        
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
        result.client_id = extract_client_id_from_jwt(session_message->get_jwt());
        
        // TODO: Redisからセッション削除処理を実装
        // _redis_client->delete_session(result.client_id);
        
        result.is_valid = true;
        result.error_message = "Signout successful";
        result.unreceived_messages = json::array();
        
        log_session_event(result.client_id, "signout_successful", socket);
        LOG_INFO("[SessionHandler] SIGNOUT request processed for client: {}", result.client_id);
        
    } catch (const std::exception& e) {
        result.error_message = "SIGNOUT processing failed: " + std::string(e.what());
        log_session_event("unknown", "signout_error", socket);
        LOG_ERROR("[SessionHandler] SIGNOUT request failed: {}", e.what());
    }
    
    return result;
}

void SessionHandler::log_session_event(const std::string& client_id, const std::string& event, socket_t socket) {
    try {
        LOG_INFO("[SessionHandler] Client {} {} on socket {}", client_id, event, socket);
        // TODO: Additional audit logging if needed
    } catch (const std::exception& e) {
        LOG_ERROR("[SessionHandler] Failed to log session event: {}", e.what());
    }
}