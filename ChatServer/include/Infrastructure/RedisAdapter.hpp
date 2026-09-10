#ifndef REDISADAPTER_HPP
#define REDISADAPTER_HPP
#include "Utils/common.hpp"
#include "Database/RedisHelper.hpp"
class RedisAdapter
{
private:
    std::unique_ptr<RedisHelper> _redis_helper; // Redisヘルパーオブジェクト


public:
    RedisAdapter();
    ~RedisAdapter();
    bool user_session_exists(const std::string &session_key); // ユーザーのセッションキーが存在するか確認
    bool initialize_counter_key(const std::string& key, std::uint64_t initial_value = 0);
    bool increment_counter_key(const std::string& key, std::uint64_t& next_value);
    std::uint64_t get_counter_value(const std::string& key);
    bool delete_key(const std::string& key);

};

#endif // !REDISADAPTER_HPP
