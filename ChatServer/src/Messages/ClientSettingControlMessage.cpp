#include "Messages/ClientSettingControlMessage.hpp"

ClientSettingControlMessage::ClientSettingControlMessage()
{
}

ClientSettingControlMessage::~ClientSettingControlMessage()
{
}

bool ClientSettingControlMessage::validate_payload(const json &message) const
{
    return false;
}

std::unique_ptr<MessageBase> ClientSettingControlMessage::create_instance(const json &message) const
{
    return std::unique_ptr<MessageBase>();
}
bool ClientSettingControlMessage::validate_value() const
{
    return false;
}
