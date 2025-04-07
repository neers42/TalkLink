#include "MessageType101.h"

MessageType101::MessageType101()
	:BaseMessage(ContentType::NOTIFICATION, "101"), 
	_client_id{ "" },
	_login_result{ false }
{
}

MessageType101::MessageType101(const std::string& client_id, bool login_result)
	:BaseMessage(ContentType::NOTIFICATION, "101"),
	_client_id{ client_id },
	_login_result{ login_result }
{
}


MessageType101::~MessageType101()
{
}

json MessageType101::desirialize_message(const std::unique_ptr<MessageType101>& mt101, const json& unread_messages, const std::string& error_message)
{
	json mt101_json;

	mt101_json["content_type"] = "NOTIFICATION";
	mt101_json["message_type"] = "101";
	mt101_json["notification_time"] = ServerTime::get_current_time();
	// nullptr チェックを追加
	if (mt101 != nullptr) {
		mt101_json["client_id"] = mt101->_client_id;
		mt101_json["login_result"] = mt101->_login_result;
	}
	else {
		// nullptr の場合のエラーハンドリング（例：空のクライアントIDなど）
		mt101_json["client_id"] = "";
		mt101_json["login_result"] = false;
	}
	mt101_json["unread_messages"] = unread_messages;

	// login_result によってエラーメッセージを決定
	if (mt101 != nullptr && mt101->_login_result == true) {
		mt101_json["error_reason"] = "No Error";  // 変更: No Error. から No Error に
	}
	else {
		mt101_json["error_reason"] = error_message;
	}
		
    return mt101_json;
}
