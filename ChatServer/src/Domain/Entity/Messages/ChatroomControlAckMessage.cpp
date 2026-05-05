#include "Domain/Entity/Messages/ChatroomControlAckMessage.hpp"

ChatroomControlAckMessage::ChatroomControlAckMessage() 
	: MessageBase(MessageBase::MessageType::CHATROOM_CONTROL_ACK),_send_status(MessageSendStatus::SEND_FAILED), _room_id(""), _unsend_cid_list()
{
} 
ChatroomControlAckMessage::ChatroomControlAckMessage(
    const MessageBase::CommonField& common_fields,
    MessageSendStatus send_status,
    const std::string &room_id,
    uint64_t send_message_id,
    const std::vector<std::string> &unsend_cid_list)
    : MessageBase(common_fields, MessageBase::MessageType::CHATROOM_CONTROL_ACK),
	  _message_id(send_message_id),
      _send_status(send_status),
      _room_id(room_id),
      _unsend_cid_list(unsend_cid_list)
{
}

ChatroomControlAckMessage::~ChatroomControlAckMessage()
{
}


json ChatroomControlAckMessage::serialize_payload() const
{
	json payload, specific_fields, unsend_cids = json::array();

	try
	{
		if(validate_payload() == false) {
			throw std::runtime_error("ChatRoomControlAckMessage specific fields validation failed during serialization.");
		}else{
			specific_fields["content_type"] = "NOTIFICATION";
			specific_fields["mt"] = this->get_message_type();
			switch (this->_send_status)
			{
				case MessageSendStatus::SEND_SUCCESS:
					specific_fields["send_status"] = "SEND_SUCCESS";
					break;
				case MessageSendStatus::SEND_PARTICAL_SUCCESS:
					specific_fields["send_status"] = "SEND_PARTICAL_SUCCESS";
					break;
				case MessageSendStatus::SEND_FAILED:
					specific_fields["send_status"] = "SEND_FAILED";
					break;
			default:
				specific_fields["send_status"] = "SEND_FAILED";
				break;
			}
			specific_fields["room_id"] = this->_room_id;
			specific_fields["send_message_id"] = this->get_message_id();
			if (this->_unsend_cid_list.size() > 0) {
				specific_fields["unsend_cid_list"] = this->_unsend_cid_list;
			}else if(this->_unsend_cid_list.size() == 0){
				specific_fields["unsend_cid_list"] = json::array();
			}else{
				throw std::runtime_error("Unsend CID list size is invalid.");
				return json();
			}
			payload["payload"] = specific_fields;
			return payload;

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
}

bool ChatroomControlAckMessage::validate_payload() const
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

bool ChatroomControlAckMessage::is_valid_room_id(const std::string &room_id) const
{
	if (room_id.size() != 32) {
		LOG_ERROR("dst chatroom id is invalid. dst chatroom id byte size : {}", room_id.size());
		return false;
	}
	return true;
}
