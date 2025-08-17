#include "Network/MessageType004.hpp"


MessageType004::MessageType004(BaseMessage::ContentType c_type, const std::string& mt, const uint64_t m_id,
	const std::string& room_id, const std::vector<std::string>& mention_ids, const std::string& sender_id, 
	const std::string& send_time, const std::string& content)
	:BaseMessage::BaseMessage(c_type, mt, m_id),
	_room_id{room_id},
	_mention_ids{mention_ids},
	_sender_id{sender_id},
	_send_time{send_time},
	_content{ content }
{
}

MessageType004::MessageType004(int c_type, 
	const std::string& mt,
	const uint64_t m_id,
	const std::string& room_id, 
	const std::vector<std::string>& mention_ids, const std::string& sender_id, 
	const std::string& send_time, const std::string& content)	
	:BaseMessage::BaseMessage(exchange_content_type_from_int(c_type), mt, m_id),
	_mention_ids{mention_ids},
	_sender_id{sender_id},
	_send_time{send_time},
	_content{content},
	_room_id{room_id}
{
}


MessageType004::~MessageType004()
{
}


const std::vector<std::string>& MessageType004::get_mention_ids() const
{
	return this->_mention_ids;
}

void MessageType004::set_mention_ids(const std::vector<std::string>& mention_ids)
{
	this->_mention_ids = mention_ids;
}

std::string MessageType004::get_sender_cid() const
{
	return this->_sender_id;
}

void MessageType004::set_sender_id(const std::string& src_id)
{
	this->_sender_id = src_id;
}

std::string MessageType004::get_send_time() const
{
	return this->_send_time;
}

std::string MessageType004::get_content() const
{
	return this->_content;
}

void MessageType004::set_content(const std::string& content)
{
	this->_content = content;
}

std::string MessageType004::get_room_id()
{
	return this->_room_id;
}

void MessageType004::set_room_id(const std::string& id)
{
	this->_room_id = id;
}


bool MessageType004::is_valid()
{
	if (is_valid_mesasge_type(this->_message_type) == true &&
			is_valid_message_id(this->_message_id) == true &&
			is_valid_room_id(this->_room_id) == true &&
			is_valid_mention_ids(this->_mention_ids) == true &&
			is_valid_sender_id(this->_sender_id) == true &&
			is_valid_conent(this->_content) == true)
		{
			return true;
		}else{
			return false;
		}
}

std::unique_ptr<MessageType004> MessageType004::serialize_message(const json& json_message)
{
	try
	{
		// 1. 必須キーがすべて存在するかチェック
		static const std::vector<std::string> required_keys = {
			"content_type", "message_type", "room_id","mention_ids",
			"sender_cid", "send_time"
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
		if (!json_message["room_id"].is_string()) {
			throw std::runtime_error("Invalid type for 'room_id'");
		}
		if (!json_message["mention_ids"].is_array()) {
			throw std::runtime_error("Invalid type for 'mention_ids'");
		}else{
			// mention_idsの各要素が文字列であることを確認
			for (const auto& item : json_message["mention_ids"]) {
				if (!item.is_string()) {
					throw std::runtime_error("Invalid type in 'mention_ids' array");
				}
			}
		}
		if (!json_message["sender_cid"].is_string()) {
			throw std::runtime_error("Invalid type for 'sender_cid'");
		}
		if (!json_message["send_time"].is_string()) {
			throw std::runtime_error("Invalid type for 'send_time'");
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
		else if (content_type_str == "HEARTBEAT") {
			content_type = 4;
		}
		else {
			throw std::runtime_error("Invalid content type");
		}
		std::string message_type = json_message["message_type"];
		std::string room_id = json_message["room_id"];
		// mention_ids を vector に変換
		std::vector<std::string> mention_ids;
		for (const auto& item : json_message["mention_ids"]) {
			mention_ids.push_back(item.get<std::string>());
		}
		std::string sender_cid = json_message["sender_cid"];
		std::string send_time = json_message["send_time"];
		std::string content = json_message.dump();
		uint64_t message_id = MessageIDGenerator::generate_message_id();

		return std::make_unique<MessageType004>(
			static_cast<int>(content_type),
			message_type,
			message_id,
			room_id,
			mention_ids,
			sender_cid,
			send_time,
			content
		);
			

	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error deserializing MessageType004: {}", e.what());
		return nullptr;
	}
}

bool MessageType004::is_valid_room_id(const std::string& room_id)
{
	//dst_idが32byteではない場合
	if (room_id.size() != 32) {
		LOG_ERROR("dst chatroom id is invalid. dst chatroom id byte size : {}", room_id.size());
		return false;
	}
	return true;
}

bool MessageType004::is_valid_mention_ids(const std::vector<std::string>& mention_ids)
{
	// mention_idsが空でないことを確認
	if (mention_ids.empty()) {
		return true; // 空のメンションIDは許容する
	}
	// 各メンションIDが28byteであることを確認
	for (const auto& mention_id : mention_ids) {
		if (mention_id.size() != 28) {
			LOG_ERROR("mention id is invalid. mention id byte size : {}", mention_id.size());
			return false;
		}
	}
	return true;
}

bool MessageType004::is_valid_sender_id(const std::string& sender_id)
{
	//dst_idが28byteではない場合
	if (sender_id.size() != 28) {
		LOG_ERROR("sender id is invalid. src client id byte size : {}", sender_id.size());
		return false;
	}
	return true;
}

bool MessageType004::is_valid_conent(const std::string& content)
{
	if (content.size() <= 0) {
		LOG_ERROR("content is invalid. content byte size : {}", content.size());
		return false;
	}
	return true;
}

