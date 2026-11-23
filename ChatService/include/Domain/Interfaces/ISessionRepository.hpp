#ifndef ISESSIONREPOSITORY_HPP
#define ISESSIONREPOSITORY_HPP
#include <string>
#include <optional>

// セッション情報(Redis)アクセスの抽象化。テストでは実装(RedisAdapter)を
// 差し替えて、実際のRedisサーバーに接続せずにSessionHandlerのロジックを
// テストできる。(RedisAdapterのコンストラクタは接続失敗時にexit(1)するため、
// 本物をテストで直接構築することはできない)
class ISessionRepository
{
public:
    virtual ~ISessionRepository() = default;
    virtual std::optional<std::string> get_session(const std::string& session_token_hash) = 0;
    virtual bool delete_key(const std::string& key) = 0;
};

#endif // ISESSIONREPOSITORY_HPP
