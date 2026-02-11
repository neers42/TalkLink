#include "Messages/ChatRoomControlMessage.hpp"

ChatRoomControlMessage::ChatRoomControlMessage()
	:MessageBase(MessageBase::MessageType::CHATROOM_CONTROL),
	 _message_id{0},
	 _sender_cid{""},
	 _room_id{""},
	 _add_member_list{},
	 _remove_member_list{},
	 _room_name{""},
	 _room_image_url{""},
	 _room_action{RoomAction::CREATE_ROOM},
	 _message_content{""}
{
} 

ChatRoomControlMessage::ChatRoomControlMessage(
	const MessageBase::CommonField& common_fields,
    const uint64_t mid,
    const std::string &sender_cid,
    const std::string &room_id,
    const std::vector<std::string> &add_member_list,
    const std::vector<std::string> &remove_member_list,
    const std::string &room_name,
    const std::string &room_image,
    const RoomAction &room_action,
    const std::string &message_content) 
	: MessageBase(common_fields, MessageBase::MessageType::CHATROOM_CONTROL),
	_message_id(mid),
	_sender_cid(sender_cid),
    _room_id(room_id),
    _add_member_list(add_member_list),
    _remove_member_list(remove_member_list),
    _room_name(room_name),
    _room_image_url(room_image),
    _room_action(room_action),
	_message_content(message_content)
{
}

ChatRoomControlMessage::~ChatRoomControlMessage()
{
}

uint64_t ChatRoomControlMessage::get_message_id() const
{
	return this->_message_id;
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


std::string ChatRoomControlMessage::get_message_content() const
{
    return this->_message_content;
}


bool ChatRoomControlMessage::validate_payload(const json &message) const
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
			"sender_cid", 
			"room_id",
			"room_add_member_list",
			"room_remove_member_list",
			"room_name",
			"room_image_url",
			"room_action"
		};

		for(const auto& key : required_keys)
		{
			if (!payload.contains(key)) {
				throw std::runtime_error("Missing key: " + key);
			}
		}

		if(!payload["sender_cid"].is_string())
		{
			throw std::runtime_error("Invalid type for sender_cid");
		}
		if(!payload["room_id"].is_string())
		{
			throw std::runtime_error("Invalid type for room_id");
		}
		if(!payload["room_add_member_list"].is_array())
		{
			throw std::runtime_error("Invalid type for room_add_member_list");
		}
		if(!payload["room_remove_member_list"].is_array())
		{
			throw std::runtime_error("Invalid type for room_remove_member_list");
		}
		if(!payload["room_name"].is_string())
		{
			throw std::runtime_error("Invalid type for room_name");
		}
		if(!payload["room_image_url"].is_string())
		{
			throw std::runtime_error("Invalid type for room_image_url");
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

std::unique_ptr<MessageBase> ChatRoomControlMessage::create_instance(const json &message) const
{
	std::string sender_cid;
	std::string room_id;
	std::vector<std::string> add_member_list;
	std::vector<std::string> remove_member_list;
	std::string room_name;
	std::string room_image_url;
	RoomAction room_action;

	try
    {	
		auto common_fields = this->parse_common_fields(message);
        std::string sender_cid = message["payload"]["sender_cid"].get<std::string>();
        std::string room_id = message["payload"]["room_id"].get<std::string>();
        std::vector<std::string> add_member_list = message["payload"]["room_add_member_list"].get<std::vector<std::string>>();
        std::vector<std::string> remove_member_list = message["payload"]["room_remove_member_list"].get<std::vector<std::string>>();
        std::string room_name = message["payload"]["room_name"].get<std::string>();
        std::string room_image_url = message["payload"]["room_image_url"].get<std::string>();   
        RoomAction room_action = parse_room_action(message["payload"]["room_action"].get<std::string>());
        std::string message_content = message.dump();

        return std::make_unique<ChatRoomControlMessage>(
			common_fields,
            MessageIDGenerator::generate_message_id(),
            sender_cid,
            room_id,
            add_member_list,
            remove_member_list,
            room_name,
            room_image_url,
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

bool ChatRoomControlMessage::validate_value() const
{
	if(is_valid_room_id(this->_room_id, this->_room_action) == false ||
	   is_valid_room_name(this->_room_name) == false ||
	   is_valid_room_image_url(this->_room_image_url) == false){
		LOG_ERROR("ChatRoomControlMessage value validation failed.");
    	return false;
	}else{
		spdlog::debug("ChatRoomControlMessage value validation succeeded.");
		return true;
	}
	
}

ChatRoomControlMessage::RoomAction ChatRoomControlMessage::parse_room_action(const std::string &action_str) const
{
    if (action_str == "CREATE_ROOM") return RoomAction::CREATE_ROOM;
    if (action_str == "LEAVE_ROOM") return RoomAction::LEAVE_ROOM;
    if (action_str == "ADD_MEMBER") return RoomAction::ADD_MEMBER;
    if (action_str == "REMOVE_MEMBER") return RoomAction::REMOVE_MEMBER;
    if (action_str == "ROOM_NAME_CHANGE") return RoomAction::ROOM_NAME_CHANGE;
    if (action_str == "ROOM_ICON_CHANGE") return RoomAction::ROOM_ICON_CHANGE;
        
    throw std::runtime_error("Unknown room action: " + action_str);
}

bool ChatRoomControlMessage::is_valid_room_id(const std::string &room_id, RoomAction room_action) const
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

bool ChatRoomControlMessage::is_valid_room_name(const std::string &room_name) const
{
	if (room_name.empty() || room_name.size() > 100) {
		LOG_ERROR("dst chatroom name is invalid. dst chatroom name size : {}", room_name.size());
		return false;
	}
	return true;
}

bool ChatRoomControlMessage::is_valid_room_image_url(const std::string &room_image_url) const
{
	if (room_image_url.empty() || room_image_url.size() > 200) {
		LOG_ERROR("dst chatroom image url is invalid. dst chatroom image url size : {}", room_image_url.size());
		return false;
	}
	return true;
}
