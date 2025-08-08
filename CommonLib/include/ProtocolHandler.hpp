#ifndef PROTOCOLHANDLER_HPP
#define PROTOCOLHANDLER_HPP
#include <sstream>
#include <shared_mutex>
#include <iostream>
#include <string>
#include <mutex>
#include "stdlib.h"
#include "LogHandler.hpp"
#include "nlohmann/json.hpp"
#include <thread>
#include <chrono>
#include <memory>
#include <errno.h>
#include <random>
#include <sstream>
#include "ulid/ulid.hh"
using json = nlohmann::json;
#ifdef _WIN32
#include <winsock2.h>	//Windowsではwinsock2.hを使用
#include <ws2tcpip.h>	//Windowsではws2tcpip.hを使用
using socket_t = SOCKET;	//WindowsではSOCKETを使用
#define SERVER_CERT_FILE_PASS "./certs/server-cert.pem"
#define SERVER_PRIVATEKEY_FILE_PASS "./certs/server-key.pem"
#else
using socket_t = int;		//Linuxではintを使用
	#define SERVER_CERT_FILE_PASS "/app/ChatServer/certs/server-cert.pem"
	#define SERVER_PRIVATEKEY_FILE_PASS "/app/ChatServer/certs/server-key.pem"
#endif //_WIN32
#include <openssl/err.h>
#include <openssl/ssl.h>
#include "ulid/ulid.hh"
#include "ulid/ulid_struct.hh"
#include "spdlog/spdlog.h"


//TLS接続時のプロトコルバージョンを設定
#define SERVER_METHOD_SSLV23	//サーバ及びクライアントの両方のサポートする最も高いバージョンを指定
//#define SERVER_METHOD_TLS_1_3	//TLS1.3で接続
//#define SERVER_METHOD_TLS_1_2	//TLS1.2で接続
//#define SERVER_METHOD_TLS_1_1	//TLS1.1で接続
#define MESSAGE_SIZE 1024	//SSL readで受信するメッセージのサイズ

class ProtocolHandler
{
private:
	SSL* _server_w_ssl = NULL;
	SSL_CTX* _server_w_ctx = NULL;

public:
	//コンストラクタ
	ProtocolHandler();
	ProtocolHandler(const std::string& cert_file_pass, const std::string& private_key_pass);
	ProtocolHandler(const ProtocolHandler&) = delete; // コピーコンストラクタを削除

	//デストラクタ
	~ProtocolHandler();

	//各種アクセッサ(getter/setter)
	SSL* get_ssl_object();
	SSL_CTX* get_ssl_ctx();

	//SSLオブジェクトの生成
	SSL* generate_and_attach_ssl_object(socket_t client_socket);

	//SSLのアクセプト
	SSL* ssl_accept(const socket_t client_socket);
	std::tuple<std::string, bool, bool> receive_message_str(SSL* client_ssl);	//メッセージの受信(std::string形式)
	std::tuple<json, bool, bool> receive_message_json(SSL* client_ssl);	//メッセージの受信(json形式)
	bool send_message_json(SSL* client_ssl, const json& message);	//メッセージの送信
	bool shutdown_ssl(SSL* client_ssl);	//SSLのシャットダウン


	//使用されている暗号スイートを取得
	const std::string get_cipher_suite(SSL* ssl);

	//サーバの証明書情報を取得
	const std::tuple<std::string, std::string, std::string, std::string> get_peer_cetificate(SSL* ssl);

	void print_json_message(const json& json_message) {
		if (json_message.is_null() || json_message.empty()) {
			LOG_ERROR("Invalid JSON object.");
			return;
		}
		spdlog::debug("Receive JSON DATA :\n{:s}", json_message.dump(4));
	}

	//メッセージの受信情報を表示
	void print_received_message_info(
	const uint64_t message_id,
	const std::string& sender_id,
	const std::string& room_id,
	const std::string& sender_ipv4,
	const socket_t socket_num,
	const json& json_message)
{
	if (json_message.is_null() || json_message.empty()) {
		LOG_ERROR("Invalid JSON object.");
		return;
	}
	spdlog::debug("server received Message(message id:{}) from sender(id:{}, ip:{}, socket:{}).\nReceive JSON DATA :\n{}", message_id, sender_id, sender_ipv4, socket_num, json_message.dump(4));

}


private:


	//SSLにおけるSSL通信での発生エラーを表示
	void print_ssl_error(SSL* client_ssl);

	//サーバのTLSプロトコルバージョンを設定
	const SSL_METHOD* set_server_method();

	//SSLのコンテキストを設定
	void generate_ssl_context(const SSL_METHOD* method);

	//サーバの証明書をロード
	void load_cert_file(const std::string& cert_file_pass);

	//サーバの秘密鍵をロード
	void load_private_key(const std::string& private_key_pass);

};



#endif // !PROTOCOLHANDLER_HPP



