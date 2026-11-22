#ifndef ITOKENVERIFIER_HPP
#define ITOKENVERIFIER_HPP
#include <string>
#include <cstdint>

// JWT検証の抽象化。テストでは実装(FirebaseTokenAdapter)を差し替えて、
// 実際のFirebase証明書ファイルを用意せずに検証ロジックのテストができる。
class ITokenVerifier
{
public:
    virtual ~ITokenVerifier() = default;
    virtual bool verify_token(const std::string& token, std::string& uid, std::uint64_t& expire) = 0;
};

#endif // ITOKENVERIFIER_HPP
