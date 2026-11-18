#include <gtest/gtest.h>

#include "Domain/Entity/Messages/ChatAckMessage.hpp"
#include "Domain/Entity/Messages/ChatroomControlAckMessage.hpp"
#include "Domain/Entity/Messages/ChatroomControlDeliveryMessage.hpp"
#include "Domain/Entity/Messages/ChatroomControlMessage.hpp"
#include "Domain/Entity/Messages/HeartBeatMessage.hpp"
#include "Domain/Entity/Messages/SessionRequestMessage.hpp"
#include "Domain/Entity/Messages/UserSearchRequestMessage.hpp"
#include "ServerTime.hpp"

namespace {
const std::string kValidUid = std::string(28, 'a'); // 28byte
const std::string kValidRoomIdHex = std::string(32, 'a'); // 32byte(HEX)
const std::string kValidRoomIdUuid = std::string(36, 'b'); // 36byte(UUID)

// MessageBase::parse_common_fieldsが要求するトップレベルの共通フィールドを
// 手動で組み立てる(serialize_common_message_fieldsはprivateのため使えない)。
json build_common_fields_json(const std::string& message_type, const std::string& correlation_id) {
    json j;
    j["message_type"] = message_type;
    j["correlation_id"] = correlation_id;
    j["timestamp"] = ServerTime::get_current_time_to_milliseconds();
    return j;
}
}

// ==================== ChatAckMessage (ISendMessage) ====================

TEST(ChatAckMessageTest, ValidateRoomIdPassesWhenNotEmpty)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHAT_ACK, "corr-1");
    ChatAckMessage msg(common, 1, ChatAckMessage::MessageSendStatus::SEND_SUCCESS, kValidRoomIdHex, {}, 0);

    EXPECT_TRUE(msg.validate_payload());
}

TEST(ChatAckMessageTest, ValidatePayloadFailsWhenRoomIdEmpty)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHAT_ACK, "corr-1");
    ChatAckMessage msg(common, 1, ChatAckMessage::MessageSendStatus::SEND_SUCCESS, "", {}, 0);

    EXPECT_FALSE(msg.validate_payload());
}

TEST(ChatAckMessageTest, SerializePayloadContainsExpectedFields)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHAT_ACK, "corr-1");
    ChatAckMessage msg(common, 42, ChatAckMessage::MessageSendStatus::SEND_PARTIAL_SUCCESS, kValidRoomIdHex, {"uid-x"}, 3);

    json payload = msg.serialize_payload();
    ASSERT_TRUE(payload.contains("payload"));
    EXPECT_EQ(payload["payload"]["message_id"].get<uint64_t>(), 42u);
    EXPECT_EQ(payload["payload"]["send_status"].get<int>(), 1);
    EXPECT_EQ(payload["payload"]["room_sequence"].get<uint64_t>(), 3u);
    ASSERT_EQ(payload["payload"]["unsend_uid_list"].size(), 1u);
}

// ==================== ChatroomControlAckMessage (ISendMessage) ====================

TEST(ChatroomControlAckMessageTest, ValidatePayloadPassesForHexOrUuidRoomId)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHATROOM_CONTROL_ACK, "corr-1");
    ChatroomControlAckMessage msg_hex(common, ChatroomControlAckMessage::MessageSendStatus::SEND_SUCCESS, kValidRoomIdHex, 1, {}, 0);
    ChatroomControlAckMessage msg_uuid(common, ChatroomControlAckMessage::MessageSendStatus::SEND_SUCCESS, kValidRoomIdUuid, 1, {}, 0);

    EXPECT_TRUE(msg_hex.validate_payload());
    EXPECT_TRUE(msg_uuid.validate_payload());
}

TEST(ChatroomControlAckMessageTest, ValidatePayloadFailsForInvalidRoomIdLength)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHATROOM_CONTROL_ACK, "corr-1");
    ChatroomControlAckMessage msg(common, ChatroomControlAckMessage::MessageSendStatus::SEND_SUCCESS, "short", 1, {}, 0);

    EXPECT_FALSE(msg.validate_payload());
}

