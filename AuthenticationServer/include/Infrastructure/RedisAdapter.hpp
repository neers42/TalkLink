#ifndef REDIS_ADAPTER_HPP
#define REDIS_ADAPTER_HPP

#include <hiredis/hiredis.h>
#include "common.h"
#include "Database/RedisHelper.hpp"
#include "Domain/Interfaces/IRedisAdapter.hpp"

class RedisAdapter : public IRedisAdapter {

private:
    std::unique_ptr<RedisHelper> _redis_helper; // Redisヘルパーオブジェクト

public:
    RedisAdapter();
    ~RedisAdapter();

    bool save_session(const std::string& uid, const std::string& session_token_hash, std::uint64_t expire, std::uint64_t& ttl) override;
    std::optional<std::string> get_session(const std::string& session_token_hash) override;
    bool delete_key(const std::string& key) override;

};

#endif // REDIS_ADAPTER_HPP