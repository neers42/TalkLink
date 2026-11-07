#include <gtest/gtest.h>

#include "Domain/ChatroomSequenceHandler.hpp"
#include "../Application/TestFakes.hpp"

namespace {

class ChatroomSequenceHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        fake_redis = std::make_shared<FakeRedisAdapter>();
        handler = std::make_shared<ChatroomSequenceHandler>(fake_redis);
    }

    std::shared_ptr<FakeRedisAdapter> fake_redis;
    std::shared_ptr<ChatroomSequenceHandler> handler;
    const std::string kRoomId = std::string(32, 'a');
};

} // namespace

TEST_F(ChatroomSequenceHandlerTest, InitializeRoomSequenceStartsAtGivenValue)
{
    ASSERT_TRUE(handler->initialize_room_sequence(kRoomId, 5));
    EXPECT_EQ(handler->get_current_sequence(kRoomId), 5u);
}

TEST_F(ChatroomSequenceHandlerTest, InitializeDoesNotOverwriteExistingSequence)
{
    ASSERT_TRUE(handler->initialize_room_sequence(kRoomId, 5));
    // 2回目の初期化は既存値を上書きしない(ensure_key_initialized相当のセマンティクス)
    ASSERT_TRUE(handler->initialize_room_sequence(kRoomId, 100));
    EXPECT_EQ(handler->get_current_sequence(kRoomId), 5u);
}

TEST_F(ChatroomSequenceHandlerTest, IncrementRoomSequenceStartsAtOneAndAdvances)
{
    std::uint64_t seq = 0;
    ASSERT_TRUE(handler->increment_room_sequence(kRoomId, seq));
    EXPECT_EQ(seq, 1u);

    ASSERT_TRUE(handler->increment_room_sequence(kRoomId, seq));
    EXPECT_EQ(seq, 2u);

    ASSERT_TRUE(handler->increment_room_sequence(kRoomId, seq));
    EXPECT_EQ(seq, 3u);
}

TEST_F(ChatroomSequenceHandlerTest, DeleteRoomSequenceResetsCounterToZero)
{
    std::uint64_t seq = 0;
    handler->increment_room_sequence(kRoomId, seq);
    handler->increment_room_sequence(kRoomId, seq);
    ASSERT_EQ(handler->get_current_sequence(kRoomId), 2u);

    ASSERT_TRUE(handler->delete_room_sequence(kRoomId));
    EXPECT_EQ(handler->get_current_sequence(kRoomId), 0u);
}

TEST_F(ChatroomSequenceHandlerTest, EmptyRoomIdFailsAllOperations)
{
    std::uint64_t seq = 0;
    EXPECT_FALSE(handler->initialize_room_sequence("", 1));
    EXPECT_FALSE(handler->delete_room_sequence(""));
    EXPECT_FALSE(handler->increment_room_sequence("", seq));
    EXPECT_EQ(handler->get_current_sequence(""), 0u);
}

TEST_F(ChatroomSequenceHandlerTest, GetCurrentSequenceForUnknownRoomIsZero)
{
    EXPECT_EQ(handler->get_current_sequence(kRoomId), 0u);
}
