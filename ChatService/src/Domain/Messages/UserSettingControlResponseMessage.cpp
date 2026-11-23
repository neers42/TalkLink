#include "Domain/Messages/UserSettingControlResponseMessage.hpp"

UserSettingControlResponseMessage::UserSettingControlResponseMessage()
{
}

UserSettingControlResponseMessage::~UserSettingControlResponseMessage()
{
}

json UserSettingControlResponseMessage::serialize_payload() const
{
    return json();
}

bool UserSettingControlResponseMessage::validate_payload() const
{
    return false;
}
