#include <gtest/gtest.h>

#include "Domain/Entity/Messages/AuthRequestMessage.hpp"
#include "Domain/Entity/Messages/AuthResponseMessage.hpp"

namespace {
// header.payload.signature形式かつ各セクションがURL-safe Base64文字のみで
// 構成された、50文字以上のダミーJWT
const std::string kValidJwt =
    "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9."
    "eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIn0."
    "dBjftJeZ4CVP-mB92K27uhbUJU1p1r_wW1gFWFOEjXk";
}

// ==================== AuthRequestMessage ====================

TEST(AuthRequestMessageTest, ValidJwtPassesValidation)
{
    MessageBase::CommonField common(MessageBase::MessageType::AUTHENTICATION_REQUEST, "corr-1");
    AuthRequestMessage msg(common, kValidJwt);

    EXPECT_TRUE(msg.validate_value());
    EXPECT_EQ(msg.get_jwt(), kValidJwt);
}

TEST(AuthRequestMessageTest, EmptyJwtFailsValidation)
{
    MessageBase::CommonField common(MessageBase::MessageType::AUTHENTICATION_REQUEST, "corr-1");
    AuthRequestMessage msg(common, "");

    EXPECT_FALSE(msg.validate_value());
}

TEST(AuthRequestMessageTest, TooShortJwtFailsValidation)
{
    MessageBase::CommonField common(MessageBase::MessageType::AUTHENTICATION_REQUEST, "corr-1");
    AuthRequestMessage msg(common, "a.b.c"); // 50文字未満

    EXPECT_FALSE(msg.validate_value());
}

TEST(AuthRequestMessageTest, MissingDotsFailsValidation)
{
    MessageBase::CommonField common(MessageBase::MessageType::AUTHENTICATION_REQUEST, "corr-1");
    // ドットが無い、十分な長さのダミー文字列
    const std::string no_dots(60, 'a');
    AuthRequestMessage msg(common, no_dots);

    EXPECT_FALSE(msg.validate_value());
}

TEST(AuthRequestMessageTest, InvalidCharactersFailValidation)
{
    MessageBase::CommonField common(MessageBase::MessageType::AUTHENTICATION_REQUEST, "corr-1");
    // Base64に使われない文字(スペースや!)を含む
    const std::string invalid_chars =
        "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9!."
        "eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIn0 ."
        "dBjftJeZ4CVP-mB92K27uhbUJU1p1r_wW1gFWFOEjXk";
    AuthRequestMessage msg(common, invalid_chars);

    EXPECT_FALSE(msg.validate_value());
}

TEST(AuthRequestMessageTest, ValidatePayloadRequiresJwtField)
{
    MessageBase::CommonField common(MessageBase::MessageType::AUTHENTICATION_REQUEST, "corr-1");
    AuthRequestMessage msg(common, kValidJwt);

    json message_without_jwt;
    message_without_jwt["payload"] = json::object();
    EXPECT_FALSE(msg.validate_payload(message_without_jwt));

    json message_with_jwt;
    message_with_jwt["payload"]["jwt"] = kValidJwt;
    EXPECT_TRUE(msg.validate_payload(message_with_jwt));
}

// ==================== AuthResponseMessage ====================

TEST(AuthResponseMessageTest, SerializedPayloadContainsExpectedFields)
{
    MessageBase::CommonField common(MessageBase::MessageType::AUTHENTICATION_RESPONSE, "corr-1");
    AuthResponseMessage msg(common, true, 3600, "session-token-hash");

    EXPECT_TRUE(msg.get_verify_status());
    EXPECT_EQ(msg.get_ttl(), 3600u);

    const json payload = msg.serialize_payload();
    ASSERT_TRUE(payload.contains("payload"));
    EXPECT_EQ(payload["payload"]["verify_result"].get<bool>(), true);
    EXPECT_EQ(payload["payload"]["ttl"].get<uint64_t>(), 3600u);
    EXPECT_EQ(payload["payload"]["session_token"].get<std::string>(), "session-token-hash");
}

TEST(AuthResponseMessageTest, FailedVerificationSerializesCorrectly)
{
    MessageBase::CommonField common(MessageBase::MessageType::AUTHENTICATION_RESPONSE, "corr-1");
    AuthResponseMessage msg(common, false, 0, "");

    EXPECT_FALSE(msg.get_verify_status());
    const json payload = msg.serialize_payload();
    ASSERT_TRUE(payload.contains("payload"));
    EXPECT_EQ(payload["payload"]["verify_result"].get<bool>(), false);
}
