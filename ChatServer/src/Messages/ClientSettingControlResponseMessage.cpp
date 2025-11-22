#include "Messages/ClientSettingControlResponseMessage.hpp"

ClientSettingControlResponseMessage::ClientSettingControlResponseMessage()
{
}

ClientSettingControlResponseMessage::~ClientSettingControlResponseMessage()
{
}

json ClientSettingControlResponseMessage::serialize_specific_message_fields() const
{
    return json();
}

bool ClientSettingControlResponseMessage::validate_specific_message_fields() const
{
    return false;
}
