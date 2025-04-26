#include "MessageType103B.h"


MessageType103B::MessageType103B(int c_type,
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

MessageType103B::~MessageType103B()
{
}

int MessageType103B::get_status() const
{
	switch (this->_send_status)
	{
	case MessageType103B::MessageSendStatus::SEND_SUCCESS:
		return 0;
		break;
	case MessageType103B::MessageSendStatus::SEND_PARTIAL_SUCCESS:
		return 1;
		break;
	case MessageType103B::MessageSendStatus::SEND_FAILED:
		return 2;
		break;
	default:
		return 2;
		break;
	}
}

void MessageType103B::set_status(const int status)
{
	switch (status)
	{
	case 0:
		this->_send_status = MessageType103B::MessageSendStatus::SEND_SUCCESS;
		break;
	case 1:
		this->_send_status = MessageType103B::MessageSendStatus::SEND_PARTIAL_SUCCESS;
		break;
	case 2:
		this->_send_status = MessageType103B::MessageSendStatus::SEND_FAILED;
		break;
	default:
		this->_send_status = MessageType103B::MessageSendStatus::SEND_FAILED;
		break;
	}
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

uint64_t MessageType103B::get_send_message_id() const
{
	return this->_send_message_id;
}

void MessageType103B::set_send_message_id(const uint64_t send_mid)
{
	this->_send_message_id = send_mid;
}

json MessageType103B::desirialize_message(const std::unique_ptr<MessageType103B>& mt103B)
{
	json mt103B_json, unsend_cids = json::array();
	try
	{
		
		if (mt103B != nullptr) {
			mt103B_json["content_type"] = "NOTIFICATION";
			mt103B_json["message_type"] = mt103B->_message_type;	
			switch (mt103B->_send_status)
			{
			case MessageType103B::MessageSendStatus::SEND_SUCCESS:
				mt103B_json["send_status"] = "SEND_SUCCESS";
				break;
			case MessageType103B::MessageSendStatus::SEND_PARTIAL_SUCCESS:
				mt103B_json["send_status"] = "SEND_PARTIAL_SUCCESS";
				break;
			case MessageType103B::MessageSendStatus::SEND_FAILED:
				mt103B_json["send_status"] = "SEND_FAILED";
				break;
			default:
				mt103B_json["send_status"] = "SEND_FAILED";
				break;
			}
			mt103B_json["room_id"] = mt103B->get_room_id();
			mt103B_json["send_message_id"] = mt103B->get_send_message_id();
			if (mt103B->_unsend_cid_list.size() > 0) {
				mt103B_json["unsend_cid_list"] = mt103B->_unsend_cid_list;
			}
			else {
				mt103B_json["unsend_cid_list"] = json::array();
			}
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
