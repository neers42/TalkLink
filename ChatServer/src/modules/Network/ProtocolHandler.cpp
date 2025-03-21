#include "ProtocolHandler.h"

ProtocolHandler::ProtocolHandler() :_message_id_generator(std::make_unique<MessageIDGenerator>())
{
	//wolfSSLの初期化
	initialize_wolfssl();
	wolfSSL_Debugging_ON();
	wolfSSL_SetLoggingCb(ProtocolHandler::wolfssl_log_callback);

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
	if (this->_server_w_ssl != NULL) {
		wolfSSL_shutdown(this->_server_w_ssl);
		wolfSSL_free(this->_server_w_ssl);
	}

	//SSLコンテキストの解放
	if (this->_server_w_ctx != NULL) {
		wolfSSL_CTX_free(this->_server_w_ctx);
	}
}

WOLFSSL* ProtocolHandler::get_wolfssl_object()
{
	return this->_server_w_ssl;
}

WOLFSSL_CTX* ProtocolHandler::get_wolfssl_ctx()
{
	return this->_server_w_ctx;
}


const std::string ProtocolHandler::generate_message_id()
{
	//ミリ秒単位の経過時間
	long long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	//第二引数：0~255範囲の乱数を生成する関数
	ulid::ULID ulid = ulid::Create(now, random::random_byte);
	std::string message_id = ulid::Marshal(ulid);
	spdlog::info("generated message id:{:s}.", message_id);
	return message_id;
}

WOLFSSL* ProtocolHandler::generate_and_attach_ssl_object(SOCKET client_socket)
{
	WOLFSSL* new_client_ssl = wolfSSL_new(this->_server_w_ctx);
	//クライアントのSSL生成が正常に処理されなかった場合
	if (new_client_ssl == NULL) {
		LOG_ERROR("new client ssl object creation failed.");
		//spdlog::error("new client ssl object creation failed.");
		wolfSSL_free(new_client_ssl);
	}
	//クライアントのSSL生成が正常に処理された場合
	else {
		spdlog::info("new client ssl object creation success!!.");
		//SSLオブジェクトとクライアントソケットを関連付けする。
		wolfSSL_set_fd(new_client_ssl, client_socket);
		return new_client_ssl;
	}
}


WOLFSSL* ProtocolHandler::wolfssl_accept(const SOCKET client_socket)
{
	//SSLオブジェクトの生成及び
	WOLFSSL* new_client_ssl = generate_and_attach_ssl_object(client_socket);
	if (wolfSSL_accept(new_client_ssl) == SSL_SUCCESS) {
		return new_client_ssl;

	}
	else {
		this->print_wolfssl_error(new_client_ssl);
		wolfSSL_free(new_client_ssl);
		return nullptr;
	}
}

