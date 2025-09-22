#include "Messages/HeartBeatMessage.hpp"


HeartBeatMessage::HeartBeatMessage(const std::string& client_id)
	:BaseMessage(BaseMessage::ContentType::HEARTBEAT, "000"), _client_id{client_id}
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



