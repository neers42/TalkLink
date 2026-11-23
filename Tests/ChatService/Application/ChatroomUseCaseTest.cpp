#include <gtest/gtest.h>
#include <cstring>

#include "Application/UseCases/ChatroomUseCase.hpp"
#include "Domain/ChatroomRepositoryHandler.hpp"
#include "Domain/ChatroomSequenceHandler.hpp"
#include "Domain/UserRepositoryHandler.hpp"
#include "Domain/UserAggregateStore.hpp"
#include "Domain/Messages/ChatroomControlMessage.hpp"
#include "ServerTime.hpp"
#include "TestFakes.hpp"

namespace {

using RoomAction = ChatroomControlMessage::RoomAction;

const std::string kRoomIdHex = std::string(32, 'a');
const std::string kCreatorUid = std::string(28, 'c');
const std::string kMemberUid = std::string(28, 'm');
const socket_t kCreatorSocket = 111;

class ChatroomUseCaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        fake_delivery = std::make_shared<FakeMessageSender>();
        fake_chatroom_repo = std::make_shared<FakeChatroomRepository>();
        fake_user_repo = std::make_shared<FakeUserRepository>();
        fake_message_repo = std::make_shared<FakeMessageRepository>();
        fake_redis = std::make_shared<FakeRedisAdapter>();

        chatroom_repository_handler = std::make_shared<ChatroomRepositoryHandler>(
            fake_chatroom_repo, fake_user_repo, fake_message_repo, /*max_user_num_in_chatroom=*/100);
        chatroom_sequence_handler = std::make_shared<ChatroomSequenceHandler>(fake_redis);

        user_store = std::make_shared<UserAggregateStore>();
        server_time = std::make_shared<ServerTime>();
        user_repository_handler = std::make_shared<UserRepositoryHandler>(user_store, fake_user_repo, server_time);

        service = std::make_shared<ChatroomUseCase>(
            fake_delivery, chatroom_repository_handler, chatroom_sequence_handler, user_repository_handler);

        sockaddr_in dummy_addr{};
        std::memset(&dummy_addr, 0, sizeof(dummy_addr));
        user_repository_handler->add_user(kCreatorSocket, nullptr, dummy_addr, "127.0.0.1");
        user_repository_handler->update_user_info(kCreatorSocket, kCreatorUid, "creator", "creator@example.com");
    }

    json build_control_json(
        const std::string& sender_uid,
        const std::string& room_id,
        const std::vector<std::string>& member_list,
        const std::string& room_name,
        RoomAction action)
    {
        json request_json = build_common_fields_json("CHATROOM_CONTROL", "corr-1");
        request_json["payload"]["sender_uid"] = sender_uid;
        request_json["payload"]["room_id"] = room_id;
        request_json["payload"]["room_member_list"] = member_list;
        request_json["payload"]["room_name"] = room_name;
        request_json["payload"]["icon_url"] = "";
        request_json["payload"]["room_action"] = room_action_to_string(action);
        return request_json;
    }

    static std::string room_action_to_string(RoomAction action) {
        switch (action) {
            case RoomAction::CREATE_ROOM: return "CREATE_ROOM";
            case RoomAction::LEAVE_ROOM: return "LEAVE_ROOM";
            case RoomAction::ADD_MEMBER: return "ADD_MEMBER";
            case RoomAction::REMOVE_MEMBER: return "REMOVE_MEMBER";
            case RoomAction::ROOM_NAME_CHANGE: return "ROOM_NAME_CHANGE";
            case RoomAction::ROOM_ICON_CHANGE: return "ROOM_ICON_CHANGE";
            case RoomAction::DELETE_ROOM: return "DELETE_ROOM";
        }
        return "";
    }

    std::shared_ptr<FakeMessageSender> fake_delivery;
    std::shared_ptr<FakeChatroomRepository> fake_chatroom_repo;
    std::shared_ptr<FakeUserRepository> fake_user_repo;
    std::shared_ptr<FakeMessageRepository> fake_message_repo;
    std::shared_ptr<FakeRedisAdapter> fake_redis;
    std::shared_ptr<ChatroomRepositoryHandler> chatroom_repository_handler;
    std::shared_ptr<ChatroomSequenceHandler> chatroom_sequence_handler;
    std::shared_ptr<UserAggregateStore> user_store;
    std::shared_ptr<ServerTime> server_time;
    std::shared_ptr<UserRepositoryHandler> user_repository_handler;
    std::shared_ptr<ChatroomUseCase> service;
};

} // namespace