void ProtocolHandler::shutdown_wolfssl(WOLFSSL* client_ssl)
{
	int ret = wolfSSL_shutdown(client_ssl);
	if (ret == WOLFSSL_SUCCESS) {
		spdlog::debug("wolfSSL shutdown success.");
		LogHandler::get_instance().logging_debug("wolfSSL shutdown success.");
	}
	else
	{
		this->print_wolfssl_error(client_ssl);
	}
	wolfSSL_free(client_ssl);

}
std::tuple<json, bool> ProtocolHandler::receive_message_tls(WOLFSSL* client_ssl)
{
	json recv_msg_json;
	char buffer[MESSAGE_SIZE];
	int message_size = 0;
	std::vector<uint8_t> message_buffer;
	bool disconnect_flag = false;
	while (true)
	{
		int bytes_read = wolfSSL_read(client_ssl, buffer, sizeof(buffer));
		message_size += bytes_read;
		//受信したバイト数が0より大きい場合msgの解析を実施する。
		if (bytes_read > 0) {
			//メッセージバッファーのvector行列に受信したbufferのmsg分を追加
			message_buffer.insert(message_buffer.end(), buffer, buffer+bytes_read);
			try
			{
				//jsonメッセージの解析
				recv_msg_json = json::from_msgpack(message_buffer);
				//完全なメッセージが解析された場合は中身を表示
				spdlog::debug("message receive success, received message size : {:d}.", bytes_read);
				//LogHandler::get_instance().logging_debug(StringFormat("message receive success, received message size : %d.", bytes_read));
			}
			catch (const json::parse_error& e)
			{
				//jsonが完全ではない場合、再度受信を繰り返す
				continue;
			}

			return std::make_tuple(recv_msg_json, disconnect_flag);
			break;
		}
		//受信したバイト数が0の場合はクライアントを切断する処理に移行する
		else if(bytes_read == 0){
			disconnect_flag = true;
			return std::make_tuple(json(), disconnect_flag);
			break;
		}
		//上記の判定以外の場合の処理
		else
		{
			int recv_error = wolfSSL_get_error(client_ssl, bytes_read);
			if (recv_error == WOLFSSL_ERROR_WANT_READ || recv_error == WOLFSSL_ERROR_WANT_WRITE) {
				continue;
			}
			//0がリターンされてきた場合の処理について
			else if (recv_error == WOLFSSL_ERROR_ZERO_RETURN) {
				disconnect_flag = true;
				return std::make_tuple(json(), disconnect_flag);
				break;
			}
			//システムコール関連のエラーが発生した場合
			else if (recv_error == WOLFSSL_ERROR_SYSCALL) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					LOG_ERROR("message receive failed, wolfSSL read timeout occured.");
					//spdlog::error("message receive failed, wolfSSL read timeout occured.");
				}
				else {
					LOG_ERROR("message receive failed, wolfSSL read syscall error occured.");
					//spdlog::error("message receive failed, wolfSSL read syscall error occured.");
				}
			}
			//それ以外の理由
			else {
				LOG_ERROR("message receive failed.");
				//spdlog::error("message receive failed.");
			}
			return std::make_tuple(json(), disconnect_flag);

		}
	}

}

bool ProtocolHandler::send_message_tls(WOLFSSL* client_ssl, json message)
{
	std::vector<uint8_t> message_buffer = json::to_msgpack(message);
	int message_size = message_buffer.size();
	int total_sent = 0;
	int chunk_size = 16384;  // 最大TLSレコードサイズ
	int ret;
	while (total_sent < message_size) {
		int to_send = (message_size - total_sent > chunk_size) ? chunk_size : (message_size - total_sent);
		ret = wolfSSL_write(client_ssl, message_buffer.data() + total_sent, to_send);
		if (ret > 0) {
			total_sent += ret;
			continue;
		}
		// エラー発生時
		int err = wolfSSL_get_error(client_ssl, ret);
		if (err == WOLFSSL_ERROR_WANT_WRITE || err == WOLFSSL_ERROR_WANT_READ) {
			// 一時的なエラー、リトライ
			this->print_wolfssl_error(client_ssl);
			spdlog::info("Temporary error (WANT_WRITE/WANT_READ), retrying...");
			continue;
		}
		else {
			// 致命的なエラー
			this->print_wolfssl_error(client_ssl);
			LOG_ERROR("wolfSSL_write failed: {:d}", err);
			//spdlog::error("wolfSSL_write failed: {:d}", err);
			wolfSSL_shutdown(client_ssl);
			return false;
		}
	}
	return true;
}

const std::string ProtocolHandler::get_tls_version(const WOLFSSL* ssl)
{
	return std::string(wolfSSL_get_version(this->_server_w_ssl));
}

const std::string ProtocolHandler::get_cipher_suite(WOLFSSL* ssl)
{
	return std::string(wolfSSL_get_cipher(this->_server_w_ssl));
}

