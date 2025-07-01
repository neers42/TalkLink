#ifndef PROTOCOLHANDLER
#define PROTOCOLHANDLER
#define SERVER_CERT_FILE_PASS ".//////certs/server-cert.pem"
#define SERVER_PRIVATEKEY_FILE_PASS ".//////certs/server-key.pem"
#include <sstream>
#include <shared_mutex>
#include "Utils/common.h"
#include "wolfssl/ssl.h"
#include "wolfssl/wolfcrypt/asn_public.h"
#include "MessageType000.h"
#include "MessageType001.h"
#include "MessageType002.h"
#include "MessageType003.h"
#include "MessageType004.h"
#include "MessageType005.h"
#include "MessageType101.h"
#include "MessageType102.h"
#include "MessageType103A.h"
#include "MessageType103B.h"
#include "MessageType104A.h"
#include "MessageType104B.h"
#include "ulid/ulid.hh"
#include "ulid/ulid_struct.hh"
#include "spdlog/spdlog.h"
#include "Chat/MessageIDGenerator.h"
#include "Chat/Client.h"
#ifdef _WIN32
	using socket_t = SOCKET;	//WindowsではSOCKETを使用
#else
	using socket_t = int;		//Linuxではintを使用
#endif //_WIN32

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
	//std::unique_ptr<MessageIDGenerator> _message_id_generator;
	

public:
	std::shared_mutex ssl_shared_mtx;

public:
	//コンストラクタ
	ProtocolHandler();

	//デストラクタ
	~ProtocolHandler();

	//各種アクセッサ(getter/setter)
	WOLFSSL* get_wolfssl_object();
	WOLFSSL_CTX* get_wolfssl_ctx();



	//メッセージIDの生成
	const std::string generate_message_id();

	//SSLオブジェクトの生成
	WOLFSSL* generate_and_attach_ssl_object(socket_t client_socket);

	//SSLのアクセプト
	WOLFSSL* wolfssl_accept(const socket_t client_socket);

	//wolfSSLのシャットダウン
	void shutdown_wolfssl(WOLFSSL* client_ssl);
	void shutdown_wolfssl(std::shared_ptr<Client> client);
	
	//メッセージの受信
	std::tuple<json, bool, bool> receive_message_tls(std::shared_ptr<Client> client);
    
	//メッセージの送信
	bool send_message_tls(std::shared_ptr<Client> client, json message);

	//TLSのバージョンを取得
	const std::string get_tls_version(const WOLFSSL* ssl);

	//使用されている暗号スイートを取得
	const std::string get_cipher_suite(WOLFSSL* ssl);

	//サーバの証明書情報を取得
	const std::tuple<std::string, std::string, std::string, std::string> get_peer_cetificate(WOLFSSL* ssl);

	static void wolfssl_log_callback(const int log_level, const char* log_message);

	void print_json_message(const json& json_message) {
		if (json_message.is_null() || json_message.empty()) {
			LOG_ERROR("Invalid JSON object.");
			return;
		}
		spdlog::debug("Receive JSON DATA :\n{:s}", json_message.dump(4));
	}
	
	//メッセージの受信情報を表示
	template<typename T>
	void print_received_message_info(const std::unique_ptr<T>& message_instance,
		const uint64_t message_id,
		const std::string& src_id,
		const std::string dst_id,
		const std::string& ipv4,
		const socket_t socket_num)
	{
		if constexpr (std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, MessageType000>) {
		
		}
		else if constexpr (std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, MessageType001>) {
			spdlog::info("server received \033[33mMessage Type 001\033[m from sender(ip:{:s}, socket:{:d})", ipv4, socket_num);
		}
		else if constexpr (std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, MessageType003>) {
			spdlog::info("server received \033[33mMessage Type 003\033[m from sender(id:{:s}ip:{:s}, socket:{:d})", dst_id, ipv4, socket_num);
		}
		else if constexpr (std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, MessageType004>) {
			spdlog::info("server received \033[33mMessage Type 004\033[m(message id:{:d}) from sender(id:{:s}, ip:{:s}, socket:{:d}) ==> receiver(id:{:s})"
				, message_id, src_id, ipv4, socket_num, dst_id);
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
	
};



#endif // !PROTOCOLHANDLER



