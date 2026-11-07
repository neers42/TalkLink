#ifndef REDISADAPTER_HPP
#define REDISADAPTER_HPP
#include "Utils/common.hpp"
#include "Database/RedisHelper.hpp"
#include "Domain/Interfaces/ISessionRepository.hpp"
#include "Domain/Interfaces/IRoomSequenceRepository.hpp"
class RedisAdapter : public ISessionRepository, public IRoomSequenceRepository
{
private:
    std::unique_ptr<RedisHelper> _redis_helper; // Redisヘルパーオブジェクト


public:
    RedisAdapter();
    ~RedisAdapter();
    bool user_session_exists(const std::string &session_key); // ユーザーのセッションキーが存在するか確認(どのインターフェースにも属さない)
    bool initialize_counter_key(const std::string& key, std::uint64_t initial_value = 0) override;
    bool increment_counter_key(const std::string& key, std::uint64_t& next_value) override;
    std::uint64_t get_counter_value(const std::string& key) override;
    bool delete_key(const std::string& key) override;
    std::optional<std::string> get_session(const std::string& session_token_hash) override;
};

#endif // !REDISADAPTER_HPP
