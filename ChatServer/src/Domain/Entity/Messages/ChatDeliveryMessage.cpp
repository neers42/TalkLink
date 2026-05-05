#include "Domain/Entity/Messages/ChatDeliveryMessage.hpp"

ChatDeliveryMessage::ChatDeliveryMessage() 
    : MessageBase(MessageBase::MessageType::CHAT_DELIVERY),
      _send_message_id(0),
      _room_id(""),
      _sender_cid(""),
      _content("")
{

} 
ChatDeliveryMessage::ChatDeliveryMessage(
	const MessageBase::CommonField& common_fields,
	const uint64_t send_message_id,
    const std::string &room_id,
    const std::string &sender_cid,
    const std::string &content)
    : MessageBase(common_fields, MessageBase::MessageType::CHAT_DELIVERY),
      _send_message_id(send_message_id),
      _room_id(room_id),
      _sender_cid(sender_cid),
      _content(content)
{
	if (room_id.empty() || sender_cid.empty() || content.empty()) {
		throw std::invalid_argument("Invalid argument: room_id, sender_cid, and content cannot be empty");
	}
}


ChatDeliveryMessage::~ChatDeliveryMessage()
{
}

uint64_t ChatDeliveryMessage::get_send_message_id() const
{
	return this->_send_message_id;
}

std::string ChatDeliveryMessage::get_room_id() const
{
	return this->_room_id;
}

std::string ChatDeliveryMessage::get_sender_cid() const
{
	return this->_sender_cid;
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

void ChatDeliveryMessage::set_content(const std::string& content)
{
	this->_content = content;
}

json ChatDeliveryMessage::serialize_payload() const
{
	json payload, specific_fields;
	try
	{
		if (validate_payload() == true)
		{
			specific_fields["room_id"] = this->_room_id;
			specific_fields["sender_cid"] = this->_sender_cid;
			specific_fields["content"] = this->_content;
			payload["payload"] = specific_fields;
			return payload;
		}
		else {
			throw std::runtime_error("ChatDeliveryMessage specific fields validation failed during serialization.");
		}
		
	}
	catch(const std::runtime_error& re)
	{
		LOG_ERROR("Runtime error serializing ChatDeliveryMessage: {}", re.what());
		return json();
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error serializing ChatDeliveryMessage: {}", e.what());
		return json();
	}
}

bool ChatDeliveryMessage::validate_payload() const
{
	try
	{
		if (this->_room_id.empty()) {
			throw std::runtime_error("Invalid ChatDeliveryMessage: room_id is empty.");
		}
		if (this->_sender_cid.empty()) {
			throw std::runtime_error("Invalid ChatDeliveryMessage: sender_cid is empty.");
		}
		if (this->_content.empty()) {
			throw std::runtime_error("Invalid ChatDeliveryMessage: content is empty.");
		}
		return true;
	}
	catch(const std::runtime_error& re)
	{
		LOG_ERROR("Runtime error validating ChatDeliveryMessage specific fields: {}", re.what());
		return false;
	}
	catch(const std::exception& e)
	{
		LOG_ERROR("Error validating ChatDeliveryMessage specific fields: {}", e.what());
		return false;
	}
}

