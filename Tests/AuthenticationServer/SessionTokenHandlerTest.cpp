#include <gtest/gtest.h>
#include <unordered_set>

#include "Domain/SessionTokenHandler.hpp"

namespace {
bool is_hex_string(const std::string& s)
{
    return !s.empty() && s.find_first_not_of("0123456789abcdef") == std::string::npos;
}
}

TEST(SessionTokenHandlerTest, GenerateTokenReturns64CharHexString)
{
    SessionTokenHandler handler;
    const std::string token = handler.generate_token();

    EXPECT_EQ(token.size(), 64u); // 256bit = 32byte -> 64 hex chars
    EXPECT_TRUE(is_hex_string(token));
}

TEST(SessionTokenHandlerTest, GenerateTokenIsNotDeterministic)
{
    SessionTokenHandler handler;
    std::unordered_set<std::string> tokens;
    for (int i = 0; i < 50; ++i) {
        tokens.insert(handler.generate_token());
    }

    // 乱数生成なので50回呼んでも衝突しないはず
    EXPECT_EQ(tokens.size(), 50u);
}

TEST(SessionTokenHandlerTest, HashTokenReturns64CharHexString)
{
    SessionTokenHandler handler;
    const std::string hash = handler.hash_token("some-session-token");

    EXPECT_EQ(hash.size(), 64u); // SHA-256 = 32byte -> 64 hex chars
    EXPECT_TRUE(is_hex_string(hash));
}

TEST(SessionTokenHandlerTest, HashTokenIsDeterministic)
{
    SessionTokenHandler handler;
    const std::string hash1 = handler.hash_token("same-input");
    const std::string hash2 = handler.hash_token("same-input");

    EXPECT_EQ(hash1, hash2);
}

TEST(SessionTokenHandlerTest, HashTokenDiffersForDifferentInput)
{
    SessionTokenHandler handler;
    const std::string hash1 = handler.hash_token("input-a");
    const std::string hash2 = handler.hash_token("input-b");

    EXPECT_NE(hash1, hash2);
}

TEST(SessionTokenHandlerTest, HashTokenHandlesEmptyInput)
{
    SessionTokenHandler handler;
    const std::string hash = handler.hash_token("");

    EXPECT_EQ(hash.size(), 64u);
    EXPECT_TRUE(is_hex_string(hash));
}
