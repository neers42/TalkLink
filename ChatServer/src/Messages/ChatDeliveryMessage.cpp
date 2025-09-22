#include "Messages/ChatDeliveryMessage.hpp"


ChatDeliveryMessage::ChatDeliveryMessage(
	int c_type,
	const std::string& mt,
	const std::string& room_id,
	const std::string& sender_cid,
	const uint64_t send_message_id,
	const std::string& send_time,
	const std::string& content)
	:BaseMessage(exchange_content_type_from_int(c_type), mt),
	_room_id(room_id),
	_sender_cid(sender_cid),
	_send_message_id(send_message_id),
	_content(content),
	_send_time(send_time)
{
	if (room_id.empty() || sender_cid.empty() || content.empty() || send_time.empty()) {
		throw std::invalid_argument("Invalid argument: room_id, sender_cid, content, and send_time cannot be empty");
	}
	

}

ChatDeliveryMessage::~ChatDeliveryMessage()
{
}

std::string ChatDeliveryMessage::get_room_id() const
{
	return this->_room_id;
}

std::string ChatDeliveryMessage::get_sender_cid() const
{
	return this->_sender_cid;
}

uint64_t ChatDeliveryMessage::get_send_message_id() const
{
	return this->_send_message_id;
}

std::string ChatDeliveryMessage::get_send_time() const
{
	return this->_send_time;
}

std::string ChatDeliveryMessage::get_content() const
{
	return this->_content;
}

void ChatDeliveryMessage::set_room_id(const std::string& room_id)
{
	this->_room_id = room_id;
}

void ChatDeliveryMessage::set_sender_cid(const std::string& sender_cid)
{
	this->_sender_cid = sender_cid;
}

void ChatDeliveryMessage::set_send_message_id(const uint64_t send_message_id)
{
	this->_send_message_id = send_message_id;
}

void ChatDeliveryMessage::set_send_time(const std::string& send_time)
{
	this->_send_time = send_time;
}

void ChatDeliveryMessage::content(const std::string& content)
{
	this->_content = content;
}

json ChatDeliveryMessage::deserialize_message(const std::unique_ptr<ChatDeliveryMessage>& chat_delivery_message)
{
	json chat_delivery_json;
	try
	{
		if (chat_delivery_message != nullptr) {
			chat_delivery_json["content_type"] = "TEXT";
			chat_delivery_json["message_type"] = chat_delivery_message->_message_type;
			chat_delivery_json["message_id"] = chat_delivery_message->_send_message_id;
			chat_delivery_json["room_id"] = chat_delivery_message->_room_id;
			chat_delivery_json["sender_cid"] = chat_delivery_message->_sender_cid;
			chat_delivery_json["content"] = chat_delivery_message->_content;
			chat_delivery_json["send_time"] = chat_delivery_message->_send_time;
			
			return chat_delivery_json;
		}
		else {
			throw std::runtime_error("ChatDeliveryMessage is null");
		}
		
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error deserializing ChatDeliveryMessage: {}", e.what());
		return nullptr;
	}
}