TEST(ChatroomControlAckMessageTest, SerializePayloadFailsSafelyWhenInvalid)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHATROOM_CONTROL_ACK, "corr-1");
    ChatroomControlAckMessage msg(common, ChatroomControlAckMessage::MessageSendStatus::SEND_SUCCESS, "short", 1, {}, 0);

    EXPECT_TRUE(msg.serialize_payload().is_null());
}

// ==================== ChatroomControlDeliveryMessage (ISendMessage) ====================

TEST(ChatroomControlDeliveryMessageTest, ValidatePayloadPassesForValidFields)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHATROOM_CONTROL_DELIVERY, "corr-1");
    ChatroomControlDeliveryMessage msg(common, 1, kValidUid, kValidRoomIdHex, "room-name", "icon.png", {}, ChatroomControlMessage::RoomAction::CREATE_ROOM, 0);

    EXPECT_TRUE(msg.validate_payload());
}

TEST(ChatroomControlDeliveryMessageTest, ValidatePayloadFailsForEmptyRoomName)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHATROOM_CONTROL_DELIVERY, "corr-1");
    ChatroomControlDeliveryMessage msg(common, 1, kValidUid, kValidRoomIdHex, "", "icon.png", {}, ChatroomControlMessage::RoomAction::CREATE_ROOM, 0);

    EXPECT_FALSE(msg.validate_payload());
}

TEST(ChatroomControlDeliveryMessageTest, ValidatePayloadFailsForOversizedIconUrl)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHATROOM_CONTROL_DELIVERY, "corr-1");
    ChatroomControlDeliveryMessage msg(common, 1, kValidUid, kValidRoomIdHex, "room-name", std::string(201, 'u'), {}, ChatroomControlMessage::RoomAction::CREATE_ROOM, 0);

    EXPECT_FALSE(msg.validate_payload());
}

TEST(ChatroomControlDeliveryMessageTest, SerializePayloadIncludesMembersForCreateRoom)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHATROOM_CONTROL_DELIVERY, "corr-1");
    std::vector<UserInfo> members{UserInfo(kValidUid, "Alice", "a@example.com", "icon.png")};
    ChatroomControlDeliveryMessage msg(common, 1, kValidUid, kValidRoomIdHex, "room-name", "icon.png", members, ChatroomControlMessage::RoomAction::CREATE_ROOM, 5);

    json payload = msg.serialize_payload();
    ASSERT_TRUE(payload.contains("payload"));
    EXPECT_EQ(payload["payload"]["room_action"].get<std::string>(), "CREATE_ROOM");
    EXPECT_EQ(payload["payload"]["room_sequence"].get<uint64_t>(), 5u);
    ASSERT_EQ(payload["payload"]["members"].size(), 1u);
}

TEST(ChatroomControlDeliveryMessageTest, SerializePayloadFailsForUnsupportedRoomAction)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHATROOM_CONTROL_DELIVERY, "corr-1");
    // LEAVE_ROOMはserialize_payloadのswitchで未対応(default節でエラー)
    ChatroomControlDeliveryMessage msg(common, 1, kValidUid, kValidRoomIdHex, "room-name", "icon.png", {}, ChatroomControlMessage::RoomAction::LEAVE_ROOM, 0);

    EXPECT_TRUE(msg.serialize_payload().is_null());
}

// ==================== ChatroomControlMessage (IReceiveMessage) ====================

namespace {
json build_chatroom_control_payload_json(const std::string& room_id, const std::string& room_action_str) {
    json j = build_common_fields_json("CHATROOM_CONTROL", "corr-1");
    json payload;
    payload["sender_uid"] = kValidUid;
    payload["room_id"] = room_id;
    payload["room_member_list"] = json::array({kValidUid});
    payload["room_name"] = "room-name";
    payload["icon_url"] = "icon.png";
    payload["room_action"] = room_action_str;
    j["payload"] = payload;
    return j;
}
}

