#ifndef REDIS_ADAPTER_HPP
#define REDIS_ADAPTER_HPP

#include <hiredis/hiredis.h>
#include "common.h"
#include "Database/RedisHelper.hpp"

class RedisAdapter {

private:
    std::unique_ptr<RedisHelper> _redis_helper; // Redisヘルパーオブジェクト

public:
    RedisAdapter();
    ~RedisAdapter();

    bool save_session(const std::string& uid, const std::string& jwt_token, std::uint64_t expire, std::uint64_t& ttl);
    bool get_session(const std::string& uid, std::string& jwt_token, std::uint64_t& expire);
    bool delete_session(const std::string& uid);

};

#endif // REDIS_ADAPTER_HPP