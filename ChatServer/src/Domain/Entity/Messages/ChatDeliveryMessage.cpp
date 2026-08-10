#include "Domain/Entity/Messages/ChatDeliveryMessage.hpp"

ChatDeliveryMessage::ChatDeliveryMessage() 
    : MessageBase(MessageBase::MessageType::CHAT_DELIVERY),
      _send_message_id(0),
      _room_id(""),
      _sender_uid(""),
      _content(""),
      _room_sequence(0)
{

} 
ChatDeliveryMessage::ChatDeliveryMessage(
	const MessageBase::CommonField& common_fields,
	const uint64_t send_message_id,
    const std::string &room_id,
    const std::string &sender_uid,
    const std::string &content,
    const std::uint64_t room_sequence)
    : MessageBase(common_fields, MessageBase::MessageType::CHAT_DELIVERY),
      _send_message_id(send_message_id),
      _room_id(room_id),
      _sender_uid(sender_uid),
      _content(content),
      _room_sequence(room_sequence)
{
	if (room_id.empty() || sender_uid.empty() || content.empty()) {
		throw std::invalid_argument("Invalid argument: room_id, sender_uid, and content cannot be empty");
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

std::string ChatDeliveryMessage::get_sender_uid() const
{
	return this->_sender_uid;
}


std::string ChatDeliveryMessage::get_content() const
{
	return this->_content;
}

std::uint64_t ChatDeliveryMessage::get_room_sequence() const
{
	return this->_room_sequence;
}

void ChatDeliveryMessage::set_room_id(const std::string& room_id)
{
	this->_room_id = room_id;
}

void ChatDeliveryMessage::set_sender_uid(const std::string& sender_uid)
{
	this->_sender_uid = sender_uid;
}

void ChatDeliveryMessage::set_content(const std::string& content)
{
	this->_content = content;
}

void ChatDeliveryMessage::set_room_sequence(const std::uint64_t room_sequence)
{
	this->_room_sequence = room_sequence;
}

json ChatDeliveryMessage::serialize_payload() const
{
	json payload, specific_fields;
	try
	{
		if (validate_payload() == true)
		{
			specific_fields["room_id"] = this->_room_id;
			specific_fields["sender_uid"] = this->_sender_uid;
			specific_fields["content"] = this->_content;
			specific_fields["room_sequence"] = this->_room_sequence;
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
		if (this->_sender_uid.empty()) {
			throw std::runtime_error("Invalid ChatDeliveryMessage: sender_uid is empty.");
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

