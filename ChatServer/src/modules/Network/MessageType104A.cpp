#include "MessageType104A.h"


MessageType104A::MessageType104A(
	int c_type,
	const std::string& mt,
	const std::string& room_id,
	const std::string& sender_id,
	const uint64_t send_message_id,
	const std::string& send_time,
	const std::string& content)
	:BaseMessage(exchange_content_type_from_int(c_type), mt),
	_room_id(room_id),
	_sender_id(sender_id),
	_send_message_id(send_message_id),
	_content(content),
	_send_time(send_time)
{
	if (room_id.empty() || sender_id.empty() || content.empty() || send_time.empty()) {
		throw std::invalid_argument("Invalid argument: room_id, sender_id, content, and send_time cannot be empty");
	}
	

}

MessageType104A::~MessageType104A()
{
}

std::string MessageType104A::get_room_id() const
{
	return this->_room_id;
}

std::string MessageType104A::get_sender_id() const
{
	return this->_sender_id;
}

uint64_t MessageType104A::get_send_message_id() const
{
	return this->_send_message_id;
}

std::string MessageType104A::get_send_time() const
{
	return this->_send_time;
}

std::string MessageType104A::get_content() const
{
	return this->_content;
}

void MessageType104A::set_room_id(const std::string& room_id)
{
	this->_room_id = room_id;
}

void MessageType104A::set_sender_id(const std::string& sender_id)
{
	this->_sender_id = sender_id;
}

void MessageType104A::set_send_message_id(const uint64_t send_message_id)
{
	this->_send_message_id = send_message_id;
}

void MessageType104A::set_send_time(const std::string& send_time)
{
	this->_send_time = send_time;
}

void MessageType104A::content(const std::string& content)
{
	this->_content = content;
}

json MessageType104A::desirialize_message(const std::unique_ptr<MessageType104A>& mt104A)
{
	json mt104A_json;
	try
	{
		if (mt104A != nullptr) {
			mt104A_json["content_type"] = "TEXT";
			mt104A_json["message_type"] = mt104A->_message_type;
			mt104A_json["message_id"] = mt104A->_send_message_id;
			mt104A_json["room_id"] = mt104A->_room_id;
			mt104A_json["sender_id"] = mt104A->_sender_id;
			mt104A_json["content"] = mt104A->_content;
			mt104A_json["send_time"] = mt104A->_send_time;
			
			return mt104A_json;
		}
		else {
			throw std::runtime_error("MT104A is null");
		}
		
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error deserializing MessageType104A: {}", e.what());
		return nullptr;
	}
}
