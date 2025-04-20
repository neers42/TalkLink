#include "MessageType103B.h"


MessageType103B::MessageType103B(int c_type,
	const std::string& mt,
	const uint64_t message_id,
	const bool status,
	const std::string& room_id)
	:BaseMessage(exchange_content_type_from_int(c_type), mt, message_id),
	_send_status(status),
	_room_id(room_id)
{
}

MessageType103B::~MessageType103B()
{
}

int MessageType103B::get_status() const
{
	return this->_send_status;
}

void MessageType103B::set_status(const bool status)
{
	this->_send_status = status;
}


std::string MessageType103B::get_room_id() const
{
	// TODO: return ステートメントをここに挿入します
	return this->_room_id;
}

void MessageType103B::set_room_id(const std::string& room_id)
{
	this->_room_id = room_id;
}

json MessageType103B::desirialize_message(const std::unique_ptr<MessageType103B>& mt103B)
{
	json mt103B_json;
	try
	{
		
		if (mt103B != nullptr) {
			mt103B_json["content_type"] = mt103B->_content_type;
			mt103B_json["message_type"] = mt103B->_message_type;
			mt103B_json["message_id"] = mt103B->get_message_id();
			mt103B_json["status"] = mt103B->_send_status;
			mt103B_json["room_id"] = mt103B->_room_id;
		}
		else {
			throw std::runtime_error("MT103B is null");
		}
		
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error deserializing MessageType103B: {}.", e.what());
	}

	return mt103B_json;
}
