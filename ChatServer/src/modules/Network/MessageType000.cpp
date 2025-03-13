#include "MessageType000.h"

MessageType000::MessageType000()
{
}

MessageType000::MessageType000(int content_type, std::string& mt, const std::string message_id, const std::string& client_id)
	:BaseMessage(exchange_content_type_from_int(content_type), mt, message_id), _client_id{ client_id}
{
}

MessageType000::~MessageType000()
{
}
