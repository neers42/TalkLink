#include <gtest/gtest.h>

#include "Domain/Entity/User.hpp"

TEST(UserEntityExtendedTest, DefaultConstructorInitializesEmptyState)
{
    User user;
    EXPECT_EQ(user.get_id(), "");
    EXPECT_EQ(user.get_name(), "");
    EXPECT_EQ(user.get_mail_address(), "");
    EXPECT_EQ(user.get_heart_beat_count(), 0);
    EXPECT_FALSE(user.get_auth_status());
    EXPECT_EQ(user.get_connect_state(), User::ConnectState::ACTIVE);
}

TEST(UserEntityExtendedTest, ParameterizedConstructorSetsFields)
{
    User user("uid-1", "Alice", "alice@example.com");
    EXPECT_EQ(user.get_id(), "uid-1");
    EXPECT_EQ(user.get_name(), "Alice");
    EXPECT_EQ(user.get_mail_address(), "alice@example.com");
}

TEST(UserEntityExtendedTest, HeartBeatCountIncrementsAndClears)
{
    User user;
    user.increment_heart_beat_count();
    user.increment_heart_beat_count();
    EXPECT_EQ(user.get_heart_beat_count(), 2);

    user.clear_heart_beat_count();
    EXPECT_EQ(user.get_heart_beat_count(), 0);
}

TEST(UserEntityExtendedTest, SetHeartBeatCountOverridesValue)
{
    User user;
    user.set_heart_beat_count(5);
    EXPECT_EQ(user.get_heart_beat_count(), 5);
}

TEST(UserEntityExtendedTest, ChangeAuthStatusTogglesBothDirections)
{
    User user;
    EXPECT_FALSE(user.get_auth_status());

    user.change_auth_status();
    EXPECT_TRUE(user.get_auth_status());

    user.change_auth_status();
    EXPECT_FALSE(user.get_auth_status());
}

TEST(UserEntityExtendedTest, SetAuthStatusDirectlyOverridesValue)
{
    User user;
    user.set_auth_status(true);
    EXPECT_TRUE(user.get_auth_status());
}

TEST(UserEntityExtendedTest, SetConnectStateUpdatesIsActive)
{
    User user;
    EXPECT_TRUE(user.is_active());

    user.set_connect_state(User::ConnectState::DISCONNECTING);
    EXPECT_FALSE(user.is_active());
    EXPECT_EQ(user.get_connect_state(), User::ConnectState::DISCONNECTING);

    user.set_connect_state(User::ConnectState::DISCONNECT);
    EXPECT_EQ(user.get_connect_state(), User::ConnectState::DISCONNECT);
}

TEST(UserEntityExtendedTest, CompareAndSetStateSucceedsWhenExpectedMatches)
{
    User user;
    User::ConnectState expected = User::ConnectState::ACTIVE;
    bool result = user.compare_and_set_state(expected, User::ConnectState::DISCONNECTING);

    EXPECT_TRUE(result);
    EXPECT_EQ(user.get_connect_state(), User::ConnectState::DISCONNECTING);
}

TEST(UserEntityExtendedTest, CompareAndSetStateFailsAndUpdatesExpectedWhenMismatched)
{
    User user; // ACTIVE
    User::ConnectState expected = User::ConnectState::DISCONNECTING; // 実際の状態と不一致
    bool result = user.compare_and_set_state(expected, User::ConnectState::DISCONNECT);

    EXPECT_FALSE(result);
    // 状態は変更されない
    EXPECT_EQ(user.get_connect_state(), User::ConnectState::ACTIVE);
    // std::atomic::compare_exchange_strongの仕様により、失敗時はexpectedに実際の値が書き込まれる
    EXPECT_EQ(expected, User::ConnectState::ACTIVE);
}

TEST(UserEntityExtendedTest, CanDisconnectIsTrueForActiveAndDisconnecting)
{
    User user;
    EXPECT_TRUE(user.can_disconnect()); // ACTIVE

    user.set_connect_state(User::ConnectState::DISCONNECTING);
    EXPECT_TRUE(user.can_disconnect());

    user.set_connect_state(User::ConnectState::DISCONNECT);
    EXPECT_FALSE(user.can_disconnect());
}

TEST(UserEntityExtendedTest, EqualityOperatorComparesById)
{
    User user_a("uid-1", "Alice", "alice@example.com");
    User user_b("uid-1", "Bob", "bob@example.com"); // idのみ一致
    User user_c("uid-2", "Alice", "alice@example.com"); // id不一致

    EXPECT_TRUE(user_a == user_b);
    EXPECT_FALSE(user_a == user_c);
}

TEST(UserEntityExtendedTest, UpdateLastActivityRoundTrips)
{
    User user;
    const time_t ts = 1700000000;
    user.update_last_activity(ts);
    EXPECT_EQ(user.get_last_activity(), ts);
}
