#include "Messages/ChatRoomControlMessage.hpp"


ChatRoomControlMessage::ChatRoomControlMessage(
	const uint64_t mid,
	const std::string& sender_cid,
	const std::string& room_id,
	const std::vector<std::string>& add_member_list,
	const std::vector<std::string>& remove_member_list,
	const std::string& room_name,
	const std::string& room_image,
	const RoomAction& room_action,
	const std::string& room_create_time,
	const std::string& message_content
):BaseMessage(BaseMessage::ContentType::NOTIFICATION, "ChatRoomControl", mid),
_sender_cid(sender_cid),
_room_id(room_id),
_add_member_list(add_member_list),
_remove_member_list(remove_member_list),
_room_name(room_name),
_room_image_url(room_image),
_room_action(room_action),
_room_create_time(room_create_time),
_message_content(message_content)
{
}

ChatRoomControlMessage::~ChatRoomControlMessage()
{
}

std::string ChatRoomControlMessage::get_sender_cid() const
{
	return this->_sender_cid;
}

std::string ChatRoomControlMessage::get_room_id() const
{
	return this->_room_id;
}


std::vector<std::string> ChatRoomControlMessage::get_add_member_list() const
{
	return this->_add_member_list;
}

std::vector<std::string> ChatRoomControlMessage::get_remove_member_list() const
{
	return this->_remove_member_list;
}

std::string ChatRoomControlMessage::get_room_name() const
{
	return this->_room_name;
}

std::string ChatRoomControlMessage::get_room_image_url() const
{
	return this->_room_image_url;
}


ChatRoomControlMessage::RoomAction ChatRoomControlMessage::get_room_action() const
{
	return this->_room_action;
}

std::string ChatRoomControlMessage::get_room_create_time() const
{
	return this->_room_create_time;
}

std::string ChatRoomControlMessage::get_message_content() const
{
    return this->_message_content;
}
std::unique_ptr<ChatRoomControlMessage> ChatRoomControlMessage::serialize_message(const json &recv_json)
{
	RoomAction room_action = RoomAction::CREATE_ROOM;
	try
	{
		static const std::vector<std::string> required_keys = {
			"content_type", 
			"message_type",
			"sender_cid", 
			"room_id",
			"room_add_member_list",
			"room_remove_member_list",
			"room_name",
			"room_image_url",
			"room_action",
			"room_create_time"
		};

		for(const auto& key : required_keys)
		{
			if (!recv_json.contains(key)) {
				throw std::runtime_error("Missing key: " + key);
			}
		}

		if (recv_json["room_action"] == "CREATE_ROOM")
		{
			room_action = RoomAction::CREATE_ROOM;
		}
		else if (recv_json["room_action"] == "LEAVE_ROOM")
		{
			room_action = RoomAction::LEAVE_ROOM;
		}
		else if (recv_json["room_action"] == "ADD_MEMBER")
		{
			room_action = RoomAction::ADD_MEMBER;
		}
		else if (recv_json["room_action"] == "REMOVE_MEMBER")
		{
			room_action = RoomAction::REMOVE_MEMBER;
		}
		else if (recv_json["room_action"] == "ROOM_NAME_CHANGE")
		{
			room_action = RoomAction::ROOM_NAME_CHANGE;
		}
		else if (recv_json["room_action"] == "ROOM_ICON_CHANGE")
		{
			room_action = RoomAction::ROOM_ICON_CHANGE;
		}
		else
		{
			LOG_ERROR("The room action is not found.");
			return nullptr;
		}
	}
	catch(const std::exception& e)
	{
		LOG_ERROR("Error deserializing ChatRoomControl message: {}", e.what());
		return nullptr;
	}
	
	

	uint64_t message_id = MessageIDGenerator::generate_message_id();
	std::string message_content = recv_json.dump();

	return std::make_unique<ChatRoomControlMessage>	(
		message_id,
		recv_json["sender_cid"].get<std::string>(),
		recv_json["room_id"].get<std::string>(),
		recv_json["add_member_list"].get<std::vector<std::string>>(),
		recv_json["remove_member_list"].get<std::vector<std::string>>(),
		recv_json["room_name"].get<std::string>(),
		recv_json["room_image_url"].get<std::string>(),
		room_action,
		recv_json["room_create_time"].get<std::string>(),
		message_content
		);
}
