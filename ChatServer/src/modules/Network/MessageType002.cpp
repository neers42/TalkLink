#include "MessageType002.h"

MessageType002::MessageType002()
{
}

MessageType002::~MessageType002()
{
}

std::unique_ptr<MessageType002> MessageType002::serialize_message(const json& recv_json)
{
	return std::unique_ptr<MessageType002>();
}
