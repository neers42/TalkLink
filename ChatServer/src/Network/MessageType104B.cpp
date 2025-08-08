#include "Network/MessageType104B.hpp"


MessageType104B::MessageType104B(int c_type,
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

MessageType104B::~MessageType104B()
{
}

int MessageType104B::get_status() const
{
	switch (this->_send_status)
	{
	case MessageType104B::MessageSendStatus::SEND_SUCCESS:
		return 0;
		break;
	case MessageType104B::MessageSendStatus::SEND_PARTIAL_SUCCESS:
		return 1;
		break;
	case MessageType104B::MessageSendStatus::SEND_FAILED:
		return 2;
		break;
	default:
		return 2;
		break;
	}
}

void MessageType104B::set_status(const int status)
{
	switch (status)
	{
	case 0:
		this->_send_status = MessageType104B::MessageSendStatus::SEND_SUCCESS;
		break;
	case 1:
		this->_send_status = MessageType104B::MessageSendStatus::SEND_PARTIAL_SUCCESS;
		break;
	case 2:
		this->_send_status = MessageType104B::MessageSendStatus::SEND_FAILED;
		break;
	default:
		this->_send_status = MessageType104B::MessageSendStatus::SEND_FAILED;
		break;
	}
}


std::string MessageType104B::get_room_id() const
{
	// TODO: return ステートメントをここに挿入します
	return this->_room_id;
}

void MessageType104B::set_room_id(const std::string& room_id)
{
	this->_room_id = room_id;
}

uint64_t MessageType104B::get_send_message_id() const
{
	return this->_send_message_id;
}

void MessageType104B::set_send_message_id(const uint64_t send_mid)
{
	this->_send_message_id = send_mid;
}

json MessageType104B::deserialize_message(const std::unique_ptr<MessageType104B>& mt104B)
{
	json mt104B_json, unsend_cids = json::array();
	try
	{
		
		if (mt104B != nullptr) {
			mt104B_json["content_type"] = "NOTIFICATION";
			mt104B_json["message_type"] = mt104B->_message_type;	
			switch (mt104B->_send_status)
			{
			case MessageType104B::MessageSendStatus::SEND_SUCCESS:
				mt104B_json["send_status"] = "SEND_SUCCESS";
				break;
			case MessageType104B::MessageSendStatus::SEND_PARTIAL_SUCCESS:
				mt104B_json["send_status"] = "SEND_PARTIAL_SUCCESS";
				break;
			case MessageType104B::MessageSendStatus::SEND_FAILED:
				mt104B_json["send_status"] = "SEND_FAILED";
				break;
			default:
				mt104B_json["send_status"] = "SEND_FAILED";
				break;
			}
			mt104B_json["room_id"] = mt104B->get_room_id();
			mt104B_json["send_message_id"] = mt104B->get_send_message_id();
			if (mt104B->_unsend_cid_list.size() > 0) {
				mt104B_json["unsend_cid_list"] = mt104B->_unsend_cid_list;
			}
			else {
				mt104B_json["unsend_cid_list"] = json::array();
			}
		}
		else {
			throw std::runtime_error("MT104B is null");
		}
		
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error deserializing MessageType104B: {}.", e.what());
	}

	return mt104B_json;
}
