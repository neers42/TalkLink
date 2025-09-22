#include "Messages/ChatRoomControlDeliveryMessage.hpp"


ChatRoomControlDeliveryMessage::ChatRoomControlDeliveryMessage(
	const std::string& sender_cid,
	const std::string& room_id, 
	const std::string& room_name, 
	const std::string& room_icon_url, 
	const std::vector<ClientInfo>& client_info_list, 
	const ChatRoomControlMessage::RoomAction& room_action
):BaseMessage(ChatRoomControlDeliveryMessage::ContentType::NOTIFICATION, "ChatRoomControlDelivery", MessageIDGenerator::generate_message_id()),
_sender_cid{ sender_cid },
_room_id{ room_id },
_room_name{ room_name },
_room_icon_url{ room_icon_url },
_client_info_list{ client_info_list },
_room_action{ room_action }
{
}

ChatRoomControlDeliveryMessage::~ChatRoomControlDeliveryMessage()
{
}

std::string ChatRoomControlDeliveryMessage::get_sender_cid() const
{
	return this->_sender_cid;
}
std::string ChatRoomControlDeliveryMessage::get_room_id() const
{
    return this->_room_id;
}

void ChatRoomControlDeliveryMessage::set_room_id(const std::string& room_id)
{
	this->_room_id = room_id;
}

std::string ChatRoomControlDeliveryMessage::get_room_name() const
{
    return this->_room_name;
}

void ChatRoomControlDeliveryMessage::set_room_name(const std::string& room_name)
{
	this->_room_name = room_name;
}

std::string ChatRoomControlDeliveryMessage::get_room_icon_url() const
{
    return this->_room_icon_url;
}

void ChatRoomControlDeliveryMessage::set_room_icon_url(const std::string& room_icon_url)
{
	this->_room_icon_url = room_icon_url;
}

std::vector<ChatRoomControlDeliveryMessage::ClientInfo> ChatRoomControlDeliveryMessage::get_client_info_list() const
{
    return this->_client_info_list;
}

void ChatRoomControlDeliveryMessage::set_client_info_list(const std::vector<ClientInfo>& client_info_list)
{
	this->_client_info_list = client_info_list;
}

ChatRoomControlMessage::RoomAction ChatRoomControlDeliveryMessage::get_room_action() const
{
	return this->_room_action;
}


json ChatRoomControlDeliveryMessage::deserialize_message(const std::unique_ptr<ChatRoomControlDeliveryMessage>& chatroom_delivery_message)
{
	json chatroom_delivery_json, room_info = json::array(), member = json::array();
	switch (chatroom_delivery_message->get_content_type())
	{
	case ChatRoomControlDeliveryMessage::ContentType::NOTIFICATION:
		chatroom_delivery_json["content_type"] = "NOTIFICATION";
		break;
	default:
		chatroom_delivery_json["content_type"] = "NOTIFICATION";
		break;
	}
	chatroom_delivery_json["message_type"] = chatroom_delivery_message->get_message_type();
	chatroom_delivery_json["message_id"] = chatroom_delivery_message->get_message_id();
	chatroom_delivery_json["created_time"] = chatroom_delivery_message->get_created_at();
	switch (chatroom_delivery_message->get_room_action())
	{
	case ChatRoomControlMessage::RoomAction::CREATE_ROOM:
		chatroom_delivery_json["room_action"] = "CREATE_ROOM";
		break;
	case ChatRoomControlMessage::RoomAction::LEAVE_ROOM:
		chatroom_delivery_json["room_action"] = "LEAVE_ROOM";
		break;
	case ChatRoomControlMessage::RoomAction::ADD_MEMBER:
		chatroom_delivery_json["room_action"] = "ADD_MEMBER";
		break;
	case ChatRoomControlMessage::RoomAction::REMOVE_MEMBER:
		chatroom_delivery_json["room_action"] = "REMOVE_MEMBER";
		break;
	case ChatRoomControlMessage::RoomAction::ROOM_NAME_CHANGE:
		chatroom_delivery_json["room_action"] = "ROOM_NAME_CHANGE";
		break;
	case ChatRoomControlMessage::RoomAction::ROOM_ICON_CHANGE:
		chatroom_delivery_json["room_action"] = "ROOM_ICON_CHANGE";
		break;
	default:
		chatroom_delivery_json["room_action"] = "UNKNOWN";
		break;
	}
	// ルーム情報
	chatroom_delivery_json["room_id"] = chatroom_delivery_message->get_room_id();
	chatroom_delivery_json["room_name"] = chatroom_delivery_message->get_room_name();
	chatroom_delivery_json["room_icon_url"] = chatroom_delivery_message->get_room_icon_url();

	switch (chatroom_delivery_message->get_room_action())
	{
	case ChatRoomControlMessage::RoomAction::CREATE_ROOM:
	case ChatRoomControlMessage::RoomAction::ADD_MEMBER:
	case ChatRoomControlMessage::RoomAction::REMOVE_MEMBER: {
		// クライアント情報（members）
		json members_json = json::array();
		for (const auto& member : chatroom_delivery_message->get_client_info_list()) {
			json m;
			m["cid"] = member.cid;
			m["name"] = member.name;
			m["icon_url"] = member.icon_url;
			members_json.push_back(m);
		}
		chatroom_delivery_json["members"] = members_json;
		break;
	}
	default:
		break;
		
	}
	

	return chatroom_delivery_json;

}