const std::tuple<std::string, std::string, std::string, std::string>  ProtocolHandler::get_peer_cetificate(WOLFSSL* ssl)
{
	std::string cert_subject_name = "";
	std::string cert_issuer_name = "";
	std::string cert_not_before = "";
	std::string cert_not_after = "";
	char subject_name_buffer[256] = { 0 },
		issuer_name_buffer[256] = { 0 },
		cert_not_before_buffer[32] = { 0 },
		cert_not_after_buffer[32] = { 0 };
	if (!ssl) {
		LOG_ERROR("SSL object is null.");
		//spdlog::error("SSL object is null.");
		return std::make_tuple(cert_subject_name, cert_issuer_name, cert_not_before, cert_not_after);
	}

	///////////////////////


	WOLFSSL_X509* server_cert = wolfSSL_get_peer_certificate(ssl);
	if (!server_cert) {
		spdlog::warn("No client certificate received.");
		return std::make_tuple(cert_subject_name, cert_issuer_name, cert_not_before, cert_not_after);
	}
	// 証明書のサブジェクト名（発行対象）
	wolfSSL_X509_NAME_oneline(wolfSSL_X509_get_subject_name(server_cert), subject_name_buffer, sizeof(subject_name_buffer));
	cert_subject_name = std::string(subject_name_buffer);

	// 証明書の発行者（CA）
	wolfSSL_X509_NAME_oneline(wolfSSL_X509_get_issuer_name(server_cert), issuer_name_buffer, sizeof(issuer_name_buffer));
	cert_issuer_name = std::string(issuer_name_buffer);


	// 証明書の有効期限（開始日）
	auto cert_not_before_time = wolfSSL_X509_get_notBefore(server_cert);
	auto before_time = wolfSSL_ASN1_TIME_get_data(cert_not_before_time);
	std::string before_time_str(reinterpret_cast<const char*>(before_time));
	if (before_time_str.size() < 14) {
		cert_not_before = "";
	}
	else {
		// YYYYMMDDHHMMSS を "YYYY-MM-DD HH:MM:SS" の形式に変換
		std::ostringstream oss_before;
		oss_before
			<< before_time_str.substr(0, 4) << "-"  // YYYY-
			<< before_time_str.substr(4, 2) << "-"  // MM-
			<< before_time_str.substr(6, 2) << " "  // DD
			<< before_time_str.substr(8, 2) << ":"  // HH:
			<< before_time_str.substr(10, 2) << ":" // MM:
			<< before_time_str.substr(12, 2);       // SS
		cert_not_before = oss_before.str();
	}


	// 証明書の有効期限（終了日）
	auto cert_not_after_time = wolfSSL_X509_get_notAfter(server_cert);
	auto after_time = wolfSSL_ASN1_TIME_get_data(cert_not_after_time);
	std::string after_time_str(reinterpret_cast<const char*>(after_time));
	if (after_time_str.size() < 14)
	{
		cert_not_after = "";
	}
	else {
		std::stringstream oss_after;
		oss_after
			<< after_time_str.substr(0, 4) << "-"  // YYYY-
			<< after_time_str.substr(4, 2) << "-"  // MM-
			<< after_time_str.substr(6, 2) << " "  // DD
			<< after_time_str.substr(8, 2) << ":"  // HH:
			<< after_time_str.substr(10, 2) << ":" // MM:
			<< after_time_str.substr(12, 2);       // SS
		cert_not_after = oss_after.str();
	}

	// 証明書オブジェクトを解放
	wolfSSL_X509_free(server_cert);
	spdlog::info(" * Server Certificate Subject Name : {:s}", cert_subject_name);
	spdlog::info(" * Server Certificate Issuer Name : {:s}", cert_issuer_name);
	spdlog::info(" * Server Certificate Not Before : {:s}", cert_not_before);
	spdlog::info(" * Server Certificate Not After : {:s}", cert_not_after);
	return std::make_tuple(cert_subject_name, cert_issuer_name, cert_not_before, cert_not_after);

}

