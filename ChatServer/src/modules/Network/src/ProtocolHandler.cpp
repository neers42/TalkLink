#include "ProtocolHandler.h"

ProtocolHandler::ProtocolHandler()
{
	wolfSSL_Init();
}

ProtocolHandler::~ProtocolHandler()
{
}

template <typename T>
T ProtocolHandler::serialize_message(const std::vector<uint8_t> recv_data)
{
	json recev_json = json::from_msgpack(recv_data);
	std::string mt = recev_json["MessageType"];
	
	//MT001 = AUTH認証メッセージ
	if (std::equal(mt.begin(), mt.end(), "001")) {
		return this->serialize_mt001(recev_json);	//MT001を返す
	}
	//MT002 = 通常メッセージの場合
	else if (std::equal(mt.begin(), mt.end(), "002")) {
		return this->serialize_mt002(recev_json);	//MT002を返す
	}
	//MT003 = の場合
	else if (std::equal(mt.begin(), mt.end(), "003")) {
		
	}
	//MT004 = の場合
	else if (std::equal(mt.begin(), mt.end(), "004")) {

	}

	else {
		LogHandler::get_instance().logging_error("The receive process was interrupted due to a bad message.");
	}
	return true;
}

MessageType001 ProtocolHandler::serialize_mt001(const json& recv_json)
{
	int content_type = recv_json["content_type"];
	std::string message_type = recv_json["message_type"];
	int sign_type = recv_json["sign_type"];
	int auth_type = recv_json["auth_type"];
	std::string client_name = recv_json["client_name"];
	std::string client_mail_addr = recv_json["client_mail_address"];
	std::string client_password = recv_json["client_password"];
	return MessageType001(content_type, message_type, sign_type, auth_type, client_name, client_mail_addr, client_password);
}

MessageType002 ProtocolHandler::serialize_mt002(const json& recv_json)
{
	int content_type = recv_json["content_type"];
	std::string message_type = recv_json["message_type"];
	int trans_type = recv_json["trans_type"];
	std::string dst_client_id = recv_json["dst_client_id"];
	std::string src_client_id = recv_json["src_client_id"];
	std::string dst_chatroom_id = recv_json["dst_chatroom_id"];
	int data_type = recv_json["content_type"];
	std::string content = recv_json["main_content"];
	std::string message_id = "";
	return MessageType002(content_type, message_type, dst_client_id, src_client_id, content, message_id, trans_type, dst_chatroom_id);
}


