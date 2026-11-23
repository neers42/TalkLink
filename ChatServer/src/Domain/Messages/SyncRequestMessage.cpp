#include "Domain/Messages/SyncRequestMessage.hpp"

SyncRequestMessage::SyncRequestMessage()
	: MessageBase(MessageBase::MessageType::SYNC_REQUEST), _room_id(""), _target_uid(""), _from_sequence(0)
{
}

SyncRequestMessage::SyncRequestMessage(
	const MessageBase::CommonField& common_fields,
	const std::string& room_id,
	const std::string& target_uid,
	const std::uint64_t from_sequence)
	: MessageBase(common_fields, MessageBase::MessageType::SYNC_REQUEST),
	_room_id{room_id},
	_target_uid{target_uid},
	_from_sequence{from_sequence}
{
}

SyncRequestMessage::~SyncRequestMessage()
{
}

std::string SyncRequestMessage::get_room_id() const
{
	return this->_room_id;
}

std::string SyncRequestMessage::get_target_uid() const
{
	return this->_target_uid;
}

std::uint64_t SyncRequestMessage::get_from_sequence() const
{
	return this->_from_sequence;
}

std::unique_ptr<MessageBase> SyncRequestMessage::create_instance(const json &message) const
{
	try
	{
		// 各フィールドを読み取る
		auto common_fields = this->parse_common_fields(message);
		std::string room_id = message["payload"]["room_id"];
		std::string target_uid = message["payload"]["target_uid"];
		std::uint64_t from_sequence = message["payload"]["from_sequence"].get<std::uint64_t>();

		return std::make_unique<SyncRequestMessage>(
			common_fields,
			room_id,
			target_uid,
			from_sequence
		);
	}
	catch (const std::runtime_error& re)
	{
		LOG_ERROR("[SyncRequestMessage] Runtime error deserializing SyncRequestMessage: {}", re.what());
		return nullptr;
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("[SyncRequestMessage] Error deserializing SyncRequestMessage: {}", e.what());
		return nullptr;
	}
}

bool SyncRequestMessage::validate_payload(const json &message) const
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
			"target_uid",
			"from_sequence"};
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
		if (!payload["target_uid"].is_string())
		{
			throw std::runtime_error("Invalid type for 'target_uid'");
		}
		if (!payload["from_sequence"].is_number_unsigned())
		{
			throw std::runtime_error("Invalid type for 'from_sequence'");
		}
	}
	catch(const std::runtime_error& re)
	{
		LOG_ERROR("[SyncRequestMessage] Runtime error : {}", re.what());
		return false;
	}
	catch(const std::exception& e)
	{
		LOG_ERROR("[SyncRequestMessage] Exception : {}", e.what());
		return false;
	}

	return true;
}

bool SyncRequestMessage::validate_value() const
{
	try
	{
		if (is_valid_room_id(this->_room_id) == true &&
			is_valid_target_uid(this->_target_uid) == true)
		{
			LOG_DEBUG("[SyncRequestMessage] SyncRequestMessage value validation succeeded.");
			return true;
		}else{
			throw std::runtime_error("SyncRequestMessage value validation failed.");
		}
	}
	catch(const std::exception& e)
	{
		LOG_ERROR("[SyncRequestMessage] Value validation failed: {}", e.what());
		return false;
	}
}

bool SyncRequestMessage::is_valid_room_id(const std::string &room_id) const
{
	// room_idはUUID(36)またはHEX(32)を許容
	if (room_id.size() != 36 && room_id.size() != 32) {
		LOG_ERROR("[SyncRequestMessage] room id is invalid. expected UUID(36) or HEX(32), actual size : {}", room_id.size());
		return false;
	}
	return true;
}

bool SyncRequestMessage::is_valid_target_uid(const std::string &target_uid) const
{
	// uidは28byteであることを確認
	if (target_uid.size() != 28) {
		LOG_ERROR("[SyncRequestMessage] target uid is invalid. target uid byte size : {}", target_uid.size());
		return false;
	}
	return true;
}
