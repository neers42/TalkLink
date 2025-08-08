#ifndef REDIS_CLIENT_HPP
#define REDIS_CLIENT_HPP

#include <hiredis/hiredis.h>
#include "common.h"
#include "RedisHelper.hpp"

class RedisClient {

private:
    std::unique_ptr<RedisHelper> _redis_helper; // Redisヘルパーオブジェクト

public:
    RedisClient();
    ~RedisClient();

    bool save_session(const std::string& cid, const std::string& jwt_token, std::uint64_t expire, std::uint64_t& ttl);
    bool get_session(const std::string& cid, std::string& jwt_token, std::uint64_t& expire);
    bool delete_session(const std::string& cid);

};

#endif // REDIS_CLIENT_HPP