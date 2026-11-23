#include <gtest/gtest.h>
#include <cstring>

#include "Domain/UserRepositoryHandler.hpp"
#include "Domain/UserAggregateStore.hpp"
#include "Domain/Entity/User.hpp"
#include "Domain/Interfaces/IUserRepository.hpp"
#include "ServerTime.hpp"

namespace {

// IUserRepositoryのテスト用フェイク実装(DBに一切アクセスしない)
class FakeUserRepository : public IUserRepository {
public:
    bool insert_user_info_data(const std::string&, const std::string&, const std::string&,
                                const std::string&, const std::string&, const std::string&) override {
        return true;
    }
    UserInfo get_user_info(const std::string&) override { return UserInfo{}; }
    bool update_last_login(const std::string&, const std::string&) override { return true; }
    UserExistence user_exists(const std::string&) override { return UserExistence::NOT_FOUND; }
    std::string get_user_last_login(const std::string&) override { return ""; }
    std::vector<UserInfo> search_user_infos(const std::string&) override { return {}; }
};

class UserAuthStatusTest : public ::testing::Test {
protected:
    void SetUp() override {
        store = std::make_shared<UserAggregateStore>();
        fake_repo = std::make_shared<FakeUserRepository>();
        server_time = std::make_shared<ServerTime>();
        handler = std::make_shared<UserRepositoryHandler>(store, fake_repo, server_time);
        std::memset(&dummy_addr, 0, sizeof(dummy_addr));
    }

    std::shared_ptr<UserAggregateStore> store;
    std::shared_ptr<FakeUserRepository> fake_repo;
    std::shared_ptr<ServerTime> server_time;
    std::shared_ptr<UserRepositoryHandler> handler;
    sockaddr_in dummy_addr{};
    const socket_t kSocket = 123;
};

} // namespace

// ==================== User エンティティ単体 ====================

TEST(UserEntityTest, DefaultAuthStatusIsFalse)
{
    User user;
    EXPECT_FALSE(user.is_authenticated());
}

TEST(UserEntityTest, SetAuthStatusTrueMakesAuthenticated)
{
    User user;
    user.set_auth_status(true);
    EXPECT_TRUE(user.is_authenticated());
}

// ==================== UserRepositoryHandler経由の認証ゲート関連 ====================
// 2026-10の「未認証接続によるメッセージ処理となりすまし」修正の再発防止テスト

TEST_F(UserAuthStatusTest, NewlyAcceptedSocketIsNotAuthenticated)
{
    ASSERT_TRUE(handler->add_user(kSocket, nullptr, dummy_addr, "127.0.0.1"));

    auto user = handler->get_user(kSocket);
    ASSERT_NE(user, nullptr);
    EXPECT_FALSE(user->is_authenticated());
    EXPECT_EQ(handler->get_uid_by_socket(kSocket), "");
}

TEST_F(UserAuthStatusTest, UpdateAuthStatusTrueReflectsOnUser)
{
    ASSERT_TRUE(handler->add_user(kSocket, nullptr, dummy_addr, "127.0.0.1"));

    ASSERT_TRUE(handler->update_user_auth_status(kSocket, true));

    auto user = handler->get_user(kSocket);
    ASSERT_NE(user, nullptr);
    EXPECT_TRUE(user->is_authenticated());
}

// SIGNOUT後もidは残るが、is_authenticated()はfalseに戻る、という
// ChatroomUseCase/ChatUseCase等のなりすまし対策チェックだけでは
// 検出できない「ログアウト済みだが本人のuidは一致する」ケースの土台を確認する。
TEST_F(UserAuthStatusTest, SignOutKeepsUidButClearsAuthStatus)
{
    const std::string uid = std::string(28, 'a'); // 28 bytes
    ASSERT_TRUE(handler->add_user(kSocket, nullptr, dummy_addr, "127.0.0.1"));

    // SIGNIN相当
    ASSERT_TRUE(handler->update_user_info(kSocket, uid, "name", "mail@example.com"));
    ASSERT_TRUE(handler->update_user_auth_status(kSocket, true));
    EXPECT_EQ(handler->get_uid_by_socket(kSocket), uid);
    EXPECT_TRUE(handler->get_user(kSocket)->is_authenticated());

    // SIGNOUT相当(uidは再度渡されるがauth_statusはfalseにする)
    ASSERT_TRUE(handler->update_user_info(kSocket, uid, "name", "mail@example.com"));
    ASSERT_TRUE(handler->update_user_auth_status(kSocket, false));

    // uidはまだ残っている(= payloadのuidと一致するチェックだけでは弾けない)
    EXPECT_EQ(handler->get_uid_by_socket(kSocket), uid);
    // しかしauth_statusはfalseに戻っている(= 認証ゲートで弾ける)
    EXPECT_FALSE(handler->get_user(kSocket)->is_authenticated());
}

TEST_F(UserAuthStatusTest, UpdateAuthStatusOnUnknownSocketFails)
{
    EXPECT_FALSE(handler->update_user_auth_status(kSocket, true));
}
