#include "Application/Services/AuthenticationService.hpp"
#include "Domain/Interfaces/ITokenHandler.hpp"
#include "Domain/Interfaces/IRedisAdapter.hpp"
#include "Domain/SessionTokenHandler.hpp"
#include "LogHandler.hpp"
#include <stdexcept>

AuthenticationService::AuthenticationService(
    std::shared_ptr<ITokenHandler> token_handler,
    std::shared_ptr<IRedisAdapter> redis_adapter,
    std::shared_ptr<SessionTokenHandler> session_token_handler)
    : _token_handler(std::move(token_handler)),
      _redis_adapter(std::move(redis_adapter)),
      _session_token_handler(std::move(session_token_handler))
{
    if (!_token_handler || !_redis_adapter || !_session_token_handler) {
        throw std::runtime_error("Failed to initialize AuthenticationService dependencies");
    }
}

bool AuthenticationService::verify_auth_process(
    const std::string &jwt,
    std::string &uid,
    std::uint64_t &expire,
    std::uint64_t& ttl,
    std::string& session_token)
{
    try
    {
        // 検証を実施し、トークンが有効な場合の処理
        if (this->_token_handler->verify_token(jwt, uid, expire) == true)
        {
            session_token = _session_token_handler->generate_token();
            auto session_token_hash = _session_token_handler->hash_token(session_token);

            if (this->_redis_adapter->save_session(uid, session_token_hash, expire, ttl) == true)
            {
                LOG_INFO("Session saved in Redis for CID: {} with TTL: {}.", uid, ttl);
                return true;
            }
            else
            {
                LOG_ERROR("Failed to save session in Redis for CID: {}.", uid);
                return false;
            }
        }
        else
        {
            LOG_ERROR("Invalid jwt received from client.");
            return false;
        }
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Exception : {}", e.what());
        return false;
    }
}
