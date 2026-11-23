#ifndef SESSIONTOKENHANDLER_HPP
#define SESSIONTOKENHANDLER_HPP
#include <string>

class SessionTokenHandler
{
public:
    SessionTokenHandler() = default;

    std::string generate_token() const;
    std::string hash_token(const std::string& token) const;
};

#endif //SESSIONTOKENHANDLER_HPP