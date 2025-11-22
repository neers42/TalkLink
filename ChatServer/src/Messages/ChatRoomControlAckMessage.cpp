#include "Messages/ChatRoomControlAckMessage.hpp"

ChatRoomControlAckMessage::ChatRoomControlAckMessage() {} 
ChatRoomControlAckMessage::ChatRoomControlAckMessage(
    int content_type,
    const std::string &mt,
    MessageSendStatus send_status,
    const std::string &room_id,
    uint64_t send_message_id,
    const std::vector<std::string> &unsend_cid_list)
    : MessageBase(static_cast<MessageBase::ContentType>(content_type), mt, send_message_id),
      _send_status(send_status),
      _room_id(room_id),
      _unsend_cid_list(unsend_cid_list)
{
}

ChatRoomControlAckMessage::~ChatRoomControlAckMessage()
{
}


json ChatRoomControlAckMessage::serialize_specific_message_fields() const
{
	json chatroom_control_ack_json, unsend_cids = json::array();

	try
	{
		if(validate_specific_message_fields() == false) {
			throw std::runtime_error("ChatRoomControlAckMessage specific fields validation failed during serialization.");
		}else{
			chatroom_control_ack_json["content_type"] = "NOTIFICATION";
			chatroom_control_ack_json["mt"] = this->get_message_type();
			switch (this->_send_status)
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
			chatroom_control_ack_json["room_id"] = this->_room_id;
			chatroom_control_ack_json["send_message_id"] = this->get_message_id();
			if (this->_unsend_cid_list.size() > 0) {
				chatroom_control_ack_json["unsend_cid_list"] = this->_unsend_cid_list;
			}else if(this->_unsend_cid_list.size() == 0){
				chatroom_control_ack_json["unsend_cid_list"] = json::array();
			}else{
				throw std::runtime_error("Unsend CID list size is invalid.");
			}
		}
	}
	catch (const std::runtime_error& re)
	{
		LOG_ERROR("Error serializing ChatRoomControlAckMessage specific fields: {}", re.what());
		return json();
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error deserializing ChatRoomControlAck message: {}", e.what());
		return json();
	}
	return chatroom_control_ack_json;
}

bool ChatRoomControlAckMessage::validate_specific_message_fields() const
{
	try
	{
		if(is_valid_room_id(_room_id) == false){
			throw std::runtime_error("Invalid room ID");
		}

	}
	catch(const std::exception& e)
	{
		LOG_ERROR("Specific message fields validation failed: {}", e.what());
		return false;
	}

    return true;
}

bool ChatRoomControlAckMessage::is_valid_room_id(const std::string &room_id) const
{
	if (room_id.size() != 32) {
		LOG_ERROR("dst chatroom id is invalid. dst chatroom id byte size : {}", room_id.size());
		return false;
	}
	return true;
}
