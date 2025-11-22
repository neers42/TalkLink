#include "Messages/ChatDeliveryMessage.hpp"

ChatDeliveryMessage::ChatDeliveryMessage() 
{

} 
ChatDeliveryMessage::ChatDeliveryMessage(
    int c_type,
    const std::string &mt,
    const std::string &room_id,
    const std::string &sender_cid,
    const uint64_t send_message_id,
    const std::string &send_time,
    const std::string &content)
    : MessageBase(static_cast<ContentType>(c_type), mt, send_message_id, send_time),
      _room_id(room_id),
      _sender_cid(sender_cid),
      _content(content)
{
	if (room_id.empty() || sender_cid.empty() || content.empty()) {
		throw std::invalid_argument("Invalid argument: room_id, sender_cid, and content cannot be empty");
	}
}

ChatDeliveryMessage::ChatDeliveryMessage(
	MessageBase::ContentType c_type, 
	const std::string &mt, 
	const std::string &room_id, 
	const std::string &sender_id, 
	const uint64_t send_message_id, 
	const std::string &send_time, 
	const std::string &content)
	: MessageBase(c_type, mt, send_message_id, send_time),
	  _room_id(room_id),
	  _sender_cid(sender_id),
	  _content(content)
{
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

json ChatDeliveryMessage::serialize_specific_message_fields() const
{
	json specific_fields;
	try
	{
		if (validate_specific_message_fields() == false)
		{
			specific_fields["room_id"] = this->_room_id;
			specific_fields["sender_cid"] = this->_sender_cid;
			specific_fields["content"] = this->_content;
			return specific_fields;
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

bool ChatDeliveryMessage::validate_specific_message_fields() const
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

