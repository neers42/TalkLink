#include <gtest/gtest.h>
#include <cstring>

#include "Application/Services/ChatService.hpp"
#include "Domain/ChatroomRepositoryHandler.hpp"
#include "Domain/ChatroomSequenceHandler.hpp"
#include "Domain/UserRepositoryHandler.hpp"
#include "Domain/UserAggregateStore.hpp"
#include "Domain/Entity/Messages/ChatMessage.hpp"
#include "ServerTime.hpp"
#include "TestFakes.hpp"

namespace {

const std::string kRoomIdHex = std::string(32, 'a');
const std::string kSenderUid = std::string(28, 's');
const socket_t kSenderSocket = 222;

class ChatServiceTest : public ::testing::Test {
protected:
    // max_resend_count=1にして、配信失敗テストで再送ループ(sleepを含む)が
    // 空ループで即終了するようにする(execute_message_resendの冒頭1秒sleepのみ)
    void SetUp() override {
        fake_delivery = std::make_shared<FakeMessageDeliveryService>();
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

        // max_resend_count=2: 再送ループを1回だけ回し、失敗シナリオの
        // execute_message_resendが実際に"失敗"を返せるようにする
        // (1だとループ本体が1回も実行されず常に成功扱いになってしまう)
        service = std::make_shared<ChatService>(
            fake_delivery, user_repository_handler, chatroom_repository_handler, chatroom_sequence_handler,
            /*max_resend_count=*/2);

        sockaddr_in dummy_addr{};
        std::memset(&dummy_addr, 0, sizeof(dummy_addr));
        user_repository_handler->add_user(kSenderSocket, nullptr, dummy_addr, "127.0.0.1");
        user_repository_handler->update_user_info(kSenderSocket, kSenderUid, "sender", "sender@example.com");

        fake_chatroom_repo->room_members[kRoomIdHex] = { kSenderUid };
    }

    json build_chat_message_json(const std::string& room_id, const std::string& sender_uid, const std::string& content) {
        json request_json = build_common_fields_json("CHAT_MESSAGE", "corr-1");
        request_json["payload"]["room_id"] = room_id;
        request_json["payload"]["sender_uid"] = sender_uid;
        request_json["payload"]["content"] = content;
        request_json["payload"]["mention_ids"] = json::array();
        return request_json;
    }

    std::shared_ptr<FakeMessageDeliveryService> fake_delivery;
    std::shared_ptr<FakeChatroomRepository> fake_chatroom_repo;
    std::shared_ptr<FakeUserRepository> fake_user_repo;
    std::shared_ptr<FakeMessageRepository> fake_message_repo;
    std::shared_ptr<FakeRedisAdapter> fake_redis;
    std::shared_ptr<ChatroomRepositoryHandler> chatroom_repository_handler;
    std::shared_ptr<ChatroomSequenceHandler> chatroom_sequence_handler;
    std::shared_ptr<UserAggregateStore> user_store;
    std::shared_ptr<ServerTime> server_time;
    std::shared_ptr<UserRepositoryHandler> user_repository_handler;
    std::shared_ptr<ChatService> service;
};

} // namespace

TEST_F(ChatServiceTest, ValidMessageIsDeliveredAndAcknowledged)
{
    const json request = build_chat_message_json(kRoomIdHex, kSenderUid, "hello");

    const bool result = service->process_chat_message(request, kSenderSocket);

    EXPECT_TRUE(result);
    ASSERT_EQ(fake_delivery->delivered_to_room.size(), 1u);
    ASSERT_EQ(fake_delivery->delivered_p2p.size(), 1u); // ACK
    EXPECT_EQ(fake_delivery->delivered_p2p[0].second, kSenderUid);
    EXPECT_TRUE(fake_message_repo->last_insert_called);
}

TEST_F(ChatServiceTest, SpoofedSenderUidIsRejected)
{
    const std::string attacker_claimed_uid = std::string(28, 'z');
    const json request = build_chat_message_json(kRoomIdHex, attacker_claimed_uid, "hello");

    const bool result = service->process_chat_message(request, kSenderSocket);

    EXPECT_FALSE(result);
    EXPECT_TRUE(fake_delivery->delivered_to_room.empty());
}

TEST_F(ChatServiceTest, UnknownRoomFailsToSaveAndReturnsFalse)
{
    const std::string unknown_room = std::string(32, 'b');
    const json request = build_chat_message_json(unknown_room, kSenderUid, "hello");

    const bool result = service->process_chat_message(request, kSenderSocket);

    // 配信自体はdeliver_to_room(フェイクなので常に成功)するが、
    // chatroom_existsがfalseのためDB保存に失敗しfalseが返る
    EXPECT_FALSE(result);
}

TEST_F(ChatServiceTest, RoomSequenceAdvancesOnEachMessage)
{
    const json request1 = build_chat_message_json(kRoomIdHex, kSenderUid, "first");
    ASSERT_TRUE(service->process_chat_message(request1, kSenderSocket));

    const json request2 = build_chat_message_json(kRoomIdHex, kSenderUid, "second");
    ASSERT_TRUE(service->process_chat_message(request2, kSenderSocket));

    EXPECT_EQ(chatroom_sequence_handler->get_current_sequence(kRoomIdHex), 2u);
}

TEST_F(ChatServiceTest, DeliveryFailureStillReturnsResultForAcknowledgment)
{
    // deliver_to_room/deliver_p2pの両方が失敗し、再送対象も残り続けるケース。
    // 再送ロジックが実際に動く(1秒のsleepを含む)ため、他のテストより時間がかかる。
    fake_delivery->should_succeed = false;
    fake_delivery->unsent_uids_to_return = { kSenderUid };

    const json request = build_chat_message_json(kRoomIdHex, kSenderUid, "hello");
    const bool result = service->process_chat_message(request, kSenderSocket);

    EXPECT_FALSE(result);
}
