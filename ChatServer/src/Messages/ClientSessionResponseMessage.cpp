#include "Messages/ClientSessionResponseMessage.hpp"


ClientSessionResponseMessage::ClientSessionResponseMessage(const std::string& client_id, bool verify_result)
	:BaseMessage(ContentType::NOTIFICATION, "SessionResponse"),
	_client_id{ client_id },
	_verify_result{ verify_result }
{
}


ClientSessionResponseMessage::~ClientSessionResponseMessage()
{
}

json ClientSessionResponseMessage::deserialize_message(const std::unique_ptr<ClientSessionResponseMessage>& session_response_message, const json& unsend_messages, const std::string& error_message)
{
	json session_response_json;
	try
	{
		session_response_json["content_type"] = "NOTIFICATION";
		session_response_json["message_type"] = "SessionResponse";
		session_response_json["notification_time"] = ServerTime::get_current_time_to_milliseconds();
		// nullptr チェックを追加
		if (session_response_message != nullptr)
		{
			session_response_json["client_id"] = session_response_message->_client_id;
			session_response_json["verify_result"] = session_response_message->_verify_result;
		}
		else
		{
			// nullptr の場合のエラーハンドリング（例：空のクライアントIDなど）
			session_response_json["client_id"] = "";
			session_response_json["verify_result"] = false;
		}

		// unreceived_messages 配列だけを代入
		if (session_response_message->_verify_result == true)
		{
			session_response_json["unsend_messages"] = unsend_messages["unreceived_messages"];
		}
		else
		{
			session_response_json["unsend_messages"] = json::array();
		}

		if (session_response_message != nullptr)
		{
			session_response_json["error_reason"] = error_message;
		}
		spdlog::info("SessionResponse message deserialized successfully.");
	}
	catch(const std::exception& e)
	{
		LOG_ERROR("Error deserializing SessionResponse message: {}", e.what());
		return json(); // 空のJSONオブジェクトを返す
	}

    return session_response_json;
}