TEST_F(ChatroomUseCaseTest, CreateRoomPersistsMembersAndReturnsSuccess)
{
    const json request = build_control_json(
        kCreatorUid, "", { kMemberUid }, "room name", RoomAction::CREATE_ROOM);

    const auto result = service->process_chatroom_operation(request, kCreatorSocket);

    EXPECT_TRUE(result.success);
    ASSERT_FALSE(result.room_id.empty());
    const auto members = fake_chatroom_repo->get_member_uids(result.room_id);
    EXPECT_EQ(members.size(), 2u); // creator + member
}

TEST_F(ChatroomUseCaseTest, LeaveRoomRemovesOnlyRequesterRegardlessOfPayload)
{
    fake_chatroom_repo->room_members[kRoomIdHex] = { kCreatorUid, kMemberUid };

    // payloadのroom_member_listに何を入れても、本人(kCreatorUid)しか退出させられない
    const json request = build_control_json(
        kCreatorUid, kRoomIdHex, { kMemberUid }, "room name", RoomAction::LEAVE_ROOM);

    const auto result = service->process_chatroom_operation(request, kCreatorSocket);

    EXPECT_TRUE(result.success);
    const auto members = fake_chatroom_repo->get_member_uids(kRoomIdHex);
    EXPECT_EQ(members.size(), 1u);
    EXPECT_EQ(members[0], kMemberUid); // kMemberUidは残っている
}

TEST_F(ChatroomUseCaseTest, SpoofedSenderUidIsRejected)
{
    // ソケットはkCreatorUidとして認証済みだが、payloadには別人のuidを指定する
    const std::string attacker_claimed_uid = std::string(28, 'z');
    fake_chatroom_repo->room_members[kRoomIdHex] = { attacker_claimed_uid };

    const json request = build_control_json(
        attacker_claimed_uid, kRoomIdHex, {}, "room name", RoomAction::LEAVE_ROOM);

    const auto result = service->process_chatroom_operation(request, kCreatorSocket);

    EXPECT_FALSE(result.success);
}

TEST_F(ChatroomUseCaseTest, NonMemberCannotOperateOnRoom)
{
    // kRoomIdHexにはkCreatorUidが参加していない
    fake_chatroom_repo->room_members[kRoomIdHex] = { kMemberUid };

    const json request = build_control_json(
        kCreatorUid, kRoomIdHex, { std::string(28, 'n') }, "room name", RoomAction::ADD_MEMBER);

    const auto result = service->process_chatroom_operation(request, kCreatorSocket);

    EXPECT_FALSE(result.success);
}

TEST_F(ChatroomUseCaseTest, DeleteRoomRemovesRoomSequence)
{
    fake_chatroom_repo->room_members[kRoomIdHex] = { kCreatorUid };
    chatroom_sequence_handler->initialize_room_sequence(kRoomIdHex, 5);

    const json request = build_control_json(
        kCreatorUid, kRoomIdHex, {}, "room name", RoomAction::DELETE_ROOM);

    const auto result = service->process_chatroom_operation(request, kCreatorSocket);

    EXPECT_TRUE(result.success);
    EXPECT_TRUE(fake_chatroom_repo->deleted_rooms.count(kRoomIdHex));
    EXPECT_EQ(chatroom_sequence_handler->get_current_sequence(kRoomIdHex), 0u);
}
