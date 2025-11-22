#include "Messages/ChatAckMessage.hpp"

ChatAckMessage::ChatAckMessage() 
{

}
ChatAckMessage::ChatAckMessage(
	MessageBase::ContentType c_type, 
	const std::string &mt, 
	const uint64_t _send_mid, 
	const MessageSendStatus status, 
	const std::string &room_id, 
	const std::vector<std::string> &_unsend_cid_list)
	: MessageBase(c_type, mt, _send_mid),
	 _send_status(status), _room_id(room_id), _unsend_cid_list(_unsend_cid_list)
{
}
ChatAckMessage::ChatAckMessage(int c_type,
	const std::string &mt, 
	const uint64_t _send_mid, 
	const MessageSendStatus status,
	const std::string &room_id, 
	const std::vector<std::string> &_unsend_cids)
    : MessageBase(static_cast<MessageBase::ContentType>(c_type), mt, _send_mid),
	 _send_status(status), _room_id(room_id), _unsend_cid_list(_unsend_cids)
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



json ChatAckMessage::serialize_specific_message_fields() const
{
	json chat_ack_message, unsend_cids = json::array();
	try
	{
		if (validate_specific_message_fields() != false) {

			switch (this->_send_status)
			{
			case ChatAckMessage::MessageSendStatus::SEND_SUCCESS:
				chat_ack_message["send_status"] = "SEND_SUCCESS";
				break;
			case ChatAckMessage::MessageSendStatus::SEND_PARTIAL_SUCCESS:
				chat_ack_message["send_status"] = "SEND_PARTIAL_SUCCESS";
				break;
			case ChatAckMessage::MessageSendStatus::SEND_FAILED:
				chat_ack_message["send_status"] = "SEND_FAILED";
				break;
			default:
				chat_ack_message["send_status"] = "SEND_FAILED";
				break;
			}
			chat_ack_message["room_id"] = this->get_room_id();
			if (this->_unsend_cid_list.size() > 0) {
				chat_ack_message["unsend_cid_list"] = this->_unsend_cid_list;
			}
			else {
				chat_ack_message["unsend_cid_list"] = json::array();
			}
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

	return chat_ack_message;
}

bool ChatAckMessage::validate_specific_message_fields() const
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
