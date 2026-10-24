#include "Domain/SessionHandler.hpp"
#include "Domain/UserRepositoryHandler.hpp"
#include "Infrastructure/RedisAdapter.hpp"
#include "Domain/Interfaces/IUserRepository.hpp"
#include "Domain/Interfaces/IMessageRepository.hpp"
#include "ServerTime.hpp"

SessionHandler::SessionHandler(
    std::shared_ptr<UserRepositoryHandler> repository,
    std::shared_ptr<RedisAdapter> redis_adapter,
    std::shared_ptr<IUserRepository> user_repository,
    std::shared_ptr<IMessageRepository> message_repository,
    std::shared_ptr<ServerTime> server_time)
    : _repository(std::move(repository)), 
    _redis_adapter(std::move(redis_adapter)), 
    _user_repository(std::move(user_repository)),
    _message_repository(std::move(message_repository)), 
    _server_time(std::move(server_time))
{
    if (!_repository || !_redis_adapter || !_user_repository || !_message_repository || !_server_time) {
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

std::string SessionHandler::get_uid_from_session_token(const std::string& session_token) {
    try {
        //sessionからuidを抽出
        const auto session_token_hash =this->hash_token(session_token);
        const auto uid = _redis_adapter->get_session(session_token_hash);

        if (!uid)
        {
            // セッション無効
            throw std::runtime_error("Invalid Session.");
        }

        return uid.value();
    
    } catch (const std::runtime_error& e) {
        LOG_ERROR("[SessionHandler] Invalid Session format: {}", e.what());
        throw;
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
        auto user_info = _user_repository->get_user_info(uid);
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

std::string SessionHandler::hash_token(const std::string &token) const
{
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_length = 0;

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();

    if (ctx == nullptr)
    {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(
            ctx,
            token.data(),
            token.size()) != 1 ||
        EVP_DigestFinal_ex(
            ctx,
            hash,
            &hash_length) != 1)
    {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("Failed to hash session token");
    }

    EVP_MD_CTX_free(ctx);

    std::ostringstream oss;

    for (unsigned int i = 0; i < hash_length; ++i)
    {
        oss << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<int>(hash[i]);
    }

    return oss.str();
}

json SessionHandler::get_unreceived_messages(const std::string& uid, uint64_t last_received_id) {
    json unreceived_messages = json::array();
    
    try {
        LOG_DEBUG("[SessionHandler] Getting unreceived messages for user: {}, last_received_id: {}", 
                 uid, last_received_id);

        if (uid.empty()) {
            LOG_WARNING("[SessionHandler] uid is empty while getting unreceived messages");
            return unreceived_messages;
        }
        if (!_message_repository) {
            LOG_ERROR("[SessionHandler] MessageRepository is not initialized");
            return unreceived_messages;
        }

        json db_result = _message_repository->get_unreceived_messages(uid, last_received_id);

        if (db_result.is_object() && db_result.contains("unreceived_messages") && db_result["unreceived_messages"].is_array()) {
            LOG_INFO("[SessionHandler] Retrieved {} unreceived messages for user {}", db_result["unreceived_messages"].size(), uid);
            return db_result;
        }

        if (db_result.is_array()) {
            LOG_INFO("[SessionHandler] Retrieved {} unreceived messages(array) for user {}", db_result.size(), uid);
            return db_result;
        }

        LOG_WARNING("[SessionHandler] Unexpected unreceived messages format for user {}. Returning empty array.", uid);
        
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

    std::string jwt = session_message->get_session_token();
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
        // session_tokenからUIDを抽出
        result.uid = get_uid_from_session_token(session_message->get_session_token());
        
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
        // SIGNUP時もセッショントークンから認証情報を確認
        // （get_uid_from_session_token が Redis 上のセッションを検証し、
        //   無効な場合は例外を投げるため、ここで別途確認する必要はない）
        result.uid = get_uid_from_session_token(session_message->get_session_token());

        // ユーザの情報をDBに登録
        auto insert_db_result = _user_repository->insert_user_info_data(
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
        result.uid = get_uid_from_session_token(session_message->get_session_token());

        // Redisからセッションを削除し、以後このトークンでの認証を無効化する
        // (キーはget_session/save_sessionと同じ"session:"+トークンハッシュの形式)
        const std::string session_key = "session:" + this->hash_token(session_message->get_session_token());
        if (!_redis_adapter->delete_key(session_key)) {
            LOG_WARNING("[SessionHandler] Failed to delete session from Redis for user: {}", result.uid);
        }

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