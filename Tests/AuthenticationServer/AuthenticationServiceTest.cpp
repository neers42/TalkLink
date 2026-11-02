#include <gtest/gtest.h>

#include "Application/Services/AuthenticationService.hpp"
#include "Domain/Interfaces/ITokenHandler.hpp"
#include "Domain/Interfaces/IRedisAdapter.hpp"
#include "Domain/SessionTokenHandler.hpp"

namespace {

// ITokenHandlerのテスト用フェイク実装(実際のFirebase証明書を使わない)
class FakeTokenHandler : public ITokenHandler {
public:
    bool should_succeed = true;
    std::string fake_uid = "uid_test_0000000001";
    std::uint64_t fake_expire = 1234567890;

    bool verify_token(const std::string& /*token*/, std::string& uid, std::uint64_t& expire) override {
        if (!should_succeed) {
            return false;
        }
        uid = fake_uid;
        expire = fake_expire;
        return true;
    }
};

// IRedisAdapterのテスト用フェイク実装(実際のRedisサーバーに接続しない)
class FakeRedisAdapter : public IRedisAdapter {
public:
    bool should_succeed = true;
    std::string last_uid;
    std::string last_session_token_hash;
    std::uint64_t ttl_to_return = 3600;

    bool save_session(const std::string& uid, const std::string& session_token_hash,
                       std::uint64_t /*expire*/, std::uint64_t& ttl) override {
        last_uid = uid;
        last_session_token_hash = session_token_hash;
        ttl = ttl_to_return;
        return should_succeed;
    }
};

class AuthenticationServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        fake_token_handler = std::make_shared<FakeTokenHandler>();
        fake_redis_adapter = std::make_shared<FakeRedisAdapter>();
        session_token_handler = std::make_shared<SessionTokenHandler>();
        service = std::make_shared<AuthenticationService>(
            fake_token_handler, fake_redis_adapter, session_token_handler);
    }

    std::shared_ptr<FakeTokenHandler> fake_token_handler;
    std::shared_ptr<FakeRedisAdapter> fake_redis_adapter;
    std::shared_ptr<SessionTokenHandler> session_token_handler;
    std::shared_ptr<AuthenticationService> service;
};

} // namespace

TEST_F(AuthenticationServiceTest, ValidJwtProducesSessionTokenAndSavesSession)
{
    std::string uid, session_token;
    std::uint64_t expire = 0, ttl = 0;

    const bool result = service->verify_auth_process("dummy.jwt.token", uid, expire, ttl, session_token);

    EXPECT_TRUE(result);
    EXPECT_EQ(uid, "uid_test_0000000001");
    EXPECT_EQ(expire, 1234567890u);
    EXPECT_EQ(ttl, 3600u);
    EXPECT_EQ(session_token.size(), 64u); // SessionTokenHandler::generate_tokenの実装に基づく
    EXPECT_EQ(fake_redis_adapter->last_uid, "uid_test_0000000001");
    EXPECT_FALSE(fake_redis_adapter->last_session_token_hash.empty());
}

TEST_F(AuthenticationServiceTest, InvalidJwtFailsWithoutTouchingRedis)
{
    fake_token_handler->should_succeed = false;
    std::string uid, session_token;
    std::uint64_t expire = 0, ttl = 0;

    const bool result = service->verify_auth_process("bad.jwt.token", uid, expire, ttl, session_token);

    EXPECT_FALSE(result);
    EXPECT_TRUE(session_token.empty());
    EXPECT_TRUE(fake_redis_adapter->last_uid.empty()); // Redisには保存されていないはず
}

TEST_F(AuthenticationServiceTest, RedisSaveFailureReturnsFalse)
{
    fake_redis_adapter->should_succeed = false;
    std::string uid, session_token;
    std::uint64_t expire = 0, ttl = 0;

    const bool result = service->verify_auth_process("dummy.jwt.token", uid, expire, ttl, session_token);

    EXPECT_FALSE(result);
    // JWT検証自体は成功しているので、uidはセットされている
    EXPECT_EQ(uid, "uid_test_0000000001");
}

TEST_F(AuthenticationServiceTest, EachCallGeneratesADifferentSessionToken)
{
    std::string uid1, session_token1;
    std::uint64_t expire1 = 0, ttl1 = 0;
    service->verify_auth_process("dummy.jwt.token", uid1, expire1, ttl1, session_token1);

    std::string uid2, session_token2;
    std::uint64_t expire2 = 0, ttl2 = 0;
    service->verify_auth_process("dummy.jwt.token", uid2, expire2, ttl2, session_token2);

    EXPECT_NE(session_token1, session_token2);
}
