#include <gtest/gtest.h>

#include "Domain/Entity/Messages/ChatMessage.hpp"
#include "Domain/Entity/Messages/ChatDeliveryMessage.hpp"
#include "Domain/Entity/Messages/SyncRequestMessage.hpp"
#include "Domain/Entity/Messages/SyncResponseMessage.hpp"
#include "Domain/Entity/Messages/SessionResponseMessage.hpp"
#include "Domain/Entity/Messages/UserSearchResponseMessage.hpp"

namespace {
// 28byteのダミーuid、32/36文字のダミーroom_idを用意するヘルパー
const std::string kValidUid = std::string(28, 'a'); // 28 bytes
const std::string kValidRoomIdHex = std::string(32, 'a');    // 32 bytes (HEX形式)
}

// ==================== ChatMessage ====================

TEST(ChatMessageValidationTest, ValidValuesPassValidation)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHAT_MESSAGE, "corr-1");
    ChatMessage msg(common, 1, kValidRoomIdHex, {}, kValidUid, "hello");

    EXPECT_TRUE(msg.validate_value());
}

TEST(ChatMessageValidationTest, InvalidRoomIdLengthFailsValidation)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHAT_MESSAGE, "corr-1");
    ChatMessage msg(common, 1, "too_short_room_id", {}, kValidUid, "hello");

    EXPECT_FALSE(msg.validate_value());
}

TEST(ChatMessageValidationTest, InvalidSenderUidLengthFailsValidation)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHAT_MESSAGE, "corr-1");
    ChatMessage msg(common, 1, kValidRoomIdHex, {}, "too_short_uid", "hello");

    EXPECT_FALSE(msg.validate_value());
}

TEST(ChatMessageValidationTest, InvalidMentionIdLengthFailsValidation)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHAT_MESSAGE, "corr-1");
    ChatMessage msg(common, 1, kValidRoomIdHex, {"short"}, kValidUid, "hello");

    EXPECT_FALSE(msg.validate_value());
}

// ==================== ChatDeliveryMessage ====================
// message_idがserialize_payloadに含まれていないバグ(2026-10頃に発見・修正)の再発防止テスト

TEST(ChatDeliveryMessageTest, SerializedPayloadContainsMessageId)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHAT_DELIVERY, "corr-1");
    ChatDeliveryMessage msg(common, 42, kValidRoomIdHex, kValidUid, "hello", 7);

    const json payload = msg.serialize_payload();

    ASSERT_TRUE(payload.contains("payload"));
    ASSERT_TRUE(payload["payload"].contains("message_id"));
    EXPECT_EQ(payload["payload"]["message_id"].get<uint64_t>(), 42u);
    EXPECT_EQ(payload["payload"]["room_sequence"].get<uint64_t>(), 7u);
}

// ==================== SyncRequestMessage ====================

TEST(SyncRequestMessageValidationTest, ValidValuesPassValidation)
{
    MessageBase::CommonField common(MessageBase::MessageType::SYNC_REQUEST, "corr-1");
    SyncRequestMessage msg(common, kValidRoomIdHex, kValidUid, 10);

    EXPECT_TRUE(msg.validate_value());
}

TEST(SyncRequestMessageValidationTest, InvalidTargetUidLengthFailsValidation)
{
    MessageBase::CommonField common(MessageBase::MessageType::SYNC_REQUEST, "corr-1");
    SyncRequestMessage msg(common, kValidRoomIdHex, "short", 10);

    EXPECT_FALSE(msg.validate_value());
}

TEST(SyncRequestMessageValidationTest, InvalidRoomIdLengthFailsValidation)
{
    MessageBase::CommonField common(MessageBase::MessageType::SYNC_REQUEST, "corr-1");
    SyncRequestMessage msg(common, "bad_room_id", kValidUid, 10);

    EXPECT_FALSE(msg.validate_value());
}

// ==================== SyncResponseMessage ====================

