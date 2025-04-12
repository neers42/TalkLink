#include "MessageType003.h"

MessageType003::MessageType003()
	:BaseMessage(), _chat_type{ChatType::Group}, _room_id{""}
{
}

MessageType003::MessageType003(BaseMessage::ContentType c_type, const std::string& mt, const uint64_t m_id, ChatType tt,
	const std::string& room_id, const std::string& receiver_id, const std::string& sender_id, 
	const std::string& send_time, const std::string& content)
	//BaseMessage::ContentType c_type, const std::string& mt, const std::string& dst_id, const std::string& src_id, const std::string& content, const uint64_t m_id, ChatType tt, const std::string& dst_chatroom_id)
	:BaseMessage::BaseMessage(c_type, mt, m_id),
	_room_id{room_id},
	_receiver_id{receiver_id},
	_sender_id{sender_id},
	_send_time{send_time},
	_content{ content },
	_chat_type{tt}
{
}

MessageType003::MessageType003(int c_type, 
	const std::string& mt,
	const uint64_t m_id,
	int chat_type,
	const std::string& room_id, 
	const std::string& receiver_id, const std::string& sender_id, 
	const std::string& send_time, const std::string& content)	
	//int c_type, const std::string& mt, const std::string& dst_id, const std::string& src_id, const std::string& content, const uint64_t m_id, int trans_type, const std::string& dst_chatroom_id)
	:BaseMessage::BaseMessage(exchange_content_type_from_int(c_type), mt, m_id),
	_chat_type{exchange_trans_type_from_int(chat_type)},
	_receiver_id{receiver_id},
	_sender_id{sender_id},
	_send_time{send_time},
	_content{content},
	_room_id{room_id}
{
}


MessageType003::~MessageType003()
{
}

MessageType003::ChatType MessageType003::get_chat_type()
{
	return this->_chat_type;
}

int MessageType003::convert_chat_type_to_int()
{
	switch (this->get_chat_type())
	{
	case ChatType::Group:
		return 0;
		break;
	case ChatType::P2P:
		return 1;
		break;
	default:
		return -1;
		break;
	}

}

void MessageType003::set_chat_type(ChatType tt)
{
	this->_chat_type = tt;
}

void MessageType003::set_chat_type_from_int(const int value)
{
	switch (value)
	{
	case 0:
		this->_chat_type = ChatType::Group;
		break;
	case 1:
		this->_chat_type = ChatType::P2P;
		break;
	default:
		break;
	}
}

std::string MessageType003::get_receiver_id() const
{
	return this->_receiver_id;
}

void MessageType003::set_receiver_id(const std::string& dst_id)
{
	this->_receiver_id = dst_id;
}

std::string MessageType003::get_sender_id() const
{
	return this->_sender_id;
}

void MessageType003::set_sender_id(const std::string& src_id)
{
	this->_sender_id = src_id;
}

std::string MessageType003::get_send_time() const
{
	return this->_send_time;
}

std::string MessageType003::get_content() const
{
	return this->_content;
}

void MessageType003::set_content(const std::string& content)
{
	this->_content = content;
}

std::string MessageType003::get_room_id()
{
	return this->_room_id;
}

void MessageType003::set_room_id(const std::string& id)
{
	this->_room_id = id;
}

MessageType003::ChatType MessageType003::exchange_trans_type_from_int(int value)
{
	switch (value)
	{
	case 0:
		return ChatType::Group;
		break;
	case 1:
		return ChatType::P2P;
		break;
	default:
		break;
	}
}

bool MessageType003::is_valid()
{
	//グループチャットの場合
	if (this->get_chat_type() == MessageType003::ChatType::Group) {
		if (is_valid_mesasge_type(this->_message_type) == true &&
			is_valid_message_id(this->_message_id) == true &&
			is_valid_room_id(this->_room_id) == true &&
			is_valid_sender_id(this->_sender_id) == true &&
			is_valid_conent(this->_content) == true)
		{
			return true;
		}
	//P2Pの場合
	}else if (this->get_chat_type() == MessageType003::ChatType::P2P) {
		if (is_valid_mesasge_type(this->_message_type) == true &&
			is_valid_message_id(this->_message_id) == true &&
			is_valid_room_id(this->_room_id) == true &&
			is_valid_receiver_id(this->_receiver_id) == true &&
			is_valid_sender_id(this->_sender_id) == true &&
			is_valid_conent(this->_content) == true)
		{
			return true;
		}
	}
	return false;	
	
}

