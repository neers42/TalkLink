#include "Messages/HeartBeatMessage.hpp"


HeartBeatMessage::HeartBeatMessage(){}
HeartBeatMessage::HeartBeatMessage(const std::string &client_id)
    : MessageBase(MessageBase::ContentType::HEARTBEAT, "HeartBeat"),
      _client_id{client_id}
{
}

HeartBeatMessage::~HeartBeatMessage()
{
}

std::unique_ptr<HeartBeatMessage> HeartBeatMessage::serialize_message(const json& json_message)
{
	try
	{
		static const std::vector<std::string> required_keys = {
			"content_type", "message_type", "client_id"
		};
		for (const auto& key : required_keys)
		{
			if (!json_message.contains(key))
			{
				throw std::runtime_error("Missing key: " + key);
			}

		}
		if (!json_message["content_type"].is_string())
		{
			throw std::runtime_error("Invalid type for 'content_type'");
		}
		if (!json_message["message_type"].is_string())
		{
			throw std::runtime_error("Invalid type for 'message_type'");
		}
		if (!json_message["client_id"].is_string())
		{
			throw std::runtime_error("Invalid type for 'client_id'");
		}
		std::string client_id = json_message["client_id"];
		if (client_id.size() != 28)
		{
			throw std::runtime_error("Invalid client ID size");
		}
		return std::make_unique<HeartBeatMessage>(client_id);

	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error deserializing MessageType000: {}", e.what());
		return nullptr;
	}
}

std::unique_ptr<MessageBase> HeartBeatMessage::create_instance(const json &message) const
{
	try
	{
		std::string client_id = message["client_id"];
		return std::make_unique<HeartBeatMessage>(client_id);
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

bool HeartBeatMessage::validate_specific_fields(const json &message) const
{
    try
	{
		static const std::vector<std::string> required_keys = {
			"client_id"
		};
		for (const auto& key : required_keys)
		{
			if (!message.contains(key))
			{
				throw std::runtime_error("Missing key: " + key);
			}
		}
		std::string client_id = message["client_id"];
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
