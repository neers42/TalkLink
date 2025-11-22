#include "Messages/ChatRoomControlMessage.hpp"

ChatRoomControlMessage::ChatRoomControlMessage() {

} 

ChatRoomControlMessage::ChatRoomControlMessage(
    const uint64_t mid,
    const std::string &time_stamp,
    const std::string &sender_cid,
    const std::string &room_id,
    const std::vector<std::string> &add_member_list,
    const std::vector<std::string> &remove_member_list,
    const std::string &room_name,
    const std::string &room_image,
    const RoomAction &room_action,
    const std::string &message_content) : MessageBase(MessageBase::ContentType::NOTIFICATION, "ChatRoomControl", mid, time_stamp),
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


bool ChatRoomControlMessage::validate_specific_fields(const json &message) const
{
	RoomAction room_action = RoomAction::CREATE_ROOM;
	try
	{
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
			if (!message.contains(key)) {
				throw std::runtime_error("Missing key: " + key);
			}
		}

		if(!message["sender_cid"].is_string())
		{
			throw std::runtime_error("Invalid type for sender_cid");
		}
		if(!message["room_id"].is_string())
		{
			throw std::runtime_error("Invalid type for room_id");
		}
		if(!message["room_add_member_list"].is_array())
		{
			throw std::runtime_error("Invalid type for room_add_member_list");
		}
		if(!message["room_remove_member_list"].is_array())
		{
			throw std::runtime_error("Invalid type for room_remove_member_list");
		}
		if(!message["room_name"].is_string())
		{
			throw std::runtime_error("Invalid type for room_name");
		}
		if(!message["room_image_url"].is_string())
		{
			throw std::runtime_error("Invalid type for room_image_url");
		}
		if(!message["room_action"].is_string())
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
		uint64_t message_id = MessageIDGenerator::generate_message_id();
        std::string time_stamp = message["time_stamp"].get<std::string>();
        std::string sender_cid = message["sender_cid"].get<std::string>();
        std::string room_id = message["room_id"].get<std::string>();
        std::vector<std::string> add_member_list = message["room_add_member_list"].get<std::vector<std::string>>();
        std::vector<std::string> remove_member_list = message["room_remove_member_list"].get<std::vector<std::string>>();
        std::string room_name = message["room_name"].get<std::string>();
        std::string room_image_url = message["room_image_url"].get<std::string>();   
        RoomAction room_action = parse_room_action(message["room_action"].get<std::string>());
        std::string message_content = message.dump();

        return std::make_unique<ChatRoomControlMessage>(
            message_id,
            time_stamp,
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
	if(is_valid_room_id(this->_room_id) == false ||
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

bool ChatRoomControlMessage::is_valid_room_id(const std::string &room_id) const
{
	if (room_id.size() != 32) {
		LOG_ERROR("dst chatroom id is invalid. dst chatroom id byte size : {}", room_id.size());
		return false;
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
