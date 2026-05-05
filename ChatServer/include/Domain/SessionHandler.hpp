#pragma once

#include <string>
#include <memory>
#include <jwt-cpp/jwt.h>
#include "Domain/Entity/Client.hpp"
#include "Domain/Entity/Messages/SessionRequestMessage.hpp"
#include "Domain/Entity/Messages/SessionResponseMessage.hpp"
#include "common.hpp"
#include "LogHandler.hpp"

class ClientRepositoryHandler;
class RedisClient;
class MySQLClient;
class ServerTime;

// セッション検証結果
struct SessionValidationResult {
    bool is_valid = false;
    std::string error_message;
    std::string client_id;
    std::string email;
    std::string name;
    json unreceived_messages;

    SessionValidationResult(){
        is_valid = false;
        error_message = "";
        client_id = "";
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
     * @param repository Client repository for data access
     * @param redis_client Redis client for session storage access
     * @param mysql_client MySQL client for message history access
     * @param server_time Server time utility for timestamp management
     */
    SessionHandler(
        std::shared_ptr<ClientRepositoryHandler> repository,
        std::shared_ptr<RedisClient> redis_client,
        std::shared_ptr<MySQLClient> mysql_client,
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
     * @param socket Client socket identifier
     * @return SessionValidationResult containing validation result and client info
     */
    SessionValidationResult validate_session(
        const std::unique_ptr<SessionRequestMessage>& session_message, 
        socket_t socket
    );

    /**
     * @brief Extract client ID from JWT token
     * 
     * @param jwt JWT token string
     * @return Client ID extracted from token
     * @throws jwt::error exceptions on parsing failure
     */
    std::string extract_client_id_from_jwt(const std::string& jwt);

    /**
     * @brief Verify session exists in Redis
     * 
     * @param client_id Client identifier to check
     * @return true if session exists and is valid, false otherwise
     */
    bool verify_session_in_redis(const std::string& client_id);

    /**
     * @brief Get unreceived messages for client
     * 
     * @param client_id Client identifier
     * @param last_received_id Last received message ID
     * @return JSON array of unreceived messages
     */
    json get_unreceived_messages(const std::string& client_id, uint64_t last_received_id);

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
     * @param socket Client socket
     * @return SessionValidationResult for the specific request type
     */
    SessionValidationResult process_session_request_by_type(
        const std::unique_ptr<SessionRequestMessage>& session_message,
        socket_t socket
    );

private:
    std::shared_ptr<ClientRepositoryHandler> _repository;
    std::shared_ptr<RedisClient> _redis_client;
    std::shared_ptr<MySQLClient> _mysql_client;
    std::shared_ptr<ServerTime> _server_time;

    // JWT configuration
    static constexpr const char* JWT_SECRET_KEY = "your-secret-key"; // TODO: Move to config
    static constexpr const char* JWT_ISSUER = "TalkLink";

    /**
     * @brief Log session validation events
     * 
     * @param client_id Client identifier
     * @param event Event description
     * @param socket Socket identifier
     */
    void log_session_event(const std::string& client_id, const std::string& event, socket_t socket);

    /**
     * @brief Handle SIGNIN session request
     * 
     * @param session_message Session request message
     * @param socket Client socket
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
     * @param socket Client socket
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
     * @param socket Client socket
     * @return SessionValidationResult for signout
     */
    SessionValidationResult handle_signout_request(
        const std::unique_ptr<SessionRequestMessage>& session_message,
        socket_t socket
    );
};