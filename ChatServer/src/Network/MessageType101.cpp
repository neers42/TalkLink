#include "Network/MessageType101.hpp"


MessageType101::MessageType101(const std::string& client_id, bool verify_result)
	:BaseMessage(ContentType::NOTIFICATION, "101"),
	_client_id{ client_id },
	_verify_result{ verify_result }
{
}


MessageType101::~MessageType101()
{
}

json MessageType101::deserialize_message(const std::unique_ptr<MessageType101>& mt101, const json& unsend_messages, const std::string& error_message)
{
	json mt101_json;

	mt101_json["content_type"] = "NOTIFICATION";
	mt101_json["message_type"] = "101";
	mt101_json["notification_time"] = ServerTime::get_current_time_to_milliseconds();
	// nullptr チェックを追加
	if (mt101 != nullptr) {
		mt101_json["client_id"] = mt101->_client_id;
		mt101_json["verify_result"] = mt101->_verify_result;
	}
	else {
		// nullptr の場合のエラーハンドリング（例：空のクライアントIDなど）
		mt101_json["client_id"] = "";
		mt101_json["verify_result"] = false;
	}

	// unreceived_messages 配列だけを代入
	if(mt101->_verify_result == true){
		mt101_json["unsend_messages"] = unsend_messages["unreceived_messages"];
	}else{
		mt101_json["unsend_messages"] = json::array();
	}

	if (mt101 != nullptr) {
		mt101_json["error_reason"] = error_message; 
	}
	spdlog::info("MT101 message deserialized successfully.");

    return mt101_json;
}
