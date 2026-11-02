#ifndef IREDISADAPTER_HPP
#define IREDISADAPTER_HPP
#include <string>
#include <cstdint>

// セッション保存の抽象化。テストでは実装(RedisAdapter)を差し替えて、
// 実際のRedisサーバーに接続せずに検証ロジックのテストができる。
// (RedisAdapterのコンストラクタは接続失敗時にexit(1)するため、本物を
//  テストで直接構築することはできない)
class IRedisAdapter
{
public:
    virtual ~IRedisAdapter() = default;
    virtual bool save_session(const std::string& uid, const std::string& session_token_hash,
                               std::uint64_t expire, std::uint64_t& ttl) = 0;
};

#endif // IREDISADAPTER_HPP
