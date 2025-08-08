#include "Network/MessageType103B.hpp"

MessageType103B::MessageType103B(
	int content_type, 
	const std::string& mt, 
	MessageSendStatus send_status, 
	const std::string& room_id, 
	uint64_t send_message_id, 
	const std::vector<std::string>& unsend_cid_list
)
	:BaseMessage(exchange_content_type_from_int(content_type), mt),
	_send_status(send_status),
	_room_id(room_id),
	_send_message_id(send_message_id),
	_unsend_cid_list(unsend_cid_list)
{
}

MessageType103B::~MessageType103B()
{
}

json MessageType103B::deserialize_message(const std::unique_ptr<MessageType103B>& mt103B)
{
	json mt103B_json, unsend_cids = json::array();

	try
	{
		if (mt103B != nullptr) {
			mt103B_json["content_type"] = "NOTIFICATION";
			mt103B_json["mt"] = mt103B->get_message_type();
			switch (mt103B->_send_status)
			{
				case MessageSendStatus::SEND_SUCCESS:
					mt103B_json["send_status"] = "SEND_SUCCESS";
					break;
				case MessageSendStatus::SEND_PARTICAL_SUCCESS:
					mt103B_json["send_status"] = "SEND_PARTICAL_SUCCESS";
					break;
				case MessageSendStatus::SEND_FAILED:
					mt103B_json["send_status"] = "SEND_FAILED";
					break;
			default:
				mt103B_json["send_status"] = "SEND_FAILED";
				break;
			}
			mt103B_json["room_id"] = mt103B->_room_id;
			mt103B_json["send_message_id"] = mt103B->_send_message_id;
			if (mt103B->_unsend_cid_list.size() > 0) {
				mt103B_json["unsend_cid_list"] = mt103B->_unsend_cid_list;
			}
		}
		else {
			throw std::runtime_error("MT103B is null");
		}
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error deserializing MT103B: {}", e.what());
	}
	return mt103B_json;
}
