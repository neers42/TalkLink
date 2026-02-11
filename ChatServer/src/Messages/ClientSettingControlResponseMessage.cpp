#include "Messages/ClientSettingControlResponseMessage.hpp"

ClientSettingControlResponseMessage::ClientSettingControlResponseMessage()
{
}

ClientSettingControlResponseMessage::~ClientSettingControlResponseMessage()
{
}

json ClientSettingControlResponseMessage::serialize_payload() const
{
    return json();
}

bool ClientSettingControlResponseMessage::validate_payload() const
{
    return false;
}
