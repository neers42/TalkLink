#include "Domain/Entity/Messages/ChatMessage.hpp"

ChatMessage::ChatMessage() 
	: MessageBase(MessageBase::MessageType::CHAT_MESSAGE),_message_id(0), _room_id(""), _mention_ids(), _sender_cid(""), _content("")
{
}

ChatMessage::ChatMessage(const MessageBase::CommonField& common_fields,
						const uint64_t m_id,
						const std::string &room_id, 
						const std::vector<std::string> &mention_ids, 
						const std::string &sender_id, 
						const std::string &content)
    : MessageBase(common_fields, MessageBase::MessageType::CHAT_MESSAGE),
	_message_id{m_id}, 
	_room_id{room_id}, 
	_mention_ids{mention_ids}, 
	_sender_cid{sender_id}, 
	_content{content}
{
}


ChatMessage::~ChatMessage()
{
}

uint64_t ChatMessage::get_message_id() const
{
	return this->_message_id;
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
	return this->_sender_cid;
}

void ChatMessage::set_sender_cid(const std::string& sender_cid)
{
	this->_sender_cid = sender_cid;
}


std::string ChatMessage::get_content() const
{
	return this->_content;
}

void ChatMessage::set_content(const std::string& content)
{
	this->_content = content;
}

std::string ChatMessage::get_room_id() const
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
		std::string room_id = message["payload"]["room_id"];
		// mention_ids を vector に変換
		std::vector<std::string> mention_ids;
		for (const auto& item : message["payload"]["mention_ids"]) {
			mention_ids.push_back(item.get<std::string>());
		}
		std::string sender_cid = message["payload"]["sender_cid"];

		//メッセージの実際の内容のみ抽出
		std::string content = message["payload"]["content"];
		uint64_t message_id = MessageIDGenerator::generate_message_id();

		return std::make_unique<ChatMessage>(
			common_fields,
			message_id,
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
bool ChatMessage::validate_payload(const json &message) const
{
	try
	{
		// payloadキーの存在をチェック
		if (!message.contains("payload"))
		{
			throw std::runtime_error("Missing key: payload");
		}

		const auto& payload = message["payload"];

		// 必須キーがすべて存在するかチェック
		static const std::vector<std::string> required_keys = {
			"room_id",
			"mention_ids",
			"sender_cid",
			"content"};
		for (const auto &key : required_keys)
		{
			if (!payload.contains(key))
			{
				throw std::runtime_error("Missing key in payload: " + key);
			}
		}

		// 各フィールドの型をチェック
		if (!payload["room_id"].is_string())
		{
			throw std::runtime_error("Invalid type for 'room_id'");
		}
		if (!payload["sender_cid"].is_string())
		{
			throw std::runtime_error("Invalid type for 'sender_cid'");
		}
		if (!payload["content"].is_string())
		{
			throw std::runtime_error("Invalid type for 'content'");
		}
		if (!payload["mention_ids"].is_array())
		{
			throw std::runtime_error("Invalid type for 'mention_ids'");
		}
		else
		{
			// mention_idsの各要素が文字列であることを確認
			for (const auto &item : payload["mention_ids"])
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
			is_valid_sender_id(this->_sender_cid) == true &&
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