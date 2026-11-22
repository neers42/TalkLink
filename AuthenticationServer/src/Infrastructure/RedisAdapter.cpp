#include "Infrastructure/RedisAdapter.hpp"

RedisAdapter::RedisAdapter()
{
        char* env_host = nullptr;
    char* env_port = nullptr;

#ifdef _WIN32
    size_t len = 0;
    if (_dupenv_s(&env_host, &len, "REDIS_HOST") == 0 && env_host != nullptr) {
        LOG_DEBUG("REDIS_HOST: {}", env_host);
    } else {
        LOG_ERROR("REDIS_HOST environment variable is not set.");
    }
    if (_dupenv_s(&env_port, &len, "REDIS_PORT") == 0 && env_port != nullptr) {
        LOG_DEBUG("REDIS_PORT: {}", env_port);
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

RedisAdapter::~RedisAdapter() 
{
    if (this->_redis_helper) {
        this->_redis_helper->disconnect();
    }
}

bool RedisAdapter::save_session(
    const std::string& uid,
    const std::string& session_token_hash,
    std::uint64_t expire_seconds,
    std::uint64_t& ttl)
{
    const std::string session_key = "session:" + session_token_hash;

    const std::unordered_map<std::string, std::string> fields = {
        {"uid", uid}
    };

    // セッション情報を保存
    if (!_redis_helper->hmset(session_key, fields))
    {
        LOG_ERROR("Failed to store session data for UID: {} in Redis.", uid);

        return false;
    }

    // TTL設定
    if (!_redis_helper->expire(
            session_key,
            expire_seconds,
            ttl))
    {
        LOG_WARNING("Failed to set TTL for session data for UID: {} in Redis.", uid);

        // TTL設定に失敗した場合、
        // セッションが永続的に残るのを防ぐため削除
        if (_redis_helper->delete_key({session_key}))
        {
            LOG_INFO("Session data for UID: {} has been deleted from Redis.", uid);
        }
        else
        {
            LOG_ERROR(
                "Failed to delete session data for UID: {} from Redis.",
                uid);
        }

        return false;
    }

    LOG_INFO("Session data for UID: {} has been stored in Redis.", uid);

    return true;
}

std::optional<std::string> RedisAdapter::get_session(const std::string& session_token_hash)
{
    const std::string session_key = "session:" + session_token_hash;

    std::unordered_map<std::string, std::string> fields;
    if (!_redis_helper->hmget(session_key, {"uid"}, fields))
    {
        LOG_WARNING("Session not found for token hash.");
        return std::nullopt;
    }

    auto it = fields.find("uid");
    if (it == fields.end())
    {
        LOG_WARNING("Session hash for token hash has no 'uid' field.");
        return std::nullopt;
    }

    return it->second;
}

bool RedisAdapter::delete_key(const std::string& key)
{
    return _redis_helper->delete_key({key});
}
