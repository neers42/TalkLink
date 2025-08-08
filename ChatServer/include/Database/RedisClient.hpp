#ifndef REDISCLIENT_HPP
#define REDISCLIENT_HPP
#include "Utils/common.hpp"
#include "RedisHelper.hpp"
class RedisClient
{
private:
    std::unique_ptr<RedisHelper> _redis_helper; // Redisヘルパーオブジェクト


public:
    RedisClient();
    ~RedisClient();
    bool is_client_session_key(const std::string &session_key); // クライアントのセッションキーが存在するか確認

};

#endif // !REDISCLIENT_HPP
