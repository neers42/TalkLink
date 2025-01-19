#include "ProtocolHandler.h"

ProtocolHandler::ProtocolHandler()
{
	//wolfSSLの初期化
	initialize_wolfssl();

	//TLS接続時のバージョン指定
	auto server_method = this->set_server_method();

	//コンテクストの生成
	generate_ssl_context(server_method);

	//サーバ証明書のロード
	load_cert_file();

	//サーバの秘密鍵のロード
	load_private_key();
	
}

ProtocolHandler::~ProtocolHandler()
{
	//sslオブジェクトの解放
	if (this->_w_ssl != NULL) {
		wolfSSL_shutdown(this->_w_ssl);
		wolfSSL_free(this->_w_ssl);
	}

	//SSLコンテキストの解放
	if (this->_w_ctx != NULL) {
		wolfSSL_CTX_free(this->_w_ctx);
	}
}

WOLFSSL* ProtocolHandler::get_wolfssl_ssl()
{
	return this->_w_ssl;
}

WOLFSSL_CTX* ProtocolHandler::get_wolfssl_ctx()
{
	return this->_w_ctx;
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

void ProtocolHandler::generate_ssl_object()
{
	this->_w_ssl = wolfSSL_new(this->_w_ctx);
	if (_w_ssl == NULL) {
		LogHandler::get_instance().logging_critical(StringFormat("SSL object, failed, SSL obejct creation failed."));
		exit(1);
	}
}

void ProtocolHandler::ssl_object_atattch_socket(SOCKET client_socket)
{
	//クライアントソケットとの
	wolfSSL_set_fd(_w_ssl, (int)client_socket);
}

void ProtocolHandler::wolfssl_accept()
{
	if (wolfSSL_accept(_w_ssl) == SSL_SUCCESS) {
		LogHandler::get_instance().logging_info(StringFormat("client ssl, success, accept client ssl."));
	}
	else {
		this->print_wolfssl_error();
	}
}

//SSL通信以外で発生するエラーの表示
void ProtocolHandler::print_wolfssl_global_error()
{
	int err = 0;
	char buffer[80];
	err = wolfSSL_ERR_get_error();
	wolfSSL_ERR_error_string(err, buffer);
	LogHandler::get_instance().logging_critical(StringFormat("wolfSSL, fauled, error code : %s (error code : %d)"));
}

//SSL通信で発生するエラーの表示
void ProtocolHandler::print_wolfssl_error()
{
	int err = 0;
	char buffer[80];
	err = wolfSSL_get_error(_w_ssl, 0);
	wolfSSL_ERR_error_string(err, buffer);
	LogHandler::get_instance().logging_critical(StringFormat("wolfSSL, fauled, error code : %s (error code : %d)"));
}

void ProtocolHandler::initialize_wolfssl()
{
	if (wolfSSL_Init() == SSL_FAILURE) {
		this->print_wolfssl_global_error();
		exit(1);
	}
}

WOLFSSL_METHOD* ProtocolHandler::set_server_method()
{
#ifdef SERVER_METHOD_SSLV23
	//TLS接続時のバージョン指定(v23＝サーバー及びクライアントがサポートする最も高いバージョンを指定)
	auto server_method = wolfSSLv23_server_method();
	if (server_method == NULL) {
		LogHandler::get_instance().logging_critical(StringFormat("SSL v23 server method, failed, unable to get method."));
		exit(1);
	}
	else
	{
		LogHandler::get_instance().logging_debug("server_method set v23 for wolfssl context.");
	}
#endif // SERVER_METHOD_SSLV23

#ifdef SERVER_METHOD_TLS_1_3
	//TLS接続時のバージョン指定(v23＝サーバー及びクライアントがサポートする最も高いバージョンを指定)
	auto server_method = wolfTLSv1_3_server_method();
	if (server_method == NULL) {
		LogHandler::get_instance().logging_critical(StringFormat("TLS v1.3 server method, failed, unable to get method."));
	}
	else
	{
		LogHandler::get_instance().logging_debug("server_method set TLS 1.3 for wolfssl context.");
	}

#endif // server_method_TLS1_3

#ifdef SERVER_METHOD_TLS_1_2
	//TLS接続時のバージョン指定(v23＝サーバー及びクライアントがサポートする最も高いバージョンを指定)
	auto server_method = wolfTLSv1_2_server_method();
	if (server_method == NULL) {
		LogHandler::get_instance().logging_critical(StringFormat("TLS v1.2 server method, failed, unable to get method."));
	}
	else
	{
		LogHandler::get_instance().logging_debug("server_method set TLS 1.2 for wolfssl context.");
	}

#endif // SERVER_METHOD_TLS_1_2

#ifdef SERVER_METHOD_TLS_1_1
	//TLS接続時のバージョン指定(v23＝サーバー及びクライアントがサポートする最も高いバージョンを指定)
	auto server_method = wolfTLSv1_1_server_method();
	if (server_method == NULL) {
		LogHandler::get_instance().logging_critical(StringFormat("TLS v1.1 server method, failed, unable to get method."));
	}
	else
	{
		LogHandler::get_instance().logging_debug("server_method set TLS 1.1 for wolfssl context.");
	}
#endif // SERVER_METHOD_TLS_1_1


	return server_method;
}

void ProtocolHandler::generate_ssl_context(WOLFSSL_METHOD* method)
{
	//wolf contextの確保
	_w_ctx = wolfSSL_CTX_new(method);
	if (_w_ctx == NULL) {
		LogHandler::get_instance().logging_critical(StringFormat("SSL context, failed, context creation failed."));
		exit(1);
	}
}

void ProtocolHandler::load_cert_file()
{
	//サーバ証明書のロード
	if (wolfSSL_CTX_use_certificate_file(_w_ctx, SERVER_CERT_FILE_PASS, SSL_FILETYPE_PEM) == SSL_FAILURE) {
		this->print_wolfssl_global_error();
	}
}

void ProtocolHandler::load_private_key()
{
	//プライベート鍵のロード
	if (wolfSSL_CTX_use_PrivateKey_file(_w_ctx, SERVER_PRIVATEKEY_FILE_PASS, SSL_FILETYPE_PEM) == SSL_FAILURE) {
		this->print_wolfssl_global_error();
	}
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

