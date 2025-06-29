#include "Network/MessageType103A.h"


MessageType103A::MessageType103A(
	const std::string& sender_id,
	const std::string& room_id, 
	const std::string& room_name, 
	const std::string& room_icon_url, 
	const std::vector<ClientInfo>& client_info_list, 
	const MessageType003::RoomAction& room_action
):BaseMessage(MessageType103A::ContentType::NOTIFICATION, "103A", MessageIDGenerator::generate_message_id()),
_sender_id{ sender_id },
_room_id{ room_id },
_room_name{ room_name },
_room_icon_url{ room_icon_url },
_client_info_list{ client_info_list },
_room_action{ room_action }
{
}

MessageType103A::~MessageType103A()
{
}

std::string MessageType103A::get_sender_id() const
{
	return this->_sender_id;
}
std::string MessageType103A::get_room_id() const
{
    return this->_room_id;
}

void MessageType103A::set_room_id(const std::string& room_id)
{
	this->_room_id = room_id;
}

std::string MessageType103A::get_room_name() const
{
    return this->_room_name;
}

void MessageType103A::set_room_name(const std::string& room_name)
{
	this->_room_name = room_name;
}

std::string MessageType103A::get_room_icon_url() const
{
    return this->_room_icon_url;
}

void MessageType103A::set_room_icon_url(const std::string& room_icon_url)
{
	this->_room_icon_url = room_icon_url;
}

std::vector<MessageType103A::ClientInfo> MessageType103A::get_client_info_list() const
{
    return this->_client_info_list;
}

void MessageType103A::set_client_info_list(const std::vector<ClientInfo>& client_info_list)
{
	this->_client_info_list = client_info_list;
}

MessageType003::RoomAction MessageType103A::get_room_action() const
{
	return this->_room_action;
}


json MessageType103A::deserialize_message(const std::unique_ptr<MessageType103A>& mt103A)
{
	json mt103A_json, room_info = json::array(), member = json::array();
	switch (mt103A->get_content_type())
	{
	case MessageType103A::ContentType::NOTIFICATION:
		mt103A_json["content_type"] = "NOTIFICATION";
		break;
	default:
		mt103A_json["content_type"] = "NOTIFICATION";
		break;
	}
	mt103A_json["message_type"] = "103A";
	mt103A_json["message_id"] = mt103A->get_message_id();
	mt103A_json["created_time"] = mt103A->get_created_at();
	switch (mt103A->get_room_action())
	{
	case MessageType003::RoomAction::CREATE_ROOM:
		mt103A_json["room_action"] = "CREATE_ROOM";
		break;
	case MessageType003::RoomAction::LEAVE_ROOM:
		mt103A_json["room_action"] = "LEAVE_ROOM";
		break;
	case MessageType003::RoomAction::ADD_MEMBER:
		mt103A_json["room_action"] = "ADD_MEMBER";
		break;
	case MessageType003::RoomAction::REMOVE_MEMBER:
		mt103A_json["room_action"] = "REMOVE_MEMBER";
		break;
	case MessageType003::RoomAction::ROOM_NAME_CHANGE:
		mt103A_json["room_action"] = "ROOM_NAME_CHANGE";
		break;
	case MessageType003::RoomAction::ROOM_ICON_CHANGE:
		mt103A_json["room_action"] = "ROOM_ICON_CHANGE";
		break;
	default:
		mt103A_json["room_action"] = "UNKNOWN";
		break;
	}
	// ルーム情報
	mt103A_json["room_id"] = mt103A->get_room_id();
	mt103A_json["room_name"] = mt103A->get_room_name();
	mt103A_json["room_icon_url"] = mt103A->get_room_icon_url();

	switch (mt103A->get_room_action())
	{
	case MessageType003::RoomAction::CREATE_ROOM:
	case MessageType003::RoomAction::ADD_MEMBER:
	case MessageType003::RoomAction::REMOVE_MEMBER: {
		// クライアント情報（members）
		json members_json = json::array();
		for (const auto& member : mt103A->get_client_info_list()) {
			json m;
			m["cid"] = member.cid;
			m["name"] = member.name;
			m["icon_url"] = member.icon_url;
			members_json.push_back(m);
		}
		mt103A_json["members"] = members_json;
		break;
	}
	default:
		break;
		
	}
	

	return mt103A_json;

}
