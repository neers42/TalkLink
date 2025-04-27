#include "MessageType003.h"


MessageType003::MessageType003(
	const std::string& sender_id,
	const std::string& room_id_add_member,
	const std::string& room_id_remove_member,
	const std::vector<std::string>& add_member_list,
	const std::vector<std::string>& remove_member_list,
	const std::string& room_name,
	const std::string& room_image,
	const std::string& room_id,
	const RoomAction& room_action,
	const std::string& room_create_time
):BaseMessage(BaseMessage::ContentType::NOTIFICATION, "003"),
_sender_id(sender_id),
_room_id_add_member(room_id_add_member),
_room_id_remove_member(room_id_remove_member),
_add_member_list(add_member_list),
_remove_member_list(remove_member_list),
_room_name(room_name),
_room_image_url(room_image),
_room_id(room_id),
_room_action(room_action),
_room_create_time(room_create_time)
{
}

MessageType003::~MessageType003()
{
}

std::string MessageType003::get_sender_id() const
{
	return this->_sender_id;
}

std::string MessageType003::get_room_id_add_member() const
{
	return this->_room_id_add_member;
}

std::string MessageType003::get_room_id_remove_member() const
{
	return this->_room_id_remove_member;
}

std::vector<std::string> MessageType003::get_add_member_list() const
{
	return this->_add_member_list;
}

std::vector<std::string> MessageType003::get_remove_member_list() const
{
	return this->_remove_member_list;
}

std::string MessageType003::get_room_name() const
{
	return this->_room_name;
}

std::string MessageType003::get_room_image_url() const
{
	return this->_room_image_url;
}

std::string MessageType003::get_room_id() const
{
	return this->_room_id;
}

MessageType003::RoomAction MessageType003::get_room_action() const
{
	return this->_room_action;
}

std::string MessageType003::get_room_create_time() const
{
	return this->_room_create_time;
}

std::unique_ptr<MessageType003> MessageType003::serialize_message(const json& recv_json)
{
	RoomAction room_action = RoomAction::CREATE_ROOM;
	return std::make_unique<MessageType003>	(
		recv_json["sender_id"].get<std::string>(),
		recv_json["room_id_add_member"].get<std::string>(),
		recv_json["room_id_remove_member"].get<std::string>(),
		recv_json["add_member_list"].get<std::vector<std::string>>(),
		recv_json["remove_member_list"].get<std::vector<std::string>>(),
		recv_json["room_name"].get<std::string>(),
		recv_json["room_image_url"].get<std::string>(),
		recv_json["room_id"].get<std::string>(),
		room_action,
		recv_json["room_create_time"].get<std::string>()
		);
}