void ProtocolHandler::wolfssl_log_callback(const int log_level, const char* log_message)
{
	std::string log_str(log_message);
	switch (log_level) {
	case ERROR_LOG:
		LOG_ERROR(log_str);
		//spdlog::error(log_str);
			break;
	case INFO_LOG:
		spdlog::debug(log_str);
		break;
	case ENTER_LOG:
		spdlog::debug(log_str);
		break;
	default:
		spdlog::debug(log_str);
		break;
	}
}


//SSL通信以外で発生するエラーの表示
void ProtocolHandler::print_wolfssl_global_error()
{
	int err = 0;
	char buffer[80];
	err = wolfSSL_ERR_get_error();
	wolfSSL_ERR_error_string(err, buffer);
	LOG_ERROR("wolfSSL, failed, error content : {:s} (code : {:d}.)", std::string(buffer), err);
	//spdlog::error("wolfSSL, failed, error content : {:s} (code : {:d}.)", std::string(buffer), err);
}

//SSL通信で発生するエラーの表示
void ProtocolHandler::print_wolfssl_error(WOLFSSL* client_ssl)
{
	int err = 0;
	char buffer[80];
	err = wolfSSL_get_error(client_ssl, 0);
	wolfSSL_ERR_error_string(err, buffer);
	//spdlog::error("wolfSSL, failed, error content : {:s} (code : {:d})", std::string(buffer), err);
	LOG_ERROR("wolfSSL, failed, error content : {:s} (code : {:d}.)", std::string(buffer), err);
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
		spdlog::critical("SSL v23 server method, failed, unable to get method.");
		exit(1);
	}
	else
	{
		spdlog::debug("server_method set v23 for wolfssl context.");
	}
#endif // SERVER_METHOD_SSLV23

#ifdef SERVER_METHOD_TLS_1_3
	//TLS接続時のバージョン指定(v23＝サーバー及びクライアントがサポートする最も高いバージョンを指定)
	auto server_method = wolfTLSv1_3_server_method();
	if (server_method == NULL) {
		spdlog::critical("TLS v1.3 server method, failed, unable to get method.");
	}
	else
	{
		spdlog::debug("server_method set TLS 1.3 for wolfssl context.");
	}

#endif // server_method_TLS1_3

#ifdef SERVER_METHOD_TLS_1_2
	//TLS接続時のバージョン指定(v23＝サーバー及びクライアントがサポートする最も高いバージョンを指定)
	auto server_method = wolfTLSv1_2_server_method();
	if (server_method == NULL) {
		spdlog::critical("TLS v1.2 server method, failed, unable to get method.");
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
		spdlog::critical("TLS v1.1 server method, failed, unable to get method.");
		LogHandler::get_instance().logging_critical(StringFormat("TLS v1.1 server method, failed, unable to get method."));
	}
	else
	{
		spdlog::debug("server_method set TLS 1.1 for wolfssl context.");
		LogHandler::get_instance().logging_debug("server_method set TLS 1.1 for wolfssl context.");
	}
#endif // SERVER_METHOD_TLS_1_1


	return server_method;
}

void ProtocolHandler::generate_ssl_context(WOLFSSL_METHOD* method)
{
	//wolf contextの確保
	_server_w_ctx = wolfSSL_CTX_new(method);
	if (_server_w_ctx == NULL) {
		spdlog::critical("SSL context, failed, context creation failed.");
		exit(1);
	}
}

void ProtocolHandler::load_cert_file()
{
	//サーバ証明書のロード
	if (wolfSSL_CTX_use_certificate_file(_server_w_ctx, SERVER_CERT_FILE_PASS, SSL_FILETYPE_PEM) == SSL_FAILURE) {
		this->print_wolfssl_global_error();
	}
}

void ProtocolHandler::load_private_key()
{
	//プライベート鍵のロード
	if (wolfSSL_CTX_use_PrivateKey_file(_server_w_ctx, SERVER_PRIVATEKEY_FILE_PASS, SSL_FILETYPE_PEM) == SSL_FAILURE) {
		this->print_wolfssl_global_error();
	}
}


