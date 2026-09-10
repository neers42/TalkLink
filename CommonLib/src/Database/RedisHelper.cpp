#include "Database/RedisHelper.hpp"

RedisHelper::RedisHelper()
    : _host("localhost"), _port(6379), _context(nullptr, redisFree)
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
}


RedisHelper::RedisHelper(const std::string& host, int port)
    : _host(host), _port(port), _context(nullptr, redisFree)
{

}

RedisHelper::~RedisHelper()
{
    disconnect();
}

bool RedisHelper::connect()
{
    _context.reset(redisConnect(_host.c_str(), _port));
    if (_context == nullptr) {
        LOG_ERROR("Redis connection failed: Unable to allocate redis context.");
        return false;
    }else{
		if (_context->err) {
			LOG_ERROR("Redis connection error: {}", _context->errstr);
			return false;
		}
		spdlog::info("Connected to Redis server at {}:{}", _host, _port);
	}
    return _context->err == 0;
}

bool RedisHelper::disconnect()
{
    if (_context != nullptr) {
        redisFree(_context.release());
    }else {
        LOG_ERROR("Redis context is already null");
        return false; // 既に切断されている場合はfalseを返す
    }
    return true; // 切断処理が成功した場合はtrueを返す
}

bool RedisHelper::set_key_value(const std::string& key, const std::string& value)
{
    if (_context == nullptr) {
        LOG_ERROR("Redis context is null");
        return false;
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(_context.get(), "SET %s %s", key.c_str(), value.c_str()));
    if (!reply) {
        LOG_ERROR("Redis command error: {}", _context && _context->errstr ? _context->errstr : "Unknown error");
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool RedisHelper::set_key_if_not_exists(const std::string& key, const std::string& value)
{
    if (_context == nullptr) {
        LOG_ERROR("Redis context is null");
        return false;
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(_context.get(), "SET %s %s NX", key.c_str(), value.c_str()));
    if (!reply) {
        LOG_ERROR("Redis command error: {}", _context && _context->errstr ? _context->errstr : "Unknown error");
        return false;
    }

    const bool success = (reply->type == REDIS_REPLY_STATUS && reply->str && std::string(reply->str) == "OK");
    freeReplyObject(reply);
    return success;
}

bool RedisHelper::ensure_key_initialized(const std::string& key, const std::string& value)
{
    if (_context == nullptr) {
        LOG_ERROR("Redis context is null");
        return false;
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(_context.get(), "SET %s %s NX", key.c_str(), value.c_str()));
    if (!reply) {
        LOG_ERROR("Redis command error: {}", _context && _context->errstr ? _context->errstr : "Unknown error");
        return false;
    }

    const bool success = (reply->type == REDIS_REPLY_STATUS && reply->str && std::string(reply->str) == "OK") ||
                         reply->type == REDIS_REPLY_NIL;
    freeReplyObject(reply);
    return success;
}

std::string RedisHelper::get_key_value(const std::string& key)
{
    if (_context == nullptr) {
        LOG_ERROR("Redis context is null");
        return "";
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(_context.get(), "GET %s", key.c_str()));
    if (!reply) {
        LOG_ERROR("Redis command error: {}", _context && _context->errstr ? _context->errstr : "Unknown error");
        return "";
    }

    std::string value = reply->str ? reply->str : "";
    freeReplyObject(reply);
    return value;
}


bool RedisHelper::hmset(const std::string& key, const std::unordered_map<std::string, std::string>& fields)
{
    if (this->_context == nullptr) {
        LOG_ERROR("Redis context is null");
        return false;
    }

    // redisCommandの引数配列を動的に作成（argv, argvlen形式）
    // HMSET key field1 value1 field2 value2 ...
    size_t argc = 2 + fields.size() * 2;
    const char **argv = new const char *[argc];
    size_t *argvlen = new size_t[argc];

    argv[0] = "HMSET";
    argvlen[0] = 5;
    argv[1] = key.c_str();
    argvlen[1] = key.size();

    size_t i = 2;
    for (const auto &[field, value] : fields)
    {
        argv[i] = field.c_str();
        argvlen[i] = field.size();
        argv[i + 1] = value.c_str();
        argvlen[i + 1] = value.size();
        i += 2;
    }

    redisReply *reply = (redisReply *)redisCommandArgv(this->_context.get(), argc, argv, argvlen);

    delete[] argv;
    delete[] argvlen;

    if (!reply)
    {
        LOG_ERROR("Redis HMSET command failed");
        return false;
    }

    bool success = (reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "OK");
    freeReplyObject(reply);

    if (!success)
    {
        LOG_ERROR("Redis HMSET command did not return OK");
    }

    return success;
}

bool RedisHelper::hmget(const std::string& key, const std::vector<std::string>& fields_to_get, std::unordered_map<std::string, std::string>& out_fields)
{
    if (this->_context == nullptr) {
        LOG_ERROR("Redis context is null");
        return false;
    }
    if (fields_to_get.empty()) {
        LOG_ERROR("No fields to get for HMGET command");
        return false;
    }
    size_t argc = 2 + fields_to_get.size();
    const char **argv = new const char *[argc];
    size_t *argvlen = new size_t[argc];

    argv[0] = "HMGET";
    argvlen[0] = 5;
    argv[1] = key.c_str();
    argvlen[1] = key.size();

    for (size_t i = 0; i < fields_to_get.size(); ++i)
    {
        argv[2 + i] = fields_to_get[i].c_str();
        argvlen[2 + i] = fields_to_get[i].size();
    }

    redisReply *reply = (redisReply *)redisCommandArgv(this->_context.get(), argc, argv, argvlen);

    delete[] argv;
    delete[] argvlen;

    if (!reply || reply->type != REDIS_REPLY_ARRAY)
    {
        if (reply)
            freeReplyObject(reply);
        LOG_ERROR("Redis HMGET command failed or invalid reply");
        return false;
    }

    if (reply->elements != fields_to_get.size())
    {
        freeReplyObject(reply);
        LOG_ERROR("Unexpected number of elements in Redis reply");
        return false;
    }

    for (size_t i = 0; i < fields_to_get.size(); ++i)
    {
        if (reply->element[i]->type == REDIS_REPLY_STRING)
        {
            out_fields[fields_to_get[i]] = reply->element[i]->str;
        }
        else
        {
            out_fields[fields_to_get[i]] = ""; // フィールドがなければ空文字
        }
    }

    freeReplyObject(reply);
    return true;
}

bool RedisHelper::expire(const std::string &key, std::uint64_t expire, std::uint64_t& ttl)
{
    if (!this->_context) {
        LOG_ERROR("Redis context is not initialized.");
        return false;
    }
    std::uint64_t now = static_cast<std::uint64_t>(time(nullptr));

     //有効期限切れがないかチェック
    if (now >= expire) {
        LOG_ERROR("Invalid session expiration time");
        return false;
    }

    //TTL計算(秒)
    ttl = expire - now;
    redisReply *reply = (redisReply *)redisCommand(this->_context.get(), "EXPIRE %s %llu", key.c_str(), ttl);
    if (!reply) {
        LOG_ERROR("EXPIRE command failed for key: {}", key);
        return false;
    }

    bool success = (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1);
    freeReplyObject(reply);
    return success;
}

bool RedisHelper::delete_key(const std::vector<std::string>& keys)
{
   if (!this->_context) {
        LOG_ERROR("Redis context is not initialized.");
        return false;
    }

    if (keys.empty()) return true; // 削除対象なし

    // DEL key1 key2 key3 ...
    std::string cmd = "DEL";
    for (const auto& key : keys) {
        cmd += " " + key;
    }

    redisReply* reply = (redisReply*)redisCommand(this->_context.get(), cmd.c_str());
    if (!reply) {
        LOG_ERROR("DEL command failed for keys.");
        return false;
    }

    bool success = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    freeReplyObject(reply);
    return success;
}

bool RedisHelper::is_key_exists(const std::string &key)
{
    if (!this->_context) {
        LOG_ERROR("Redis context is not initialized.");
        return false;
    }

    redisReply *reply = (redisReply *)redisCommand(this->_context.get(), "EXISTS %s", key.c_str());
    if (!reply) {
        LOG_ERROR("Redis EXISTS command failed.");
        return false;
    }

    bool exists = (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1);
    freeReplyObject(reply);
    return exists;
}

bool RedisHelper::increment_key(const std::string& key, std::uint64_t& next_value)
{
    next_value = 0;

    if (!this->_context) {
        LOG_ERROR("Redis context is not initialized.");
        return false;
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(this->_context.get(), "INCR %s", key.c_str()));
    if (!reply) {
        LOG_ERROR("Redis INCR command failed for key: {}", key);
        return false;
    }

    const bool success = reply->type == REDIS_REPLY_INTEGER;
    if (success) {
        next_value = static_cast<std::uint64_t>(reply->integer);
    }
    freeReplyObject(reply);
    return success;
}
