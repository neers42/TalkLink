#ifndef PROTOCOLHANDLER
#define PROTOCOLHANDLER
#define SERVER_CERT_FILE_PASS ".//////certs/server-cert.pem"
#define SERVER_PRIVATEKEY_FILE_PASS ".//////certs/server-key.pem"
#include "common.h"
#include "wolfssl/ssl.h"
#include "MessageType001.h"
#include "MessageType002.h"
#include "MessageType003.h"
//TLS接続時のプロトコルバージョンを設定
#define SERVER_METHOD_SSLV23	//サーバ及びクライアントの両方のサポートする最も高いバージョンを指定
//#define SERVER_METHOD_TLS_1_3	//TLS1.3で接続
//#define SERVER_METHOD_TLS_1_2	//TLS1.2で接続
//#define SERVER_METHOD_TLS_1_1	//TLS1.1で接続

class ProtocolHandler
{
private:
	WOLFSSL* _w_ssl = NULL;
	WOLFSSL_CTX* _w_ctx = NULL;
	
	

public:
	//コンストラクタ
	ProtocolHandler();

	//デストラクタ
	~ProtocolHandler();

	//各種アクセッサ(getter/setter)
	WOLFSSL* get_wolfssl_ssl();
	WOLFSSL_CTX* get_wolfssl_ctx();

	//JSONメッセージのシリアライズ
	template <typename T>
	T serialize_message(const std::vector<uint8_t> recv_data);

	//メッセージIDの生成
	const std::string generate_message_id();

	void generate_ssl_object();

	void ssl_object_atattch_socket(SOCKET client_socket);

	void wolfssl_accept();

private:

	//wolfSSLにおけるSSL通信以外での発生エラーを表示
	void print_wolfssl_global_error();
	
	//wolfSSLにおけるSSL通信での発生エラーを表示
	void print_wolfssl_error();

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
	MessageType001 serialize_mt001(const json& recv_json);
	MessageType002 serialize_mt002(const json& recv_json);

};


#endif // !PROTOCOLHANDLER



