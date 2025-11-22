#include "Messages/ClientSessionResponseMessage.hpp"


ClientSessionResponseMessage::ClientSessionResponseMessage(const std::string& client_id, bool verify_result, const std::string& error_message, const json& unsend_messages)
	:MessageBase(ContentType::NOTIFICATION, "SessionResponse"),
	_client_id{ client_id },
	_verify_result{ verify_result },
	_error_message{ error_message },
	_unsend_messages{ unsend_messages }
{
}


ClientSessionResponseMessage::~ClientSessionResponseMessage()
{
}


json ClientSessionResponseMessage::serialize_specific_message_fields() const
{
	json specific_fields;
	try
	{
		if (validate_specific_message_fields() == false)
		{
			throw std::runtime_error("ClientSessionResponseMessage specific fields validation failed during serialization.");
		}
		specific_fields["client_id"] = this->_client_id;
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

	}
	catch (const std::exception &e)
	{
		LOG_ERROR("Error serializing ClientSessionResponseMessage specific fields: {}", e.what());
		return json(); // 空のJSONオブジェクトを返す
	}
	return specific_fields;
}

bool ClientSessionResponseMessage::validate_specific_message_fields() const
{
	try
	{
		//_verify_resultがtreueの場合、_client_idが空でないことを確認
		if (this->_verify_result == true)
		{
			if (this->_client_id.empty())
			{
				throw std::runtime_error("[ClientSessionResponse]Client ID is empty despite verification success.");
			}
		}
		if (this->_error_message.empty())
		{
			throw std::runtime_error("[ClientSessionResponse]Error message is empty.");
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

