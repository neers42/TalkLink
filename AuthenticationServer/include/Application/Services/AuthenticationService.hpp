#ifndef AUTHENTICATIONSERVICE_HPP
#define AUTHENTICATIONSERVICE_HPP
#include <memory>
#include <string>
#include <cstdint>

class ITokenHandler;
class IRedisAdapter;
class SessionTokenHandler;

/**
 * @brief JWT検証 -> セッショントークン発行 -> Redis保存、という
 * 認証処理のオーケストレーションのみを担う(ソケット/TLS等のネットワーク
 * 処理はAuthenticationServerFacade側に残す)。
 *
 * 依存をITokenHandler/IRedisAdapterのインターフェース経由で受け取るため、
 * 実際のFirebase証明書やRedisサーバーを用意せずに単体テストできる。
 */
class AuthenticationService
{
public:
    AuthenticationService(
        std::shared_ptr<ITokenHandler> token_handler,
        std::shared_ptr<IRedisAdapter> redis_adapter,
        std::shared_ptr<SessionTokenHandler> session_token_handler
    );

    bool verify_auth_process(
        const std::string& jwt,
        std::string& uid,
        std::uint64_t& expire,
        std::uint64_t& ttl,
        std::string& session_token
    );

private:
    std::shared_ptr<ITokenHandler> _token_handler;
    std::shared_ptr<IRedisAdapter> _redis_adapter;
    std::shared_ptr<SessionTokenHandler> _session_token_handler;
};

#endif // AUTHENTICATIONSERVICE_HPP