TEST(ChatroomControlMessageTest, ValidatePayloadPassesWithAllRequiredKeys)
{
    ChatroomControlMessage msg;
    json j = build_chatroom_control_payload_json(kValidRoomIdHex, "CREATE_ROOM");

    EXPECT_TRUE(msg.validate_payload(j));
}

TEST(ChatroomControlMessageTest, ValidatePayloadFailsWhenMissingPayloadKey)
{
    ChatroomControlMessage msg;
    json j = build_common_fields_json("CHATROOM_CONTROL", "corr-1");

    EXPECT_FALSE(msg.validate_payload(j));
}

TEST(ChatroomControlMessageTest, ValidatePayloadFailsWhenRequiredKeyMissing)
{
    ChatroomControlMessage msg;
    json j = build_chatroom_control_payload_json(kValidRoomIdHex, "CREATE_ROOM");
    j["payload"].erase("room_name");

    EXPECT_FALSE(msg.validate_payload(j));
}

TEST(ChatroomControlMessageTest, CreateInstanceBuildsMessageFromValidJson)
{
    ChatroomControlMessage msg;
    json j = build_chatroom_control_payload_json(kValidRoomIdHex, "ADD_MEMBER");

    auto instance = msg.create_instance(j);
    ASSERT_NE(instance, nullptr);

    auto* control_msg = dynamic_cast<ChatroomControlMessage*>(instance.get());
    ASSERT_NE(control_msg, nullptr);
    EXPECT_EQ(control_msg->get_room_id(), kValidRoomIdHex);
    EXPECT_EQ(control_msg->get_room_action(), ChatroomControlMessage::RoomAction::ADD_MEMBER);
}

TEST(ChatroomControlMessageTest, CreateInstanceReturnsNullForUnknownRoomAction)
{
    ChatroomControlMessage msg;
    json j = build_chatroom_control_payload_json(kValidRoomIdHex, "NOT_A_REAL_ACTION");

    EXPECT_EQ(msg.create_instance(j), nullptr);
}

TEST(ChatroomControlMessageTest, ValidateValuePassesForEmptyRoomIdOnCreateRoom)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHATROOM_CONTROL, "corr-1");
    ChatroomControlMessage msg(common, 1, kValidUid, "", {kValidUid}, "room-name", "icon.png", ChatroomControlMessage::RoomAction::CREATE_ROOM, "{}");

    EXPECT_TRUE(msg.validate_value());
}

TEST(ChatroomControlMessageTest, ValidateValueFailsForEmptyRoomIdOnNonCreateAction)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHATROOM_CONTROL, "corr-1");
    ChatroomControlMessage msg(common, 1, kValidUid, "", {kValidUid}, "room-name", "icon.png", ChatroomControlMessage::RoomAction::LEAVE_ROOM, "{}");

    EXPECT_FALSE(msg.validate_value());
}

TEST(ChatroomControlMessageTest, ValidateValueFailsForOversizedRoomName)
{
    MessageBase::CommonField common(MessageBase::MessageType::CHATROOM_CONTROL, "corr-1");
    ChatroomControlMessage msg(common, 1, kValidUid, kValidRoomIdHex, {kValidUid}, std::string(101, 'r'), "icon.png", ChatroomControlMessage::RoomAction::ADD_MEMBER, "{}");

    EXPECT_FALSE(msg.validate_value());
}

// ==================== HeartBeatMessage (IReceiveMessage) ====================

TEST(HeartBeatMessageTest, ValidateValuePassesFor28ByteUid)
{
    MessageBase::CommonField common(MessageBase::MessageType::HEARTBEAT_MESSAGE, "corr-1");
    HeartBeatMessage msg(common, kValidUid);

    EXPECT_TRUE(msg.validate_value());
}

