#include "Domain/Messages/UserSettingControlMessage.hpp"

UserSettingControlMessage::UserSettingControlMessage()
{
}

UserSettingControlMessage::~UserSettingControlMessage()
{
}

bool UserSettingControlMessage::validate_payload(const json &message) const
{
    return false;
}

std::unique_ptr<MessageBase> UserSettingControlMessage::create_instance(const json &message) const
{
    return std::unique_ptr<MessageBase>();
}
bool UserSettingControlMessage::validate_value() const
{
    return false;
}