std::unique_ptr<MessageType003> MessageType003::serialize_message(const json& json_message)
{
	try
	{
		// 1. 必須キーがすべて存在するかチェック
		static const std::vector<std::string> required_keys = {
			"content_type", "message_type", "chat_type","room_id","receiver_id",
			"sender_id", "send_time", "content"
		};
		for(const auto& key : required_keys)
		{
			if (!json_message.contains(key)) {
				throw std::runtime_error("Missing key: " + key);
			}
		}
		// 2. 各フィールドの型をチェック
		if (!json_message["content_type"].is_string()) {
			throw std::runtime_error("Invalid type for 'content_type'");
		}
		if (!json_message["message_type"].is_string()) {
			throw std::runtime_error("Invalid type for 'message_type'");
		}
		if (!json_message["chat_type"].is_string()) {
			throw std::runtime_error("Invalid type for 'chat_type'");
		}
		if (!json_message["room_id"].is_string()) {
			throw std::runtime_error("Invalid type for 'room_id'");
		}
		if (!json_message["receiver_id"].is_string()) {
			throw std::runtime_error("Invalid type for 'receiver_id'");
		}
		if (!json_message["sender_id"].is_string()) {
			throw std::runtime_error("Invalid type for 'sender_id'");
		}
		if (!json_message["send_time"].is_string()) {
			throw std::runtime_error("Invalid type for 'send_time'");
		}
		if (!json_message["content"].is_string()) {
			throw std::runtime_error("Invalid type for 'content'");
		}
		// 3. 各フィールドを読み取る
		std::string content_type_str = json_message["content_type"];
		int content_type;
		if (content_type_str == "TEXT") {
			content_type = 0;
		}
		else if (content_type_str == "IMAGE") {
			content_type = 1;
		}
		else if (content_type_str == "VIDEO") {
			content_type = 2;
		}
		else if (content_type_str == "NOTIFICATION") {
			content_type = 3;
		}
		else {
			throw std::runtime_error("Invalid content type");
		}
		std::string message_type = json_message["message_type"];
		std::string chat_type_str = json_message["chat_type"];
		int chat_type;
		if (chat_type_str == "Group") {
			chat_type = 0;
		}
		else if (chat_type_str == "P2P") {
			chat_type = 1;
		}
		else {
			throw std::runtime_error("Invalid chat type");
		}
		std::string room_id = json_message["room_id"];
		std::string receiver_id = json_message["receiver_id"];
		std::string sender_id = json_message["sender_id"];
		std::string send_time = json_message["send_time"];
		std::string content = json_message["content"];
		uint64_t message_id = MessageIDGenerator::generate_message_id();

		return std::make_unique<MessageType003>(
			static_cast<int>(content_type),
			message_type,
			message_id,
			static_cast<int>(chat_type),
			room_id,
			receiver_id,
			sender_id,
			send_time,
			content
		);
			

	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error deserializing MessageType003: {}", e.what());
		return nullptr;
	}
}

bool MessageType003::is_valid_room_id(const std::string& room_id)
{
	//dst_idが28byteではない場合
	if (room_id.size() != 26) {
		LOG_ERROR("dst chatroom id is invalid. dst chatroom id byte size : {:d}", room_id.size());
		return false;
	}
	return true;
}

bool MessageType003::is_valid_receiver_id(const std::string& receiver)
{
	//dst_idが28byteではない場合
	if (receiver.size() != 28) {
		LOG_ERROR("receiver id is invalid. dst client id byte size : {:d}", receiver.size());
		return false;
	}
	return true;
}

bool MessageType003::is_valid_sender_id(const std::string& sender_id)
{
	//dst_idが28byteではない場合
	if (sender_id.size() != 28) {
		LOG_ERROR("sender id is invalid. src client id byte size : {:d}", sender_id.size());
		return false;
	}
	return true;
}

bool MessageType003::is_valid_conent(const std::string& content)
{
	//dst_idが28byteではない場合
	if (content.size() <= 0) {
		LOG_ERROR("content is invalid. content byte size : {:d}", content.size());
		return false;
	}
	return true;
}

