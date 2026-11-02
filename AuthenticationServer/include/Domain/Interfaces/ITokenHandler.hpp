#ifndef ITOKENHANDLER_HPP
#define ITOKENHANDLER_HPP
#include <string>
#include <cstdint>

// JWT検証の抽象化。テストでは実装(TokenHandler)を差し替えて、
// 実際のFirebase証明書ファイルを用意せずに検証ロジックのテストができる。
class ITokenHandler
{
public:
    virtual ~ITokenHandler() = default;
    virtual bool verify_token(const std::string& token, std::string& uid, std::uint64_t& expire) = 0;
};

#endif // ITOKENHANDLER_HPP
