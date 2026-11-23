#include <gtest/gtest.h>

#include "Domain/ChatroomRepositoryHandler.hpp"
#include "../Application/TestFakes.hpp"

namespace {
constexpr int kMaxUserNumInChatroom = 3;

class ChatroomRepositoryHandlerTest : public ::testing::Test {
protected:
    std::shared_ptr<FakeChatroomRepository> chatroom_repo = std::make_shared<FakeChatroomRepository>();
    std::shared_ptr<FakeUserRepository> user_repo = std::make_shared<FakeUserRepository>();
    std::shared_ptr<FakeMessageRepository> message_repo = std::make_shared<FakeMessageRepository>();
    ChatroomRepositoryHandler handler{chatroom_repo, user_repo, message_repo, kMaxUserNumInChatroom};
};
}

TEST_F(ChatroomRepositoryHandlerTest, AddMembersReturnsAddedCount)
{
    int added = handler.add_members("room-1", {"uid-1", "uid-2"});
    EXPECT_EQ(added, 2);
}

TEST_F(ChatroomRepositoryHandlerTest, AddMembersReturnsMinusTwoWhenLimitExceeded)
{
    int result = handler.add_members("room-1", {"uid-1", "uid-2", "uid-3", "uid-4"}); // 上限3を超える
    EXPECT_EQ(result, -2);
}

TEST_F(ChatroomRepositoryHandlerTest, RemoveMembersRemovesExistingMembers)
{
    handler.add_members("room-1", {"uid-1", "uid-2"});
    bool result = handler.remove_members("room-1", {"uid-1"});

    EXPECT_TRUE(result);
    auto remaining = handler.get_member_uids("room-1");
    EXPECT_EQ(remaining.size(), 1u);
    EXPECT_EQ(remaining[0], "uid-2");
}

TEST_F(ChatroomRepositoryHandlerTest, ChangeRoomNameFailsForEmptyName)
{
    EXPECT_FALSE(handler.change_room_name("room-1", ""));
}

TEST_F(ChatroomRepositoryHandlerTest, ChangeRoomNameSucceedsForNonEmptyName)
{
    EXPECT_TRUE(handler.change_room_name("room-1", "new-name"));
    EXPECT_EQ(chatroom_repo->room_names["room-1"], "new-name");
}

TEST_F(ChatroomRepositoryHandlerTest, ChangeRoomIconFailsForEmptyUrl)
{
    EXPECT_FALSE(handler.change_room_icon("room-1", ""));
}

TEST_F(ChatroomRepositoryHandlerTest, ChangeRoomIconSucceedsForNonEmptyUrl)
{
    EXPECT_TRUE(handler.change_room_icon("room-1", "icon.png"));
    EXPECT_EQ(chatroom_repo->room_icons["room-1"], "icon.png");
}

TEST_F(ChatroomRepositoryHandlerTest, InsertChatroomWithMembersFailsForEmptyMembers)
{
    EXPECT_FALSE(handler.insert_chatroom_with_members("room-1", "room-name", "icon.png", {}));
}

TEST_F(ChatroomRepositoryHandlerTest, InsertChatroomWithMembersPersistsRoomAndMembers)
{
    bool result = handler.insert_chatroom_with_members("room-1", "room-name", "icon.png", {"uid-1", "uid-2"});

    EXPECT_TRUE(result);
    EXPECT_TRUE(handler.chatroom_exists("room-1"));
    auto members = handler.get_member_uids("room-1");
    EXPECT_EQ(members.size(), 2u);
}

TEST_F(ChatroomRepositoryHandlerTest, DeleteChatroomAndPersistRemovesRoom)
{
    handler.insert_chatroom_with_members("room-1", "room-name", "icon.png", {"uid-1"});
    EXPECT_TRUE(handler.delete_chatroom_and_persist("room-1"));
    EXPECT_FALSE(handler.chatroom_exists("room-1"));
}

TEST_F(ChatroomRepositoryHandlerTest, ChatroomExistsFalseForUnknownRoom)
{
    EXPECT_FALSE(handler.chatroom_exists("unknown-room"));
}

TEST_F(ChatroomRepositoryHandlerTest, FindDirectRoomByMembersReturnsEmptyForSameUid)
{
    EXPECT_EQ(handler.find_direct_room_by_members("uid-1", "uid-1"), "");
}

TEST_F(ChatroomRepositoryHandlerTest, FindDirectRoomByMembersFindsExistingDirectRoom)
{
    handler.insert_chatroom_with_members("room-1", "room-name", "icon.png", {"uid-1", "uid-2"});
    EXPECT_EQ(handler.find_direct_room_by_members("uid-1", "uid-2"), "room-1");
}

TEST_F(ChatroomRepositoryHandlerTest, InsertMessageDelegatesToMessageRepository)
{
    bool result = handler.insert_message(1, "room-1", "uid-1", "2026-01-01 00:00:00", "hello", 1, "");
    EXPECT_TRUE(result);
    EXPECT_TRUE(message_repo->last_insert_called);
}

TEST_F(ChatroomRepositoryHandlerTest, GetUnreceivedMessagesDelegatesToMessageRepository)
{
    json result = handler.get_unreceived_messages("uid-1", 0);
    EXPECT_TRUE(result.is_array());
}

TEST_F(ChatroomRepositoryHandlerTest, GetRoomMessagesAfterSequenceDelegatesToMessageRepository)
{
    message_repo->gap_messages_to_return = json::array({"msg-1", "msg-2"});
    json result = handler.get_room_messages_after_sequence("room-1", 0, "uid-1");

    ASSERT_TRUE(result.contains("gap_messages"));
    EXPECT_EQ(result["gap_messages"].size(), 2u);
}

TEST_F(ChatroomRepositoryHandlerTest, GetUserInfosReturnsInfoForEachUid)
{
    auto infos = handler.get_user_infos({"uid-1", "uid-2"});
    ASSERT_EQ(infos.size(), 2u);
    EXPECT_EQ(infos[0].uid, "uid-1");
    EXPECT_EQ(infos[1].uid, "uid-2");
}

TEST_F(ChatroomRepositoryHandlerTest, GetMemberUidsReturnsEmptyForUnknownRoom)
{
    EXPECT_TRUE(handler.get_member_uids("unknown-room").empty());
}
