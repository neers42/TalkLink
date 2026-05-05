#include "Infrastructure/RedisClient.hpp"

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

RedisClient::~RedisClient() {
    if (_redis_helper) {
        _redis_helper->disconnect();
    }
}

bool RedisClient::is_client_session_key(const std::string &session_key)
{
    return _redis_helper->is_key_exists(session_key);
}

