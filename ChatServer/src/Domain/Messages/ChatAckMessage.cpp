#include "Domain/Messages/ChatAckMessage.hpp"

ChatAckMessage::ChatAckMessage() 
	: MessageBase(MessageBase::MessageType::CHAT_ACK), _message_id(0), _send_status(MessageSendStatus::SEND_FAILED), _room_id(""), _unsend_uid_list(), _room_sequence(0)
{
}

ChatAckMessage::ChatAckMessage(
	const MessageBase::CommonField& common_fields,
	const uint64_t mid,
	const MessageSendStatus status, 
	const std::string &room_id, 
	const std::vector<std::string> &_unsend_uid_list,
	const std::uint64_t room_sequence)
	: MessageBase(common_fields, MessageBase::MessageType::CHAT_ACK), _message_id(mid), _send_status(status), _room_id(room_id), _unsend_uid_list(_unsend_uid_list), _room_sequence(room_sequence)
{
}

ChatAckMessage::~ChatAckMessage()
{
}

int ChatAckMessage::get_send_status() const
{
	switch (this->_send_status)
	{
	case ChatAckMessage::MessageSendStatus::SEND_SUCCESS:
		return 0;
		break;
	case ChatAckMessage::MessageSendStatus::SEND_PARTIAL_SUCCESS:
		return 1;
		break;
	case ChatAckMessage::MessageSendStatus::SEND_FAILED:
		return 2;
		break;
	default:
		return 2;
		break;
	}
}

void ChatAckMessage::set_send_status(const int status)
{
	switch (status)
	{
	case 0:
		this->_send_status = ChatAckMessage::MessageSendStatus::SEND_SUCCESS;
		break;
	case 1:
		this->_send_status = ChatAckMessage::MessageSendStatus::SEND_PARTIAL_SUCCESS;
		break;
	case 2:
		this->_send_status = ChatAckMessage::MessageSendStatus::SEND_FAILED;
		break;
	default:
		this->_send_status = ChatAckMessage::MessageSendStatus::SEND_FAILED;
		break;
	}
}


std::string ChatAckMessage::get_room_id() const
{
	return this->_room_id;
}

void ChatAckMessage::set_room_id(const std::string& room_id)
{
	this->_room_id = room_id;
}

std::uint64_t ChatAckMessage::get_room_sequence() const
{
	return this->_room_sequence;
}

void ChatAckMessage::set_room_sequence(const std::uint64_t room_sequence)
{
	this->_room_sequence = room_sequence;
}

json ChatAckMessage::serialize_payload() const
{
	json payload, specific_fields, unsend_uids = json::array();
	try
	{
		if (validate_payload() != false) {
			specific_fields["message_id"] = this->get_message_id();
			specific_fields["send_message_id"] = this->get_message_id();
			specific_fields["send_status"] = this->get_send_status();
			specific_fields["room_id"] = this->get_room_id();
			specific_fields["room_sequence"] = this->get_room_sequence();
			if (this->_unsend_uid_list.size() > 0) {
				specific_fields["unsend_uid_list"] = this->_unsend_uid_list;
			}
			else {
				specific_fields["unsend_uid_list"] = json::array();
			}
			payload["payload"] = specific_fields;
			return payload;
		}
		else {
			throw std::runtime_error("ChatAckMessage specific fields validation failed during serialization.");
		}
		
	}catch (const std::runtime_error& re)
	{
		LOG_ERROR("Runtime error serializing ChatAckMessage: {}.", re.what());
		return json();
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error deserializing ChatAckMessage: {}.", e.what());
		return json();
	}

	return json();
}

bool ChatAckMessage::validate_payload() const
{
	try
	{
		if(this->_room_id.empty()){
			throw std::runtime_error("Invalid ChatAckMessage: room_id is empty.");
		}
		return true;
	}
	catch(const std::exception& e)
	{
		LOG_ERROR("Error validating ChatAckMessage specific fields: {}.", e.what());
		return false;
	}
}
