#ifndef TOKEN_HANDLER_HPP
#define TOKEN_HANDLER_HPP
#include "common.h"
#define CERTS_DIR "/app/AuthenticationServer/certs/firebase"
class TokenHandler
{
public:
    TokenHandler();
    ~TokenHandler();
    bool verify_token(const std::string& token, std::string& uid, std::uint64_t& expire);

private:
    const std::string get_key_pass(const std::string& kid) const;
    const std::string read_pem_file(const std::string& file_path) const;

};


#endif // TOKEN_HANDLER_HPP