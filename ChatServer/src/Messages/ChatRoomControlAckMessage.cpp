#include "Messages/ChatRoomControlAckMessage.hpp"

ChatRoomControlAckMessage::ChatRoomControlAckMessage(
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

ChatRoomControlAckMessage::~ChatRoomControlAckMessage()
{
}

json ChatRoomControlAckMessage::deserialize_message(const std::unique_ptr<ChatRoomControlAckMessage>& chatroom_control_ack_message)
{
	json chatroom_control_ack_json, unsend_cids = json::array();

	try
	{
		if (chatroom_control_ack_message != nullptr) {
			chatroom_control_ack_json["content_type"] = "NOTIFICATION";
			chatroom_control_ack_json["mt"] = chatroom_control_ack_message->get_message_type();
			switch (chatroom_control_ack_message->_send_status)
			{
				case MessageSendStatus::SEND_SUCCESS:
					chatroom_control_ack_json["send_status"] = "SEND_SUCCESS";
					break;
				case MessageSendStatus::SEND_PARTICAL_SUCCESS:
					chatroom_control_ack_json["send_status"] = "SEND_PARTICAL_SUCCESS";
					break;
				case MessageSendStatus::SEND_FAILED:
					chatroom_control_ack_json["send_status"] = "SEND_FAILED";
					break;
			default:
				chatroom_control_ack_json["send_status"] = "SEND_FAILED";
				break;
			}
			chatroom_control_ack_json["room_id"] = chatroom_control_ack_message->_room_id;
			chatroom_control_ack_json["send_message_id"] = chatroom_control_ack_message->_send_message_id;
			if (chatroom_control_ack_message->_unsend_cid_list.size() > 0) {
				chatroom_control_ack_json["unsend_cid_list"] = chatroom_control_ack_message->_unsend_cid_list;
			}
		}
		else {
			throw std::runtime_error("ChatRoomControlAck message is null");
		}
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error deserializing ChatRoomControlAck message: {}", e.what());
	}
	return chatroom_control_ack_json;
}
