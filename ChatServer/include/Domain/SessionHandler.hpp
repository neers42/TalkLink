#ifndef SESSION_HANDLER_HPP
#define SESSION_HANDLER_HPP
#include <string>
#include <memory>
#include "Domain/Entity/User.hpp"
#include "Domain/Entity/Messages/SessionRequestMessage.hpp"
#include "Domain/Entity/Messages/SessionResponseMessage.hpp"
#include "common.hpp"
#include "LogHandler.hpp"

class UserRepositoryHandler;
class RedisAdapter;
class MySQLAdapter;
class ServerTime;

// セッション検証結果
struct SessionValidationResult {
    bool is_valid = false;
    std::string error_message;
    std::string uid;
    std::string email;
    std::string name;
    json unreceived_messages;

    SessionValidationResult(){
        is_valid = false;
        error_message = "";
        uid = "";
        email = "";
        name = "";
        unreceived_messages = json::array();
    };
};

/**
 * @brief Domain layer session management handler
 * 
 * Handles session validation logic including Redis session verification,
 * JWT token processing, and session state management.
 */
class SessionHandler {
public:
    /**
     * @brief Constructor
     * 
     * @param repository User repository for data access
     * @param redis_client Redis user for session storage access
     * @param mysql_client MySQL user for message history access
     * @param server_time Server time utility for timestamp management
     */
    SessionHandler(
        std::shared_ptr<UserRepositoryHandler> repository,
        std::shared_ptr<RedisAdapter> redis_client,
        std::shared_ptr<MySQLAdapter> mysql_client,
        std::shared_ptr<ServerTime> server_time
    );

    /**
     * @brief Destructor
     */
    ~SessionHandler();

    /**
     * @brief Validate session request and verify with Redis
     * 
     * Processes SessionRequestMessage, validates JWT token,
     * checks Redis session existence, and returns validation result.
     * 
     * @param session_message Session request message
     * @param socket User socket identifier
     * @return SessionValidationResult containing validation result and user info
     */
    SessionValidationResult validate_session(
        const std::unique_ptr<SessionRequestMessage>& session_message, 
        socket_t socket
    );

    /**
     * @brief Extract user ID from JWT token
     * 
     * @param jwt JWT token string
     * @return User ID extracted from token
     * @throws jwt::error exceptions on parsing failure
     */
    std::string extract_uid_from_jwt(const std::string& jwt);

    /**
     * @brief Verify session exists in Redis
     * 
     * @param uid User identifier to check
     * @return true if session exists and is valid, false otherwise
     */
    bool verify_session_in_redis(const std::string& uid);

    /**
     * @brief Get unreceived messages for user
     * 
     * @param uid User identifier
     * @param last_received_id Last received message ID
     * @return JSON array of unreceived messages
     */
    json get_unreceived_messages(const std::string& uid, uint64_t last_received_id);

    /**
     * @brief Validate session request message format
     * 
     * @param session_message Session request message to validate
     * @return true if message format is valid, false otherwise
     */
    bool validate_session_message(const std::unique_ptr<SessionRequestMessage>& session_message);

    /**
     * @brief Process different session request types
     * 
     * @param session_message Session request message
     * @param socket User socket
     * @return SessionValidationResult for the specific request type
     */
    SessionValidationResult process_session_request_by_type(
        const std::unique_ptr<SessionRequestMessage>& session_message,
        socket_t socket
    );

private:
    std::shared_ptr<UserRepositoryHandler> _repository;
    std::shared_ptr<RedisAdapter> _redis_adapter;
    std::shared_ptr<MySQLAdapter> _mysql_adapter;
    std::shared_ptr<ServerTime> _server_time;

    // JWT configuration
    static constexpr const char* JWT_SECRET_KEY = "your-secret-key"; // TODO: Move to config
    static constexpr const char* JWT_ISSUER = "TalkLink";

    /**
     * @brief Log session validation events
     * 
     * @param uid User identifier
     * @param event Event description
     * @param socket Socket identifier
     */
    void log_session_event(const std::string& uid, const std::string& event, socket_t socket);

    /**
     * @brief Handle SIGNIN session request
     * 
     * @param session_message Session request message
     * @param socket User socket
     * @return SessionValidationResult for signin
     */
    SessionValidationResult handle_signin_request(
        const std::unique_ptr<SessionRequestMessage>& session_message,
        socket_t socket
    );

    /**
     * @brief Handle SIGNUP session request
     * 
     * @param session_message Session request message
     * @param socket User socket
     * @return SessionValidationResult for signup
     */
    SessionValidationResult handle_signup_request(
        const std::unique_ptr<SessionRequestMessage>& session_message,
        socket_t socket
    );

    /**
     * @brief Handle SIGNOUT session request
     * 
     * @param session_message Session request message
     * @param socket User socket
     * @return SessionValidationResult for signout
     */
    SessionValidationResult handle_signout_request(
        const std::unique_ptr<SessionRequestMessage>& session_message,
        socket_t socket
    );

    /**
     * @brief Resolve user profile fields from session payload and DB fallback
     *
     * If payload values are empty, this method attempts to fill missing fields
     * from MySQL using uid.
     */
    void resolve_user_profile(
        const std::string& uid,
        const std::unique_ptr<SessionRequestMessage>& session_message,
        std::string& out_name,
        std::string& out_email
    );
};
#endif // SESSION_HANDLER_HPP