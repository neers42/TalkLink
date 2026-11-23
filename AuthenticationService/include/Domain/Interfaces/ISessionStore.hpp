#ifndef ISESSIONSTORE_HPP
#define ISESSIONSTORE_HPP
#include <string>
#include <cstdint>
#include <optional>

// セッション保存の抽象化。テストでは実装(RedisAdapter)を差し替えて、
// 実際のRedisサーバーに接続せずに検証ロジックのテストができる。
// (RedisAdapterのコンストラクタは接続失敗時にexit(1)するため、本物を
//  テストで直接構築することはできない)
class ISessionStore
{
public:
    virtual ~ISessionStore() = default;
    virtual bool save_session(const std::string& uid, const std::string& session_token_hash,
                               std::uint64_t expire, std::uint64_t& ttl) = 0;

    // SessionGrpcAdapter(ChatServiceへのセッション検証API)用の読み取り/削除。
    // ChatServiceのISessionRepositoryと同じ意味論: get_sessionはハッシュ値のみを
    // 受け取り内部で"session:"プレフィックスを付与する。delete_keyは完成済みの
    // キー全体を受け取る。
    virtual std::optional<std::string> get_session(const std::string& session_token_hash) = 0;
    virtual bool delete_key(const std::string& key) = 0;
};

#endif // ISESSIONSTORE_HPP
