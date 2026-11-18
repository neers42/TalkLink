#include <gtest/gtest.h>

#include "Domain/Entity/Messages/IncrementalSearchMessage.hpp"
#include "Domain/Entity/Messages/UserSettingControlMessage.hpp"
#include "Domain/Entity/Messages/UserSettingControlResponseMessage.hpp"

// これら3クラスは現時点で未実装のスタブで、validate_payload/create_instance/
// validate_valueが常にfalse/nullptrを返す。機能追加はこのテストの対象外であり、
// 現在の(未実装である)振る舞いをそのまま記録して、将来の実装時に意図した
// 変更であることを意識させるためのテスト。

TEST(IncrementalSearchMessageStubTest, ValidatePayloadAlwaysReturnsFalse)
{
    MessageBase::CommonField common(MessageBase::MessageType::INCLIMENTAL_SEARCH, "corr-1");
    IncrementalSearchMessage msg(common, IncrementalSearchMessage::SearchType::BY_USER, "keyword");

    json j;
    j["payload"]["search_type"] = "BY_USER";
    j["payload"]["search_keyword"] = "keyword";

    EXPECT_FALSE(msg.validate_payload(j));
    EXPECT_FALSE(msg.validate_value());
    EXPECT_EQ(msg.create_instance(j), nullptr);
}

TEST(UserSettingControlMessageStubTest, AllValidationMethodsAlwaysReturnFalseOrNull)
{
    UserSettingControlMessage msg;

    EXPECT_FALSE(msg.validate_payload(json::object()));
    EXPECT_FALSE(msg.validate_value());
    EXPECT_EQ(msg.create_instance(json::object()), nullptr);
}

TEST(UserSettingControlResponseMessageStubTest, AllValidationMethodsAlwaysReturnFalseOrNull)
{
    UserSettingControlResponseMessage msg;

    EXPECT_FALSE(msg.validate_payload());
    EXPECT_TRUE(msg.serialize_payload().is_null());
}
