#include <gtest/gtest.h>

#include "Domain/ChatroomIDGenerator.hpp"

TEST(ChatroomIDGeneratorTest, IssueChatroomIdReturnsNonEmptyString)
{
    ChatroomIDGenerator generator;

    const std::string id = generator.issue_chatroom_id();

    EXPECT_FALSE(id.empty());
}

TEST(ChatroomIDGeneratorTest, IssueChatroomIdReturns26Characters)
{
    ChatroomIDGenerator generator;

    const std::string id = generator.issue_chatroom_id();

    EXPECT_EQ(id.size(), 26);
}

TEST(ChatroomIDGeneratorTest, IssueChatroomIdGeneratesDifferentIds)
{
    ChatroomIDGenerator generator;

    const std::string id1 = generator.issue_chatroom_id();
    const std::string id2 = generator.issue_chatroom_id();

    EXPECT_NE(id1, id2);
}

TEST(ChatroomIDGeneratorTest, IssueChatroomIdUnhexReturns32Characters)
{
    ChatroomIDGenerator generator;

    const std::string id = generator.issue_chatroom_id_unhex();

    EXPECT_EQ(id.size(), 32);
}