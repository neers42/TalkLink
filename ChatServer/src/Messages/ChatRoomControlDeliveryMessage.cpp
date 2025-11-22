#include "Messages/ChatRoomControlDeliveryMessage.hpp"

ChatRoomControlDeliveryMessage::ChatRoomControlDeliveryMessage() {

}
ChatRoomControlDeliveryMessage::ChatRoomControlDeliveryMessage(
	const std::uint64_t& mid,
	const std::string& time_stamp,
    const std::string &sender_cid,
    const std::string &room_id,
    const std::string &room_name,
    const std::string &room_icon_url,
    const std::vector<ClientInfo> &client_info_list,
    const ChatRoomControlMessage::RoomAction &room_action) : MessageBase(MessageBase::ContentType::NOTIFICATION, "ChatRoomControlDelivery", mid, time_stamp),
                                                             _sender_cid{sender_cid},
                                                             _room_id{room_id},
                                                             _room_name{room_name},
                                                             _room_icon_url{room_icon_url},
                                                             _client_info_list{client_info_list},
                                                             _room_action{room_action}
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



json ChatRoomControlDeliveryMessage::serialize_specific_message_fields() const
{
	json specific_fields, member = json::array();
	try
	{
		if(validate_specific_message_fields() == false){
			throw std::runtime_error("Specific message fields validation failed.");
		}
		specific_fields["sender_cid"] = this->_sender_cid;
		specific_fields["room_id"] = this->_room_id;
		specific_fields["room_name"] = this->_room_name;
		specific_fields["room_icon_url"] = this->_room_icon_url;

		switch (this->get_room_action())
		{
		case ChatRoomControlMessage::RoomAction::CREATE_ROOM:
		case ChatRoomControlMessage::RoomAction::ADD_MEMBER:
		case ChatRoomControlMessage::RoomAction::REMOVE_MEMBER:
		{
			// クライアント情報（members）
			json members_json = json::array();
			for (const auto &member : this->get_client_info_list())
			{
				json m;
				m["cid"] = member.cid;
				m["name"] = member.name;
				m["icon_url"] = member.icon_url;
				members_json.push_back(m);
			}
			specific_fields["members"] = members_json;
			break;
		}
		default:
			break;
		}
	}
	catch(const std::runtime_error& e)
	{
		LOG_ERROR("Error serializing ChatRoomControlDeliveryMessage: {}", e.what());
		return json();
	}
	catch(const std::exception& e)
	{
		LOG_ERROR("Error serializing ChatRoomControlDeliveryMessage: {}", e.what());
		return json();
	}
    return specific_fields;
}

bool ChatRoomControlDeliveryMessage::validate_specific_message_fields() const
{
	try
	{
		if(is_valid_room_id(this->_room_id) == false ||
		   is_valid_room_name(this->_room_name) == false ||
		   is_valid_room_icon_url(this->_room_icon_url) == false){
			throw std::runtime_error("ChatRoomControlDeliveryMessage specific fields validation failed.");
		}
	}
	catch(const std::exception& e)
	{
		LOG_ERROR("Error validating ChatRoomControlDeliveryMessage: {}", e.what());
		return false;
	}

    return true;
}

ChatRoomControlMessage::RoomAction ChatRoomControlDeliveryMessage::parse_room_action(const std::string &action_str) const
{
    if (action_str == "CREATE_ROOM") return ChatRoomControlMessage::RoomAction::CREATE_ROOM;
    if (action_str == "LEAVE_ROOM") return ChatRoomControlMessage::RoomAction::LEAVE_ROOM;
    if (action_str == "ADD_MEMBER") return ChatRoomControlMessage::RoomAction::ADD_MEMBER;
    if (action_str == "REMOVE_MEMBER") return ChatRoomControlMessage::RoomAction::REMOVE_MEMBER;
    if (action_str == "ROOM_NAME_CHANGE") return ChatRoomControlMessage::RoomAction::ROOM_NAME_CHANGE;
    if (action_str == "ROOM_ICON_CHANGE") return ChatRoomControlMessage::RoomAction::ROOM_ICON_CHANGE;
    
    throw std::runtime_error("Unknown room action: " + action_str);
}

bool ChatRoomControlDeliveryMessage::is_valid_room_id(const std::string &room_id) const
{
	if (room_id.size() != 32) {
		LOG_ERROR("dst chatroom id is invalid. dst chatroom id byte size : {}", room_id.size());
		return false;
	}
	return true;
}

bool ChatRoomControlDeliveryMessage::is_valid_room_name(const std::string &room_name) const
{
	if (room_name.empty() || room_name.size() > 100) {
		LOG_ERROR("dst chatroom name is invalid. dst chatroom name byte size : {}", room_name.size());
		return false;
	}
	return true;
}

bool ChatRoomControlDeliveryMessage::is_valid_room_icon_url(const std::string &room_icon_url) const
{
	if (room_icon_url.empty() || room_icon_url.size() > 200) {
		LOG_ERROR("dst chatroom icon url is invalid. dst chatroom icon url byte size : {}", room_icon_url.size());
		return false;
	}
	return true;
}
