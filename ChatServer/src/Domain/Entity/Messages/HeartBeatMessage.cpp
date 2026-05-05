#include "Domain/Entity/Messages/HeartBeatMessage.hpp"


HeartBeatMessage::HeartBeatMessage(){}
HeartBeatMessage::HeartBeatMessage(const MessageBase::CommonField& common_fields, const std::string &client_id)
    : MessageBase(common_fields, MessageBase::MessageType::HEARTBEAT_MESSAGE), _client_id{client_id}
{
}

HeartBeatMessage::~HeartBeatMessage()
{
}


std::unique_ptr<MessageBase> HeartBeatMessage::create_instance(const json &message) const
{
	try
	{
		std::string client_id = message["payload"]["client_id"];
		return std::make_unique<HeartBeatMessage>(this->parse_common_fields(message), client_id);
	}
	catch(const std::runtime_error& re)
	{
		LOG_ERROR("Runtime error : {}", re.what());
		return nullptr;
	}
	catch(const std::exception& e)
	{
		LOG_ERROR("Error creating HeartBeatMessage instance: {}", e.what());
		return nullptr;
	}
	
	
}

bool HeartBeatMessage::validate_payload(const json &message) const
{
    try
	{
		// payloadキーの存在をチェック
		if (!message.contains("payload"))
		{
			throw std::runtime_error("Missing key: payload");
		}

		const auto& payload = message["payload"];
		
		static const std::vector<std::string> required_keys = {
			"client_id"
		};
		for (const auto& key : required_keys)
		{
			if (!payload.contains(key))
			{
				throw std::runtime_error("Missing key: " + key);
			}
		}
		std::string client_id = payload["client_id"];
		if (client_id.size() != 28)
		{
			throw std::runtime_error("Invalid client ID size");
		}
		return true;
	}
	catch (const std::runtime_error& re)
	{
		LOG_ERROR("Runtime error : {}", re.what());
		return false;
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error creating HeartBeatMessage instance: {}", e.what());
		return false;
	}
}

bool HeartBeatMessage::validate_value() const
{
	if (this->_client_id.size() == 28) {
		return true;
	}
    return false;
}
