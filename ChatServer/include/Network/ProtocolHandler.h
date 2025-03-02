#ifndef PROTOCOLHANDLER
#define PROTOCOLHANDLER
#define SERVER_CERT_FILE_PASS ".//////certs/server-cert.pem"
#define SERVER_PRIVATEKEY_FILE_PASS ".//////certs/server-key.pem"
#include <sstream>
#include "common.h"
#include "wolfssl/ssl.h"
#include "wolfssl/wolfcrypt/asn_public.h"
#include "MessageType000.h"
#include "MessageType001.h"
#include "MessageType002.h"
#include "MessageType003.h"
//TLS接続時のプロトコルバージョンを設定
#define SERVER_METHOD_SSLV23	//サーバ及びクライアントの両方のサポートする最も高いバージョンを指定
//#define SERVER_METHOD_TLS_1_3	//TLS1.3で接続
//#define SERVER_METHOD_TLS_1_2	//TLS1.2で接続
//#define SERVER_METHOD_TLS_1_1	//TLS1.1で接続
#define MESSAGE_SIZE 1024	//SSL readで受信するメッセージのサイズ

class ProtocolHandler
{
private:
	WOLFSSL* _server_w_ssl = NULL;
	WOLFSSL_CTX* _server_w_ctx = NULL;
	
	

public:
	//コンストラクタ
	ProtocolHandler();

	//デストラクタ
	~ProtocolHandler();

	//各種アクセッサ(getter/setter)
	WOLFSSL* get_wolfssl_object();
	WOLFSSL_CTX* get_wolfssl_ctx();

	//JSONメッセージのシリアライズ
	//template <typename T>
	//std::unique_ptr<T> serialize_message(const json& json_message);

	template <typename T>
	std::unique_ptr<T> serialize_message(const json& json_message)
	{
		// json_messageからMessageTypeを取得
		if constexpr (std::is_same<T, MessageType001>::value) {
			return this->serialize_mt001<T>(json_message);	//MT001を返す
		}
		else if constexpr (std::is_same<T, MessageType002>::value) {
			return this->serialize_mt002<T>(json_message);	//MT002を返す
		}
		else {
			LogHandler::get_instance().logging_error("Unsupported message type");
			return nullptr;
		}
	}

	//メッセージIDの生成
	const std::string generate_message_id();

	//SSLオブジェクトの生成
	WOLFSSL* generate_and_attach_ssl_object(SOCKET client_socket);

	//SSLのアクセプト
	WOLFSSL* wolfssl_accept(const SOCKET client_socket);

	//wolfSSLのシャットダウン
	void shutdown_wolfssl(WOLFSSL* client_ssl);
	//メッセージの受信
	std::tuple<json, bool> receive_message_tls(WOLFSSL* client_ssl);

	//メッセージの送信
	std::tuple<int, json> send_message_tls();

	//TLSのバージョンを取得
	const std::string get_tls_version(const WOLFSSL* ssl);

	//使用されている暗号スイートを取得
	const std::string get_cipher_suite(WOLFSSL* ssl);

	//サーバの証明書情報を取得
	const std::tuple<std::string, std::string, std::string, std::string> get_peer_cetificate(WOLFSSL* ssl);

	static void wolfssl_log_callback(const int log_level, const char* log_message);
	
