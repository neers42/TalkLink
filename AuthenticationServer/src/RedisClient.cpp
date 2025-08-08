#include "RedisClient.hpp"

RedisClient::RedisClient()
{
        char* env_host = nullptr;
    char* env_port = nullptr;

#ifdef _WIN32
    size_t len = 0;
    if (_dupenv_s(&env_host, &len, "REDIS_HOST") == 0 && env_host != nullptr) {
        spdlog::debug("REDIS_HOST: {}", env_host);
    } else {
        LOG_ERROR("REDIS_HOST environment variable is not set.");
    }
    if (_dupenv_s(&env_port, &len, "REDIS_PORT") == 0 && env_port != nullptr) {
        spdlog::debug("REDIS_PORT: {}", env_port);
    } else {
        LOG_ERROR("REDIS_PORT environment variable is not set.");
    }
#else
    env_host = std::getenv("REDIS_HOST");
    env_port = std::getenv("REDIS_PORT");
#endif // _WIN32
    std::string redis_host = env_host ? env_host : "localhost";  // デフォルトは任意
    int redis_port = env_port ? std::atoi(env_port) : 6379; // デフォルトポート
    _redis_helper = std::make_unique<RedisHelper>(redis_host, redis_port);
    if (!_redis_helper->connect()) {
        exit(1);
    }
}

RedisClient::~RedisClient() 
{
    if (this->_redis_helper) {
        this->_redis_helper->disconnect();
    }
}

bool RedisClient::save_session(const std::string &cid, const std::string &jwt_token, std::uint64_t expire, std::uint64_t& ttl)
{
   
    std::string session_key = "session:" + cid;
    std::unordered_map<std::string, std::string> fields = {
        {"expire", std::to_string(expire)},
        {"jwt_token", jwt_token}};
    if (_redis_helper->hmset(session_key, fields) == true)
    {
        //TTL設定
        if (this->_redis_helper->expire(session_key, expire, ttl) == true) {
            spdlog::info("Session data for CID: {} has been stored in Redis.", cid);
        }else{
            spdlog::warn("Failed to set TTL for session data for CID: {} in Redis.", cid);
            if (_redis_helper->delete_key({session_key}) == true) {
                spdlog::info("Session data for CID: {} has been deleted from Redis.", cid);
            }else{
                //keyの削除に失敗しセッションが残り続けてしまう可能性がある
                spdlog::critical("Failed to delete session data for CID: {} from Redis.", cid);
            }
            return false;
        }
    }
    else
    {
        LOG_ERROR("Failed to store session data for CID: {} in Redis.", cid);
        return false;
    }
    return true;
}

bool RedisClient::get_session(const std::string &cid, std::string &jwt_token, std::uint64_t &expire)
{
    return false;
}

bool RedisClient::delete_session(const std::string &cid)
{
    return false;
}
