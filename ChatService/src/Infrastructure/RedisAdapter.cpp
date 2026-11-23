#include "Infrastructure/RedisAdapter.hpp"

RedisAdapter::RedisAdapter()
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

RedisAdapter::~RedisAdapter() {
    if (_redis_helper) {
        _redis_helper->disconnect();
    }
}

bool RedisAdapter::user_session_exists(const std::string &session_key)
{
    return _redis_helper->is_key_exists(session_key);
}

bool RedisAdapter::initialize_counter_key(const std::string& key, std::uint64_t initial_value)
{
    return _redis_helper->ensure_key_initialized(key, std::to_string(initial_value));
}

bool RedisAdapter::increment_counter_key(const std::string& key, std::uint64_t& next_value)
{
    return _redis_helper->increment_key(key, next_value);
}

std::uint64_t RedisAdapter::get_counter_value(const std::string& key)
{
    const std::string value = _redis_helper->get_key_value(key);
    if (value.empty()) {
        return 0;
    }

    try {
        return static_cast<std::uint64_t>(std::stoull(value));
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to parse Redis counter value for key {}: {}", key, e.what());
        return 0;
    }
}

bool RedisAdapter::delete_key(const std::string& key)
{
    return _redis_helper->delete_key({key});
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
