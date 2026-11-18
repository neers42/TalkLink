#include <gtest/gtest.h>

#include "Domain/SessionHandler.hpp"
#include "Domain/UserRepositoryHandler.hpp"
#include "Domain/UserAggregateStore.hpp"
#include "ServerTime.hpp"
#include "../Application/TestFakes.hpp"

#include <openssl/evp.h>
#include <iomanip>
#include <sstream>

namespace {
// SessionHandler::hash_tokenはprivateなので、テストでは同一のSHA-256ロジックを
// ここで再現してFakeRedisAdapterへ同じキーでセッションを仕込む。
std::string hash_token_for_test(const std::string& token) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_length = 0;
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, token.data(), token.size());
    EVP_DigestFinal_ex(ctx, hash, &hash_length);
    EVP_MD_CTX_free(ctx);

    std::ostringstream oss;
    for (unsigned int i = 0; i < hash_length; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return oss.str();
}

class SessionHandlerTest : public ::testing::Test {
protected:
    std::shared_ptr<UserAggregateStore> store = std::make_shared<UserAggregateStore>();
    std::shared_ptr<FakeUserRepository> user_repo = std::make_shared<FakeUserRepository>();
    std::shared_ptr<ServerTime> server_time = std::make_shared<ServerTime>();
    std::shared_ptr<UserRepositoryHandler> user_repository_handler =
        std::make_shared<UserRepositoryHandler>(store, user_repo, server_time);

    std::shared_ptr<FakeRedisAdapter> redis_adapter = std::make_shared<FakeRedisAdapter>();
    std::shared_ptr<FakeMessageRepository> message_repo = std::make_shared<FakeMessageRepository>();

    SessionHandler handler{user_repository_handler, redis_adapter, user_repo, message_repo, server_time};

    void seed_session(const std::string& token, const std::string& uid) {
        // FakeRedisAdapter::get_sessionは"session:"プレフィックス付きで検索するため、
        // 実際のRedisAdapterと同じキー形式で仕込む。
        redis_adapter->sessions["session:" + hash_token_for_test(token)] = uid;
    }

    std::unique_ptr<SessionRequestMessage> make_request(
        SessionRequestMessage::SignType sign_type,
        const std::string& token,
        const std::string& name = "",
        const std::string& mail = "",
        uint64_t last_received_id = 0) {
        MessageBase::CommonField common(MessageBase::MessageType::SESSION_REQUEST, "corr-1");
        return std::make_unique<SessionRequestMessage>(common, sign_type, token, name, mail, last_received_id);
    }
};
}

TEST_F(SessionHandlerTest, SigninWithValidSessionSucceeds)
{
    seed_session("token-abc", "uid-1");
    auto request = make_request(SessionRequestMessage::SignType::SIGNIN, "token-abc");

    auto result = handler.validate_session(request, 1);

    EXPECT_TRUE(result.is_valid);
    EXPECT_EQ(result.uid, "uid-1");
}

TEST_F(SessionHandlerTest, SigninWithUnknownTokenFails)
{
    auto request = make_request(SessionRequestMessage::SignType::SIGNIN, "unknown-token");

    auto result = handler.validate_session(request, 1);

    EXPECT_FALSE(result.is_valid);
    EXPECT_FALSE(result.error_message.empty());
}

TEST_F(SessionHandlerTest, SignupWithValidSessionInsertsUserAndSucceeds)
{
    seed_session("token-abc", "uid-1");
    auto request = make_request(SessionRequestMessage::SignType::SIGNUP, "token-abc", "Alice", "alice@example.com");

    auto result = handler.validate_session(request, 1);

    EXPECT_TRUE(result.is_valid);
    EXPECT_TRUE(user_repo->insert_user_info_data_called);
    EXPECT_EQ(user_repo->last_inserted_uid, "uid-1");
}

TEST_F(SessionHandlerTest, SignupFailsWhenDbInsertFails)
{
    seed_session("token-abc", "uid-1");
    user_repo->insert_user_info_data_result = false;
    auto request = make_request(SessionRequestMessage::SignType::SIGNUP, "token-abc", "Alice", "alice@example.com");

    auto result = handler.validate_session(request, 1);

    EXPECT_FALSE(result.is_valid);
}

TEST_F(SessionHandlerTest, SignoutDeletesSessionFromRedisAndSucceeds)
{
    seed_session("token-abc", "uid-1");
    const std::string expected_key = "session:" + hash_token_for_test("token-abc");
    auto request = make_request(SessionRequestMessage::SignType::SIGNOUT, "token-abc");

    auto result = handler.validate_session(request, 1);

    EXPECT_TRUE(result.is_valid);
    EXPECT_EQ(result.uid, "uid-1");
    // delete_keyが呼ばれてsessionsから該当キーが消えていること
    EXPECT_EQ(redis_adapter->sessions.find(expected_key), redis_adapter->sessions.end());
}

TEST_F(SessionHandlerTest, ValidateSessionRejectsNullMessage)
{
    std::unique_ptr<SessionRequestMessage> null_request;
    auto result = handler.validate_session(null_request, 1);

    EXPECT_FALSE(result.is_valid);
    EXPECT_FALSE(result.error_message.empty());
}

TEST_F(SessionHandlerTest, ValidateSessionRejectsEmptySessionToken)
{
    auto request = make_request(SessionRequestMessage::SignType::SIGNIN, "");

    auto result = handler.validate_session(request, 1);

    EXPECT_FALSE(result.is_valid);
}
