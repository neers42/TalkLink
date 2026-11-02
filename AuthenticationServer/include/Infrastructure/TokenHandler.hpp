#ifndef TOKEN_HANDLER_HPP
#define TOKEN_HANDLER_HPP
#include "common.h"
#include "Domain/Interfaces/ITokenHandler.hpp"
#define CERTS_DIR "/app/AuthenticationServer/certs/firebase"
class TokenHandler : public ITokenHandler
{
public:
    explicit TokenHandler(const std::string& firebase_project_id);
    ~TokenHandler();
    bool verify_token(const std::string& token, std::string& uid, std::uint64_t& expire) override;

private:
    std::string _firebase_project_id; // auth.confの[Firebase]project_idから注入される
    const std::string get_key_pass(const std::string& kid) const;
    const std::string read_pem_file(const std::string& file_path) const;

};


#endif // TOKEN_HANDLER_HPP