	//メッセージの受信情報を表示
	template<typename T>
	void print_received_message_info(const std::unique_ptr<T>& message_instance,
		const std::string& src_id,
		const std::string dst_id,
		const std::string& ipv4,
		const int socket_num)
	{
		if constexpr (std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, MessageType000>) {
			/*LogHandler::get_instance().logging_debug(StringFormat("Receive message info : \n"
				"Content Type : %d\n"
				"Message Type : %s\n"
				"Message ID : %s\n"
				message_instance.get_content_type(),
				message_instance.get_message_type(),
				message_instance.get_message_id()));*/
		}
		else if constexpr (std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, MessageType001>) {
			LogHandler::get_instance().logging_info(StringFormat("server received \033[33mMessage Type 001\033[m from client(ip:%s, socket_num:%d)"
				, ipv4, socket_num));
			/*	LogHandler::get_instance().logging_debug(StringFormat("Receive message info : \n"
					"Content Type : %d\n"
					"Message Type : %s\n"
					"Message ID : %s\n"
					"Sign Type : %d\n"
					"Auth Type : %d\n"
					"Client Name : %s\n"
					"Client Mail Address : %s\n"
					"Client Password : %s\n",
					message_instance.get_content_type(),
					message_instance.get_message_type(),
					message_instance.get_message_id(),
					message_instance.get_sign_type_int(),
					message_instance.get_auth_type_int(),
					message_instance.get_client_name(),
					message_instance.get_client_mail_addr(),
					message_instance.get_client_password()));*/
		}
		else if constexpr (std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, MessageType002>) {
			LogHandler::get_instance().logging_info(StringFormat("server received \033[33mMessage Type 002\033[m from src client(id:%s, ip:%s, socket_num:%d) ==> dst client(id:%s)"
				, src_id, ipv4, socket_num, dst_id));
			/*LogHandler::get_instance().logging_debug(StringFormat("Receive message info : \n"
				"Content Type : %d\n"
				"Message Type : %s\n"
				"Message ID : %s\n"
				"Source ID : %s\n"
				"Destination ID : %s\n"
				"Transaction Type : %d\n"
				"Data Type : %d\n"
				"Main Content : %s\n",
				message_instance.get_content_type(),
				message_instance.get_message_type(),
				message_instance.get_message_id(),
				message_instance.get_src_client_id(),
				message_instance.get_dst_client_id(),
				message_instance.get_content()));*/
		}

	}


private:

	//wolfSSLにおけるSSL通信以外での発生エラーを表示
	void print_wolfssl_global_error();
	
	//wolfSSLにおけるSSL通信での発生エラーを表示
	void print_wolfssl_error(WOLFSSL* client_ssl);

	//wolfSSLの初期化
	void initialize_wolfssl();

	//サーバのTLSプロトコルバージョンを設定
	WOLFSSL_METHOD* set_server_method();

	//SSLのコンテキストを設定
	void generate_ssl_context(WOLFSSL_METHOD* method);

	//サーバの証明書をロード
	void load_cert_file();

	//サーバの秘密鍵をロード
	void load_private_key();

	//各メッセージデータをjson形式にシリアライズする(オブジェクト⇒文字系列式)
	template <typename T>
	std::unique_ptr<T> serialize_mt001(const json& recv_json) {
		int content_type = recv_json["content_type"];
		std::string message_type = recv_json["message_type"];
		int sign_type = recv_json["sign_type"];
		int auth_type = recv_json["auth_type"];
		std::string client_name = recv_json["name"];
		std::string client_mail_addr = recv_json["mail_address"];
		std::string client_password = recv_json["password"];

		if constexpr (std::is_same<T, MessageType001>::value) {
			return std::make_unique<T>(content_type, message_type, sign_type, auth_type, client_name, client_mail_addr, client_password);
		}
		else {
			// 他の型に対応する場合
			LogHandler::get_instance().logging_error("Unsupported message type");
			return nullptr;
		}
	}
	template <typename T>
	std::unique_ptr<T> serialize_mt002(const json& recv_json) {
		// jsonから必要な値を抽出
		int content_type = recv_json["content_type"].get<int>();  // int型
		std::string message_type = recv_json["message_type"];      // std::string型
		std::string dst_client_id = recv_json["dst_client_id"];    // std::string型
		std::string src_client_id = recv_json["src_client_id"];    // std::string型
		std::string content = recv_json["content"];                // std::string型
		std::string message_id = "";  // 空の文字列
		int trans_type = recv_json["trans_type"].get<int>();  // int型
		std::string dst_chatroom_id = recv_json["dst_chatroom_id"];  // std::string型

		if constexpr (std::is_same<T, MessageType002>::value) {
			// T（MessageType002）のコンストラクタに合わせて、8個の引数を渡す
			return std::make_unique<T>(content_type, message_type, dst_client_id, src_client_id,
				content, message_id, trans_type, dst_chatroom_id);
		}
		else {
			// 他の型に対応する場合
			LogHandler::get_instance().logging_error("Unsupported message type");
			return nullptr;
		}
	}

};



#endif // !PROTOCOLHANDLER