TEST(HeartBeatMessageTest, ValidateValueFailsForWrongUidLength)
{
    MessageBase::CommonField common(MessageBase::MessageType::HEARTBEAT_MESSAGE, "corr-1");
    HeartBeatMessage msg(common, "short-uid");

    EXPECT_FALSE(msg.validate_value());
}

TEST(HeartBeatMessageTest, ValidatePayloadPassesForValidPayload)
{
    HeartBeatMessage msg;
    json j = build_common_fields_json("HEARTBEAT_MESSAGE", "corr-1");
    j["payload"]["uid"] = kValidUid;

    EXPECT_TRUE(msg.validate_payload(j));
}

TEST(HeartBeatMessageTest, ValidatePayloadFailsForMissingUid)
{
    HeartBeatMessage msg;
    json j = build_common_fields_json("HEARTBEAT_MESSAGE", "corr-1");
    j["payload"] = json::object();

    EXPECT_FALSE(msg.validate_payload(j));
}

TEST(HeartBeatMessageTest, CreateInstanceBuildsMessageFromValidJson)
{
    HeartBeatMessage msg;
    json j = build_common_fields_json("HEARTBEAT_MESSAGE", "corr-1");
    j["payload"]["uid"] = kValidUid;

    auto instance = msg.create_instance(j);
    ASSERT_NE(instance, nullptr);
    auto* hb = dynamic_cast<HeartBeatMessage*>(instance.get());
    ASSERT_NE(hb, nullptr);
    EXPECT_EQ(hb->get_uid_by_socket(), kValidUid);
}

// ==================== SessionRequestMessage (IReceiveMessage) ====================

TEST(SessionRequestMessageTest, ValidateValueSigninSkipsNameAndMailValidation)
{
    MessageBase::CommonField common(MessageBase::MessageType::SESSION_REQUEST, "corr-1");
    SessionRequestMessage msg(common, SessionRequestMessage::SignType::SIGNIN, "token-abc", "", "", 0);

    EXPECT_TRUE(msg.validate_value());
}

TEST(SessionRequestMessageTest, ValidateValueSigninFailsForEmptyToken)
{
    MessageBase::CommonField common(MessageBase::MessageType::SESSION_REQUEST, "corr-1");
    SessionRequestMessage msg(common, SessionRequestMessage::SignType::SIGNIN, "", "", "", 0);

    EXPECT_FALSE(msg.validate_value());
}

TEST(SessionRequestMessageTest, ValidateValueSignoutSkipsTokenNameAndMailValidation)
{
    MessageBase::CommonField common(MessageBase::MessageType::SESSION_REQUEST, "corr-1");
    SessionRequestMessage msg(common, SessionRequestMessage::SignType::SIGNOUT, "", "", "", 0);

    EXPECT_TRUE(msg.validate_value());
}

TEST(SessionRequestMessageTest, ValidateValueSignupRequiresTokenNameAndMail)
{
    MessageBase::CommonField common(MessageBase::MessageType::SESSION_REQUEST, "corr-1");
    SessionRequestMessage valid(common, SessionRequestMessage::SignType::SIGNUP, "token-abc", "Alice", "alice@example.com", 0);
    SessionRequestMessage missing_name(common, SessionRequestMessage::SignType::SIGNUP, "token-abc", "", "alice@example.com", 0);
    SessionRequestMessage missing_mail(common, SessionRequestMessage::SignType::SIGNUP, "token-abc", "Alice", "", 0);

    EXPECT_TRUE(valid.validate_value());
    EXPECT_FALSE(missing_name.validate_value());
    EXPECT_FALSE(missing_mail.validate_value());
}