TEST(SyncResponseMessageTest, ValidPayloadSerializesSuccessfully)
{
    MessageBase::CommonField common(MessageBase::MessageType::SYNC_RESPONSE, "corr-1");
    SyncResponseMessage msg(common, kValidRoomIdHex, kValidUid, json::array());

    EXPECT_TRUE(msg.validate_payload());
    const json payload = msg.serialize_payload();
    EXPECT_FALSE(payload.is_null());
}

TEST(SyncResponseMessageTest, EmptyUidFailsValidation)
{
    MessageBase::CommonField common(MessageBase::MessageType::SYNC_RESPONSE, "corr-1");
    SyncResponseMessage msg(common, kValidRoomIdHex, "", json::array());

    EXPECT_FALSE(msg.validate_payload());
}

TEST(SyncResponseMessageTest, NonArrayGapMessagesFailsValidation)
{
    MessageBase::CommonField common(MessageBase::MessageType::SYNC_RESPONSE, "corr-1");
    SyncResponseMessage msg(common, kValidRoomIdHex, kValidUid, json::object());

    EXPECT_FALSE(msg.validate_payload());
}

// gap_messagesがjsonメンバへの波括弧初期化(_gap_messages{gap_messages})により
// 余分な配列でラップされてしまうバグ(2026-10頃に発見・修正)の再発防止テスト。
// nlohmann::jsonは単一のjson引数を波括弧で渡すとinitializer_listコンストラクタが
// 優先され、[元の値]のように一段余分にラップされてしまう。
TEST(SyncResponseMessageTest, GapMessagesAreNotDoubleWrapped)
{
    MessageBase::CommonField common(MessageBase::MessageType::SYNC_RESPONSE, "corr-1");
    const json gap_messages = json::array({ json{{"message_id", 1}}, json{{"message_id", 2}} });
    SyncResponseMessage msg(common, kValidRoomIdHex, kValidUid, gap_messages);

    const json payload = msg.serialize_payload();

    ASSERT_TRUE(payload.contains("payload"));
    ASSERT_TRUE(payload["payload"].contains("gap_messages"));
    const json& result = payload["payload"]["gap_messages"];
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0]["message_id"].get<int>(), 1);
    EXPECT_EQ(result[1]["message_id"].get<int>(), 2);
}

// ==================== SessionResponseMessage ====================
// unsend_messagesが同様の波括弧初期化バグで余分にラップされていないことの再発防止テスト

TEST(SessionResponseMessageTest, UnsendMessagesAreNotDoubleWrapped)
{
    MessageBase::CommonField common(MessageBase::MessageType::SESSION_RESPONSE, "corr-1");
    const json unsend_messages = json::array({ json{{"message_id", 1}} });
    SessionResponseMessage msg(common, kValidUid, true, "ok", unsend_messages);

    const json payload = msg.serialize_payload();

    ASSERT_TRUE(payload.contains("payload"));
    ASSERT_TRUE(payload["payload"].contains("unsend_messages"));
    const json& result = payload["payload"]["unsend_messages"];
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]["message_id"].get<int>(), 1);
}

// ==================== UserSearchResponseMessage ====================
// search_resultsが同様の波括弧初期化バグで余分にラップされていないことの再発防止テスト

TEST(UserSearchResponseMessageTest, SearchResultsAreNotDoubleWrapped)
{
    MessageBase::CommonField common(MessageBase::MessageType::USER_SEARCH_RESPONSE, "corr-1");
    const json search_results = json::array({ json{{"uid", kValidUid}} });
    UserSearchResponseMessage msg(common, search_results, "");

    const json payload = msg.serialize_payload();

    ASSERT_TRUE(payload.contains("payload"));
    ASSERT_TRUE(payload["payload"].contains("search_results"));
    const json& result = payload["payload"]["search_results"];
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]["uid"].get<std::string>(), kValidUid);
}
