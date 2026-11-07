#include <gtest/gtest.h>
#include <cstring>

#include "Application/Services/SyncService.hpp"
#include "Domain/ChatroomRepositoryHandler.hpp"
#include "Domain/UserRepositoryHandler.hpp"
#include "Domain/UserAggregateStore.hpp"
#include "Domain/Entity/Messages/SyncRequestMessage.hpp"
#include "ServerTime.hpp"
#include "TestFakes.hpp"

namespace {

const std::string kValidRoomIdHex = std::string(32, 'a');
const std::string kUid = std::string(28, 'u');
const socket_t kSocket = 777;

class SyncServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        fake_delivery = std::make_shared<FakeMessageDeliveryService>();
        fake_chatroom_repo = std::make_shared<FakeChatroomRepository>();
        fake_user_repo = std::make_shared<FakeUserRepository>();
        fake_message_repo = std::make_shared<FakeMessageRepository>();

        chatroom_repository_handler = std::make_shared<ChatroomRepositoryHandler>(
            fake_chatroom_repo, fake_user_repo, fake_message_repo, /*max_user_num_in_chatroom=*/100);

        user_store = std::make_shared<UserAggregateStore>();
        server_time = std::make_shared<ServerTime>();
        user_repository_handler = std::make_shared<UserRepositoryHandler>(user_store, fake_user_repo, server_time);

        sync_service = std::make_shared<SyncService>(fake_delivery, chatroom_repository_handler, user_repository_handler);

        // ソケットを認証済みユーザーとして登録(target_uidのなりすまし対策チェック用)
        sockaddr_in dummy_addr{};
        std::memset(&dummy_addr, 0, sizeof(dummy_addr));
        user_repository_handler->add_user(kSocket, nullptr, dummy_addr, "127.0.0.1");
        user_repository_handler->update_user_info(kSocket, kUid, "name", "mail@example.com");

        // kUidをルームのメンバーとして登録
        fake_chatroom_repo->room_members[kValidRoomIdHex].insert(kUid);
    }

    json build_sync_request_json(const std::string& room_id, const std::string& target_uid, std::uint64_t from_sequence) {
        json request_json = build_common_fields_json("SYNC_REQUEST", "corr-1");
        request_json["payload"]["room_id"] = room_id;
        request_json["payload"]["target_uid"] = target_uid;
        request_json["payload"]["from_sequence"] = from_sequence;
        return request_json;
    }

    std::shared_ptr<FakeMessageDeliveryService> fake_delivery;
    std::shared_ptr<FakeChatroomRepository> fake_chatroom_repo;
    std::shared_ptr<FakeUserRepository> fake_user_repo;
    std::shared_ptr<FakeMessageRepository> fake_message_repo;
    std::shared_ptr<ChatroomRepositoryHandler> chatroom_repository_handler;
    std::shared_ptr<UserAggregateStore> user_store;
    std::shared_ptr<ServerTime> server_time;
    std::shared_ptr<UserRepositoryHandler> user_repository_handler;
    std::shared_ptr<SyncService> sync_service;
};

} // namespace

TEST_F(SyncServiceTest, ValidRequestDeliversGapMessagesToRequester)
{
    fake_message_repo->gap_messages_to_return = json::array({ json{{"message_id", 1}}, json{{"message_id", 2}} });

    const json request = build_sync_request_json(kValidRoomIdHex, kUid, 10);
    const bool result = sync_service->process_sync_request(request, kSocket);

    EXPECT_TRUE(result);
    ASSERT_EQ(fake_delivery->delivered_p2p.size(), 1u);
    EXPECT_EQ(fake_delivery->delivered_p2p[0].second, kUid);
    const json& delivered_payload = fake_delivery->delivered_p2p[0].first["payload"];
    ASSERT_EQ(delivered_payload["gap_messages"].size(), 2u);
}

TEST_F(SyncServiceTest, SpoofedTargetUidIsRejected)
{
    // ソケットはkUidとして認証済みだが、payloadには別のuidを指定してなりすましを試みる
    const std::string other_uid = std::string(28, 'x');
    fake_chatroom_repo->room_members[kValidRoomIdHex].insert(other_uid);

    const json request = build_sync_request_json(kValidRoomIdHex, other_uid, 0);
    const bool result = sync_service->process_sync_request(request, kSocket);

    EXPECT_FALSE(result);
    EXPECT_TRUE(fake_delivery->delivered_p2p.empty());
}

TEST_F(SyncServiceTest, NonMemberIsRejected)
{
    // kUidはこのルームのメンバーではない
    const std::string other_room = std::string(32, 'b');

    const json request = build_sync_request_json(other_room, kUid, 0);
    const bool result = sync_service->process_sync_request(request, kSocket);

    EXPECT_FALSE(result);
    EXPECT_TRUE(fake_delivery->delivered_p2p.empty());
}

TEST_F(SyncServiceTest, DeliveryFailureReturnsFalse)
{
    fake_delivery->should_succeed = false;

    const json request = build_sync_request_json(kValidRoomIdHex, kUid, 0);
    const bool result = sync_service->process_sync_request(request, kSocket);

    EXPECT_FALSE(result);
}

TEST_F(SyncServiceTest, UnauthenticatedSocketIsRejected)
{
    const socket_t unknown_socket = 999;
    const json request = build_sync_request_json(kValidRoomIdHex, kUid, 0);
    const bool result = sync_service->process_sync_request(request, unknown_socket);

    EXPECT_FALSE(result);
    EXPECT_TRUE(fake_delivery->delivered_p2p.empty());
}