TEST(SessionRequestMessageTest, ValidatePayloadRequiresNameAndMailOnlyForSignUp)
{
    SessionRequestMessage msg;
    json signin_json = build_common_fields_json("SESSION_REQUEST", "corr-1");
    signin_json["payload"]["sign_type"] = "SIGN IN";
    signin_json["payload"]["session_token"] = "token-abc";
    // is_number_unsigned()でチェックされるため、符号なしリテラルを使う必要がある
    signin_json["payload"]["last_received_message_id"] = 0u;

    EXPECT_TRUE(msg.validate_payload(signin_json));

    json signup_json = signin_json;
    signup_json["payload"]["sign_type"] = "SIGN UP";
    // name/mail_addressを設定していないため、SIGN UPでは必須キー不足で失敗する
    EXPECT_FALSE(msg.validate_payload(signup_json));

    signup_json["payload"]["name"] = "Alice";
    signup_json["payload"]["mail_address"] = "alice@example.com";
    EXPECT_TRUE(msg.validate_payload(signup_json));
}

TEST(SessionRequestMessageTest, CreateInstanceBuildsSigninMessage)
{
    SessionRequestMessage msg;
    json j = build_common_fields_json("SESSION_REQUEST", "corr-1");
    j["payload"]["sign_type"] = "SIGN IN";
    j["payload"]["session_token"] = "token-abc";
    j["payload"]["last_received_message_id"] = 42;

    auto instance = msg.create_instance(j);
    ASSERT_NE(instance, nullptr);
    auto* session_msg = dynamic_cast<SessionRequestMessage*>(instance.get());
    ASSERT_NE(session_msg, nullptr);
    EXPECT_EQ(session_msg->get_sign_type(), SessionRequestMessage::SignType::SIGNIN);
    EXPECT_EQ(session_msg->get_session_token(), "token-abc");
    EXPECT_EQ(session_msg->get_last_received_message_id(), 42u);
}

TEST(SessionRequestMessageTest, CreateInstanceReturnsNullForUnknownSignType)
{
    SessionRequestMessage msg;
    json j = build_common_fields_json("SESSION_REQUEST", "corr-1");
    j["payload"]["sign_type"] = "NOT_A_SIGN_TYPE";
    j["payload"]["session_token"] = "token-abc";
    j["payload"]["last_received_message_id"] = 0;

    EXPECT_EQ(msg.create_instance(j), nullptr);
}

// ==================== UserSearchRequestMessage (IReceiveMessage) ====================

TEST(UserSearchRequestMessageTest, ValidateValuePassesForNonEmptyQuery)
{
    MessageBase::CommonField common(MessageBase::MessageType::USER_SEARCH_REQUEST, "corr-1");
    UserSearchRequestMessage msg(common, "query-text");

    EXPECT_TRUE(msg.validate_value());
}

TEST(UserSearchRequestMessageTest, ValidateValueFailsForEmptyQuery)
{
    MessageBase::CommonField common(MessageBase::MessageType::USER_SEARCH_REQUEST, "corr-1");
    UserSearchRequestMessage msg(common, "");

    EXPECT_FALSE(msg.validate_value());
}

TEST(UserSearchRequestMessageTest, ValidatePayloadFailsForEmptyQuery)
{
    UserSearchRequestMessage msg;
    json j = build_common_fields_json("USER_SEARCH_REQUEST", "corr-1");
    j["payload"]["query"] = "";

    EXPECT_FALSE(msg.validate_payload(j));
}

TEST(UserSearchRequestMessageTest, CreateInstanceBuildsMessageFromValidJson)
{
    UserSearchRequestMessage msg;
    json j = build_common_fields_json("USER_SEARCH_REQUEST", "corr-1");
    j["payload"]["query"] = "query-text";

    auto instance = msg.create_instance(j);
    ASSERT_NE(instance, nullptr);
    auto* search_msg = dynamic_cast<UserSearchRequestMessage*>(instance.get());
    ASSERT_NE(search_msg, nullptr);
    EXPECT_EQ(search_msg->get_query(), "query-text");
}
