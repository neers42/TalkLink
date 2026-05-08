#include "Domain/Entity/Messages/HeartBeatMessage.hpp"


HeartBeatMessage::HeartBeatMessage(){}
HeartBeatMessage::HeartBeatMessage(const MessageBase::CommonField& common_fields, const std::string &uid)
    : MessageBase(common_fields, MessageBase::MessageType::HEARTBEAT_MESSAGE), _uid{uid}
{
}

HeartBeatMessage::~HeartBeatMessage()
{
}


std::unique_ptr<MessageBase> HeartBeatMessage::create_instance(const json &message) const
{
	try
	{
		std::string uid = message["payload"]["uid"];
		return std::make_unique<HeartBeatMessage>(this->parse_common_fields(message), uid);
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
			"uid"
		};
		for (const auto& key : required_keys)
		{
			if (!payload.contains(key))
			{
				throw std::runtime_error("Missing key: " + key);
			}
		}
		std::string uid = payload["uid"];
		if (uid.size() != 28)
		{
			throw std::runtime_error("Invalid user ID size");
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
	if (this->_uid.size() == 28) {
		return true;
	}
    return false;
}
