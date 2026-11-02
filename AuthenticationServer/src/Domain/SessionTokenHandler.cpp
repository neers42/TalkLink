#include "Domain/SessionTokenHandler.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <iomanip>
#include <sstream>
#include <stdexcept>

std::string SessionTokenHandler::generate_token() const
{
    constexpr std::size_t TOKEN_SIZE = 32; // 256bit

    unsigned char buffer[TOKEN_SIZE];

    if (RAND_bytes(buffer, TOKEN_SIZE) != 1)
    {
        throw std::runtime_error("Failed to generate session token");
    }

    std::ostringstream oss;

    for (const auto byte : buffer)
    {
        oss << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<int>(byte);
    }

    return oss.str();
}

std::string SessionTokenHandler::hash_token(const std::string &token) const
{
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_length = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();

    if (ctx == nullptr) {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(
            ctx,
            token.data(),
            token.size()) != 1 ||
        EVP_DigestFinal_ex(
            ctx,
            hash,
            &hash_length) != 1) {

        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("Failed to hash session token");
    }

    EVP_MD_CTX_free(ctx);

    std::ostringstream oss;

    for (unsigned int i = 0; i < hash_length; ++i) {
        oss << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<int>(hash[i]);
    }

    return oss.str();
}
