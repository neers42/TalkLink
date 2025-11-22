#include "Messages/ChatMessage.hpp"

ChatMessage::ChatMessage() {}

ChatMessage::ChatMessage(MessageBase::ContentType c_type, 
						const std::string &mt, 
						const uint64_t m_id,
						const std::string &room_id, 
						const std::vector<std::string> &mention_ids, 
						const std::string &sender_id, 
						const std::string &content)
    : MessageBase(c_type, mt, m_id), 
	_room_id{room_id}, 
	_mention_ids{mention_ids}, 
	_sender_id{sender_id}, 
	_content{content}
{
}

ChatMessage::ChatMessage(int c_type, 
	const std::string& mt,
	const uint64_t m_id,
	const std::string& room_id, 
	const std::vector<std::string>& mention_ids, 
	const std::string& sender_id, 
	const std::string& content)	
	:MessageBase(static_cast<MessageBase::ContentType>(c_type), mt, m_id),
	_room_id{room_id},
	_mention_ids{mention_ids},
	_sender_id{sender_id},
	_content{content}
{
}

ChatMessage::ChatMessage(int c_type, 
	const std::string &mt, 
	const uint64_t m_id, 
	const std::string &time_stamp, 
	const std::string &room_id, 
	const std::vector<std::string> &mention_ids, 
	const std::string &sender_id, 
	const std::string &content)
	:MessageBase(static_cast<MessageBase::ContentType>(c_type), mt, m_id, time_stamp),
	_room_id{room_id},
	_mention_ids{mention_ids},
	_sender_id{sender_id},
	_content{content}
{
}
ChatMessage::~ChatMessage()
{
}


const std::vector<std::string>& ChatMessage::get_mention_ids() const
{
	return this->_mention_ids;
}

void ChatMessage::set_mention_ids(const std::vector<std::string>& mention_ids)
{
	this->_mention_ids = mention_ids;
}

std::string ChatMessage::get_sender_cid() const
{
	return this->_sender_id;
}

void ChatMessage::set_sender_id(const std::string& src_id)
{
	this->_sender_id = src_id;
}


std::string ChatMessage::get_content() const
{
	return this->_content;
}

void ChatMessage::set_content(const std::string& content)
{
	this->_content = content;
}

std::string ChatMessage::get_room_id()
{
	return this->_room_id;
}

void ChatMessage::set_room_id(const std::string& id)
{
	this->_room_id = id;
}


std::unique_ptr<MessageBase> ChatMessage::create_instance(const json &message) const
{
    try
	{
		// 各フィールドを読み取る
		auto common_fields = this->parse_common_fields(message);
		MessageBase::ContentType content_type = std::get<0>(common_fields);
		std::string message_type = std::get<1>(common_fields);
		std::string room_id = message["room_id"];
		// mention_ids を vector に変換
		std::vector<std::string> mention_ids;
		for (const auto& item : message["mention_ids"]) {
			mention_ids.push_back(item.get<std::string>());
		}
		std::string sender_cid = message["sender_cid"];
		std::string content = message.dump();
		uint64_t message_id = MessageIDGenerator::generate_message_id();
		std::string time_stamp;
		if(message.contains("time_stamp") == true){
			time_stamp = message["time_stamp"];
		}

		return std::make_unique<ChatMessage>(
			static_cast<int>(content_type),
			message_type,
			message_id,
			time_stamp,
			room_id,
			mention_ids,
			sender_cid,
			content
		);
			

	}
	catch (const std::runtime_error& re)
	{
		LOG_ERROR("Runtime error deserializing ChatMessage: {}", re.what());
		return nullptr;
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error deserializing ChatMessage: {}", e.what());
		return nullptr;
	}
}
bool ChatMessage::validate_specific_fields(const json &message) const
{
	try
	{
		// 必須キーがすべて存在するかチェック
		static const std::vector<std::string> required_keys = {
			"room_id",
			"mention_ids",
			"sender_cid"};
		for (const auto &key : required_keys)
		{
			if (!message.contains(key))
			{
				throw std::runtime_error("Missing key: " + key);
			}
		}

		// 各フィールドの型をチェック
		if (!message["room_id"].is_string())
		{
			throw std::runtime_error("Invalid type for 'room_id'");
		}
		if (!message["sender_cid"].is_string())
		{
			throw std::runtime_error("Invalid type for 'sender_cid'");
		}
		if (!message["mention_ids"].is_array())
		{
			throw std::runtime_error("Invalid type for 'mention_ids'");
		}
		else
		{
			// mention_idsの各要素が文字列であることを確認
			for (const auto &item : message["mention_ids"])
			{
				if (!item.is_string())
				{
					throw std::runtime_error("Invalid type in 'mention_ids' array");
				}
			}
		}
	}
	catch(const std::runtime_error& re)
	{
		LOG_ERROR("Runtime error : {}", re.what());
		return false;
	}
	catch(const std::exception& e)
	{
		LOG_ERROR("Exception : {}", e.what());
		return false;
	}
	
    
	return true;
}
bool ChatMessage::validate_value() const
{
	try
	{
		if (is_valid_room_id(this->_room_id) == true &&
			is_valid_mention_ids(this->_mention_ids) == true &&
			is_valid_sender_id(this->_sender_id) == true &&
			is_valid_content(this->_content) == true)
		{
			spdlog::debug("ChatMessage value validation succeeded.");
			return true;
		}else{
			throw std::runtime_error("ChatMessage value validation failed.");
		}
	}
	catch(const std::exception& e)
	{
		LOG_ERROR("Value validation failed: {}", e.what());
		return false;
	}
	
    
}
bool ChatMessage::is_valid_room_id(const std::string &room_id) const
{
	//dst_idが32byteではない場合
	if (room_id.size() != 32) {
		LOG_ERROR("dst chatroom id is invalid. dst chatroom id byte size : {}", room_id.size());
		return false;
	}
	return true;
}

bool ChatMessage::is_valid_mention_ids(const std::vector<std::string>& mention_ids) const
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

bool ChatMessage::is_valid_sender_id(const std::string& sender_id) const
{
	//dst_idが28byteではない場合
	if (sender_id.size() != 28) {
		LOG_ERROR("sender id is invalid. src client id byte size : {}", sender_id.size());
		return false;
	}
	return true;
}

bool ChatMessage::is_valid_content(const std::string& content) const
{
	if (content.size() <= 0) {
		LOG_ERROR("content is invalid. content byte size : {}", content.size());
		return false;
	}
	return true;
}