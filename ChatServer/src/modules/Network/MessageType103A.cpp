#include "MessageType103A.h"


MessageType103A::MessageType103A(int c_type,
	const std::string& mt,
	const uint64_t message_id,
	const std::string& room_id,
	const std::string& sender_id,
	const std::string& send_time,
	const std::string& content)
	:BaseMessage(exchange_content_type_from_int(c_type), mt, message_id),
	_room_id(room_id),
	_sender_id(sender_id),
	_content(content),
	_send_time(send_time)
{
}

MessageType103A::~MessageType103A()
{
}

json MessageType103A::desirialize_message(const std::unique_ptr<MessageType103A>& mt103A)
{
	json mt103A_json;
	try
	{
		if (mt103A != nullptr) {
			mt103A_json["content_type"] = mt103A->_content_type;
			mt103A_json["message_type"] = mt103A->_message_type;
			mt103A_json["message_id"] = mt103A->get_message_id();
			mt103A_json["room_id"] = mt103A->_room_id;
			mt103A_json["sender_id"] = mt103A->_sender_id;
			mt103A_json["content"] = mt103A->_content;
			mt103A_json["send_time"] = mt103A->_send_time;
			
			return mt103A_json;
		}
		else {
			throw std::runtime_error("MT103A is null");
		}
		
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error deserializing MessageType103A: {}", e.what());
		return nullptr;
	}
}
