#include "Domain/Entity/Messages/SessionResponseMessage.hpp"


SessionResponseMessage::SessionResponseMessage(const MessageBase::CommonField& common_fields, const std::string& uid, bool verify_result, const std::string& error_message, const json& unsend_messages)
	:MessageBase(common_fields, MessageBase::MessageType::SESSION_RESPONSE),
	_uid{ uid },
	_verify_result{ verify_result },
	_error_message{ error_message },
	_unsend_messages( unsend_messages )
{
}


SessionResponseMessage::~SessionResponseMessage()
{
}


json SessionResponseMessage::serialize_payload() const
{
	json payload, specific_fields;
	try
	{
		if (validate_payload() == false)
		{
			throw std::runtime_error("SessionResponseMessage specific fields validation failed during serialization.");
		}
		specific_fields["uid"] = this->_uid;
		specific_fields["verify_result"] = this->_verify_result;
		specific_fields["error_reason"] = this->_error_message;
		// unreceived_messages 配列だけを代入
		if (this->_verify_result == true)
		{
			if (_unsend_messages.is_array())
            {
                // そのまま配列を代入
                specific_fields["unsend_messages"] = _unsend_messages;
            }
            else if (_unsend_messages.is_object() && _unsend_messages.contains("unreceived_messages"))
            {
                // オブジェクトの場合は "unreceived_messages" 配列を取り出す
                specific_fields["unsend_messages"] = _unsend_messages["unreceived_messages"];
            }
            else
            {
                specific_fields["unsend_messages"] = json::array();
            }
		}
		else
		{
			specific_fields["unsend_messages"] = json::array();
		}
		payload["payload"] = specific_fields;
		return payload;

	}
	catch (const std::exception &e)
	{
		LOG_ERROR("Error serializing SessionResponseMessage specific fields: {}", e.what());
		return json(); // 空のJSONオブジェクトを返す
	}
}

bool SessionResponseMessage::validate_payload() const
{
	try
	{
		//_verify_resultがtrueの場合、_uidが空でないことを確認
		if (this->_verify_result == true)
		{
			if (this->_uid.empty())
			{
				throw std::runtime_error("[SessionResponseMessage]User ID is empty despite verification success.");
			}
		}
		if (this->_error_message.empty())
		{
			throw std::runtime_error("[SessionResponseMessage]Error message is empty.");
		}
		return true;
	}
	catch (const std::runtime_error &re)
	{
		LOG_ERROR("Runtime error : {}", re.what());
		return false;
	}
	catch (const std::exception &e)
	{
		LOG_ERROR("Validate failed: {}", e.what());
		return false;
	}

	return false;
}

