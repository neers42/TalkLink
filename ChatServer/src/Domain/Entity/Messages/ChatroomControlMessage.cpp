#include "Domain/Entity/Messages/ChatroomControlMessage.hpp"

ChatroomControlMessage::ChatroomControlMessage()
	:MessageBase(MessageBase::MessageType::CHATROOM_CONTROL),
	 _message_id{0},
	 _sender_uid{""},
	 _room_id{""},
	 _member_list{},
	 _room_name{""},
	 _icon_url{""},
	 _room_action{RoomAction::CREATE_ROOM},
	 _message_content{""}
{
} 

ChatroomControlMessage::ChatroomControlMessage(
	const MessageBase::CommonField& common_fields,
    const uint64_t mid,
    const std::string &sender_uid,
    const std::string &room_id,
    const std::vector<std::string> &member_list,
    const std::string &room_name,
    const std::string &icon_url,
    const RoomAction &room_action,
    const std::string &message_content) 
	: MessageBase(common_fields, MessageBase::MessageType::CHATROOM_CONTROL),
	_message_id(mid),
	_sender_uid(sender_uid),
    _room_id(room_id),
    _member_list(member_list),
    _room_name(room_name),
    _icon_url(icon_url),
    _room_action(room_action),
	_message_content(message_content)
{
}

ChatroomControlMessage::~ChatroomControlMessage()
{
}

uint64_t ChatroomControlMessage::get_message_id() const
{
	return this->_message_id;
}

std::string ChatroomControlMessage::get_sender_uid() const
{
	return this->_sender_uid;
}

std::string ChatroomControlMessage::get_room_id() const
{
	return this->_room_id;
}


std::vector<std::string> ChatroomControlMessage::get_member_list() const
{
	return this->_member_list;
}

std::string ChatroomControlMessage::get_room_name() const
{
	return this->_room_name;
}

std::string ChatroomControlMessage::get_icon_url() const
{
	return this->_icon_url;
}


ChatroomControlMessage::RoomAction ChatroomControlMessage::get_room_action() const
{
	return this->_room_action;
}


std::string ChatroomControlMessage::get_message_content() const
{
    return this->_message_content;
}


bool ChatroomControlMessage::validate_payload(const json &message) const
{
	RoomAction room_action = RoomAction::CREATE_ROOM;
	try
	{
		// payloadキーの存在をチェック
		if (!message.contains("payload"))
		{
			throw std::runtime_error("Missing key: payload");
		}

		const auto& payload = message["payload"];
		static const std::vector<std::string> required_keys = {
			"sender_uid", 
			"room_id",
			"room_member_list",
			"room_name",
			"icon_url",
			"room_action"
		};

		for(const auto& key : required_keys)
		{
			if (!payload.contains(key)) {
				throw std::runtime_error("Missing key: " + key);
			}
		}

		if(!payload["sender_uid"].is_string())
		{
			throw std::runtime_error("Invalid type for sender_uid");
		}
		if(!payload["room_id"].is_string())
		{
			throw std::runtime_error("Invalid type for room_id");
		}
		if(!payload["room_member_list"].is_array())
		{
			throw std::runtime_error("Invalid type for room_member_list");
		}
		if(!payload["room_name"].is_string())
		{
			throw std::runtime_error("Invalid type for room_name");
		}
		if(!payload["icon_url"].is_string())
		{
			throw std::runtime_error("Invalid type for icon_url");
		}
		if(!payload["room_action"].is_string())
		{
			throw std::runtime_error("Invalid type for room_action");
		}
		return true;
	}
	catch(const std::runtime_error& re)
	{
		LOG_ERROR("Runtime error : {}", re.what());
		return false;
	}
	catch(const std::exception& e)
	{
		LOG_ERROR("Error deserializing ChatRoomControl message: {}", e.what());
		return false;
	}
	
}

std::unique_ptr<MessageBase> ChatroomControlMessage::create_instance(const json &message) const
{
	std::string sender_uid;
	std::string room_id;
	std::vector<std::string> member_list;
	std::string room_name;
	std::string icon_url;
	RoomAction room_action;

	try
    {	
		auto common_fields = this->parse_common_fields(message);
        std::string sender_uid = message["payload"]["sender_uid"].get<std::string>();
        std::string room_id = message["payload"]["room_id"].get<std::string>();
        std::vector<std::string> member_list = message["payload"]["room_member_list"].get<std::vector<std::string>>();
        std::string room_name = message["payload"]["room_name"].get<std::string>();
        std::string icon_url = message["payload"]["icon_url"].get<std::string>();   
        RoomAction room_action = parse_room_action(message["payload"]["room_action"].get<std::string>());
        std::string message_content = message.dump();

        return std::make_unique<ChatroomControlMessage>(
			common_fields,
            MessageIDGenerator::generate_message_id(),
            sender_uid,
            room_id,
            member_list,
            room_name,
            icon_url,
            room_action,
            message_content
        );
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("Error creating ChatRoomControlMessage instance: {}", e.what());
        return nullptr;
	}
}

bool ChatroomControlMessage::validate_value() const
{
	if(is_valid_room_id(this->_room_id, this->_room_action) == false ||
	   is_valid_room_name(this->_room_name) == false ||
	   is_valid_icon_url(this->_icon_url) == false){
		LOG_ERROR("ChatRoomControlMessage value validation failed.");
    	return false;
	}else{
		spdlog::debug("ChatRoomControlMessage value validation succeeded.");
		return true;
	}
	
}

ChatroomControlMessage::RoomAction ChatroomControlMessage::parse_room_action(const std::string &action_str) const
{
    if (action_str == "CREATE_ROOM") return RoomAction::CREATE_ROOM;
    if (action_str == "LEAVE_ROOM") return RoomAction::LEAVE_ROOM;
    if (action_str == "ADD_MEMBER") return RoomAction::ADD_MEMBER;
    if (action_str == "REMOVE_MEMBER") return RoomAction::REMOVE_MEMBER;
    if (action_str == "ROOM_NAME_CHANGE") return RoomAction::ROOM_NAME_CHANGE;
    if (action_str == "ROOM_ICON_CHANGE") return RoomAction::ROOM_ICON_CHANGE;
	if (action_str == "DELETE_ROOM") return RoomAction::DELETE_ROOM;
        
    throw std::runtime_error("Unknown room action: " + action_str);
}

bool ChatroomControlMessage::is_valid_room_id(const std::string &room_id, RoomAction room_action) const
{
	if(room_action == RoomAction::CREATE_ROOM) {
		// ルーム作成の場合、room_idは空文字列であることを許容
		if (room_id.empty()) {
			return true;
		} else {
			LOG_ERROR("For CREATE_ROOM action, room_id should be empty. Provided room_id size: {}", room_id.size());
			return false;
		}
	}
	else
	{
		// その他のアクションの場合、room_idは32文字の16進数文字列であることを期待
		if (room_id.size() != 32)
		{
			LOG_ERROR("dst chatroom id is invalid. dst chatroom id byte size : {}", room_id.size());
			return false;
		}
	}

	return true;
}

bool ChatroomControlMessage::is_valid_room_name(const std::string &room_name) const
{
	if (room_name.empty() || room_name.size() > 100) {
		LOG_ERROR("dst chatroom name is invalid. dst chatroom name size : {}", room_name.size());
		return false;
	}
	return true;
}

bool ChatroomControlMessage::is_valid_icon_url(const std::string &icon_url) const
{
	if (icon_url.empty() || icon_url.size() > 200) {
		LOG_ERROR("dst chatroom icon url is invalid. dst chatroom icon url size : {}", icon_url.size());
		return false;
	}
	return true;
}
