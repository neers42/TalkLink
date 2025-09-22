#include "Messages/ChatAckMessage.hpp"


ChatAckMessage::ChatAckMessage(int c_type,
	const std::string& mt,
	const MessageSendStatus status,
	const std::string& room_id,
	const uint64_t _send_mid,
	const std::vector<std::string>& _unsend_cids)
	:BaseMessage(exchange_content_type_from_int(c_type), mt),
	_send_status(status),
	_room_id(room_id),
	_send_message_id(_send_mid),
	_unsend_cid_list(_unsend_cids)
{
}

ChatAckMessage::~ChatAckMessage()
{
}

int ChatAckMessage::get_status() const
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

void ChatAckMessage::set_status(const int status)
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
	// TODO: return ステートメントをここに挿入します
	return this->_room_id;
}

void ChatAckMessage::set_room_id(const std::string& room_id)
{
	this->_room_id = room_id;
}

uint64_t ChatAckMessage::get_send_message_id() const
{
	return this->_send_message_id;
}

void ChatAckMessage::set_send_message_id(const uint64_t send_mid)
{
	this->_send_message_id = send_mid;
}

json ChatAckMessage::deserialize_message(const std::unique_ptr<ChatAckMessage>& chatAckMessageRef)
{
	json chat_ack_message, unsend_cids = json::array();
	try
	{
		
		if (chatAckMessageRef != nullptr) {
			chat_ack_message["content_type"] = "NOTIFICATION";
			chat_ack_message["message_type"] = chatAckMessageRef->_message_type;	
			switch (chatAckMessageRef->_send_status)
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
			chat_ack_message["room_id"] = chatAckMessageRef->get_room_id();
			chat_ack_message["send_message_id"] = chatAckMessageRef->get_send_message_id();
			if (chatAckMessageRef->_unsend_cid_list.size() > 0) {
				chat_ack_message["unsend_cid_list"] = chatAckMessageRef->_unsend_cid_list;
			}
			else {
				chat_ack_message["unsend_cid_list"] = json::array();
			}
		}
		else {
			throw std::runtime_error("ChatAckMessage is null");
		}
		
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error deserializing ChatAckMessage: {}.", e.what());
	}

	return chat_ack_message;
}
