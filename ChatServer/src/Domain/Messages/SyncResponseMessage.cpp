#include "Domain/Messages/SyncResponseMessage.hpp"

SyncResponseMessage::SyncResponseMessage(
	const MessageBase::CommonField& common_fields,
	const std::string& target_room_id,
	const std::string& target_uid,
	const json& gap_messages)
	: MessageBase(common_fields, MessageBase::MessageType::SYNC_RESPONSE),
	_target_room_id{target_room_id},
	_target_uid{target_uid},
	_gap_messages(gap_messages)
{
}

SyncResponseMessage::~SyncResponseMessage()
{
}

std::string SyncResponseMessage::get_target_room_id() const
{
	return this->_target_room_id;
}

std::string SyncResponseMessage::get_target_uid() const
{
	return this->_target_uid;
}

json SyncResponseMessage::get_gap_messages() const
{
	return this->_gap_messages;
}

json SyncResponseMessage::serialize_payload() const
{
	json payload, specific_fields;
	try
	{
		if (validate_payload() == false)
		{
			throw std::runtime_error("SyncResponseMessage specific fields validation failed during serialization.");
		}
		specific_fields["room_id"] = this->_target_room_id;
		specific_fields["uid"] = this->_target_uid;
		specific_fields["gap_messages"] = this->_gap_messages;
		payload["payload"] = specific_fields;
		return payload;
	}
	catch (const std::runtime_error& re)
	{
		LOG_ERROR("Runtime error serializing SyncResponseMessage: {}", re.what());
		return json();
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error serializing SyncResponseMessage: {}", e.what());
		return json();
	}
}

bool SyncResponseMessage::validate_payload() const
{
	try
	{
		if (this->_target_room_id.empty())
		{
			throw std::runtime_error("SyncResponseMessage room_id is empty.");
		}
		if (this->_target_uid.empty())
		{
			throw std::runtime_error("SyncResponseMessage uid is empty.");
		}
		if (!this->_gap_messages.is_array())
		{
			throw std::runtime_error("SyncResponseMessage gap_messages is not an array.");
		}
		return true;
	}
	catch (const std::runtime_error& re)
	{
		LOG_ERROR("Runtime error validating SyncResponseMessage specific fields: {}", re.what());
		return false;
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error validating SyncResponseMessage specific fields: {}", e.what());
		return false;
	}
}
