#include "Domain/Entity/Messages/ChatroomControlDeliveryMessage.hpp"

ChatroomControlDeliveryMessage::ChatroomControlDeliveryMessage() 
	:MessageBase(MessageBase::MessageType::CHATROOM_CONTROL_DELIVERY),
	 _sender_cid{""},
	 _room_id{""},
	 _room_name{""},
	 _room_icon_url{""},
	 _client_info_list{},
	 _room_action{ChatroomControlMessage::RoomAction::CREATE_ROOM}
{
}

ChatroomControlDeliveryMessage::ChatroomControlDeliveryMessage(
	const MessageBase::CommonField& common_fields,
	const std::uint64_t& mid,
    const std::string &sender_cid,
    const std::string &room_id,
    const std::string &room_name,
    const std::string &room_icon_url,
    const std::vector<ClientInfo> &client_info_list,
    const ChatroomControlMessage::RoomAction &room_action) 
	: MessageBase(common_fields, MessageBase::MessageType::CHATROOM_CONTROL_DELIVERY),
	 _sender_cid{sender_cid},
	 _room_id{room_id},
	 _room_name{room_name},
	 _room_icon_url{room_icon_url},
	 _client_info_list{client_info_list},
	 _room_action{room_action}
{
}

ChatroomControlDeliveryMessage::~ChatroomControlDeliveryMessage()
{
}

std::string ChatroomControlDeliveryMessage::get_sender_cid() const
{
	return this->_sender_cid;
}
std::string ChatroomControlDeliveryMessage::get_room_id() const
{
    return this->_room_id;
}

void ChatroomControlDeliveryMessage::set_room_id(const std::string& room_id)
{
	this->_room_id = room_id;
}

std::string ChatroomControlDeliveryMessage::get_room_name() const
{
    return this->_room_name;
}

void ChatroomControlDeliveryMessage::set_room_name(const std::string& room_name)
{
	this->_room_name = room_name;
}

std::string ChatroomControlDeliveryMessage::get_room_icon_url() const
{
    return this->_room_icon_url;
}

void ChatroomControlDeliveryMessage::set_room_icon_url(const std::string& room_icon_url)
{
	this->_room_icon_url = room_icon_url;
}

std::vector<ChatroomControlDeliveryMessage::ClientInfo> ChatroomControlDeliveryMessage::get_client_info_list() const
{
    return this->_client_info_list;
}

void ChatroomControlDeliveryMessage::set_client_info_list(const std::vector<ClientInfo>& client_info_list)
{
	this->_client_info_list = client_info_list;
}

ChatroomControlMessage::RoomAction ChatroomControlDeliveryMessage::get_room_action() const
{
	return this->_room_action;
}



json ChatroomControlDeliveryMessage::serialize_payload() const
{
	json payload, specific_fields, member = json::array();
	try
	{
		if(validate_payload() == false){
			throw std::runtime_error("Specific message fields validation failed.");
		}
		specific_fields["sender_cid"] = this->_sender_cid;
		specific_fields["room_id"] = this->_room_id;
		specific_fields["room_name"] = this->_room_name;
		specific_fields["room_icon_url"] = this->_room_icon_url;

		switch (this->get_room_action())
		{
		case ChatroomControlMessage::RoomAction::CREATE_ROOM:
		case ChatroomControlMessage::RoomAction::ADD_MEMBER:
		case ChatroomControlMessage::RoomAction::REMOVE_MEMBER:
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
			payload["payload"] = specific_fields;
			return payload;
			break;
		}
		default:
			throw std::runtime_error("Unknown room action during serialization.");
			return json();
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
}

bool ChatroomControlDeliveryMessage::validate_payload() const
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

ChatroomControlMessage::RoomAction ChatroomControlDeliveryMessage::parse_room_action(const std::string &action_str) const
{
    if (action_str == "CREATE_ROOM") return ChatroomControlMessage::RoomAction::CREATE_ROOM;
    if (action_str == "LEAVE_ROOM") return ChatroomControlMessage::RoomAction::LEAVE_ROOM;
    if (action_str == "ADD_MEMBER") return ChatroomControlMessage::RoomAction::ADD_MEMBER;
    if (action_str == "REMOVE_MEMBER") return ChatroomControlMessage::RoomAction::REMOVE_MEMBER;
    if (action_str == "ROOM_NAME_CHANGE") return ChatroomControlMessage::RoomAction::ROOM_NAME_CHANGE;
    if (action_str == "ROOM_ICON_CHANGE") return ChatroomControlMessage::RoomAction::ROOM_ICON_CHANGE;
    
    throw std::runtime_error("Unknown room action: " + action_str);
}

bool ChatroomControlDeliveryMessage::is_valid_room_id(const std::string &room_id) const
{
	if (room_id.size() != 32) {
		LOG_ERROR("dst chatroom id is invalid. dst chatroom id byte size : {}", room_id.size());
		return false;
	}
	return true;
}

bool ChatroomControlDeliveryMessage::is_valid_room_name(const std::string &room_name) const
{
	if (room_name.empty() || room_name.size() > 100) {
		LOG_ERROR("dst chatroom name is invalid. dst chatroom name byte size : {}", room_name.size());
		return false;
	}
	return true;
}

bool ChatroomControlDeliveryMessage::is_valid_room_icon_url(const std::string &room_icon_url) const
{
	if (room_icon_url.empty() || room_icon_url.size() > 200) {
		LOG_ERROR("dst chatroom icon url is invalid. dst chatroom icon url byte size : {}", room_icon_url.size());
		return false;
	}
	return true;
}
