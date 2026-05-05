#include "ProtocolHandler.hpp"

ProtocolHandler::ProtocolHandler(const std::string& cert_file_pass, const std::string& private_key_pass) : _running(true)
{
	//SSLの初期化
	SSL_library_init();
	SSL_load_error_strings();

	//TLS接続時のバージョン指定
	auto server_method = this->set_server_method();

	//コンテクストの生成
	generate_ssl_context(server_method);

	//サーバ証明書のロード
	load_cert_file(cert_file_pass);

	//サーバの秘密鍵のロード
	load_private_key(private_key_pass);

}

ProtocolHandler::~ProtocolHandler()
{
	//hot_reloadを停止
	this->stop_hot_reload();
	//sslオブジェクトの解放
	if (this->_server_ssl != nullptr) {
		SSL_shutdown(this->_server_ssl);
		SSL_free(this->_server_ssl);
		this->_server_ssl = nullptr;
	}
}

SSL* ProtocolHandler::get_ssl_object()
{
	return this->_server_ssl;
}

std::shared_ptr<SSL_CTX> ProtocolHandler::get_ssl_ctx()
{
	return this->_server_ctx;
}



SSL* ProtocolHandler::generate_and_attach_ssl_object(socket_t client_socket)
{
	std::shared_ptr<SSL_CTX> ctx_copy;

    // ロックは shared_ptr をコピーする瞬間だけ
    {
        std::lock_guard<std::mutex> lock(this->_ctx_mutex);
        ctx_copy = this->_server_ctx;
    }

    if (!ctx_copy) {
        LOG_ERROR("SSL_CTX is null.");
        return nullptr;
    }

    SSL* new_client_ssl = SSL_new(ctx_copy.get());
    if (!new_client_ssl) {
        LOG_ERROR("new client ssl object creation failed.");
        return nullptr;
    }

    spdlog::info("new client ssl object creation success!!");
    SSL_set_fd(new_client_ssl, static_cast<int>(client_socket));
    return new_client_ssl;


}


SSL* ProtocolHandler::ssl_accept(const socket_t client_socket)
{
	//SSLオブジェクトの生成及び
	SSL* new_client_ssl = generate_and_attach_ssl_object(client_socket);
	if (SSL_accept(new_client_ssl) == 1) {
		return new_client_ssl;

	}
	else {
		this->print_ssl_error(new_client_ssl);
		SSL_free(new_client_ssl);
		return nullptr;
	}
}

std::tuple<std::string, bool, bool> ProtocolHandler::receive_message_str(SSL *client_ssl)
{
	char buffer[MESSAGE_SIZE];
	int message_size = 0;
	std::vector<uint8_t> message_buffer;
	bool disconnect_flag = false;	//接続断の判定フラグ
	bool recv_flag = false;	//受信完了の判定フラグ
	int bytes_read = 0;

	if (client_ssl == nullptr || SSL_is_init_finished(client_ssl) == false) {
		LOG_ERROR("SSL is not initialized, cannot send message.");
		disconnect_flag = true;
		return std::make_tuple(std::string(), disconnect_flag, recv_flag);
	}
	while (true)
	{
		bytes_read = SSL_read(client_ssl, buffer, sizeof(buffer));
		message_size += bytes_read;

		//受信したバイト数が0より大きい場合msgの解析を実施する。
		if (bytes_read > 0) {
			//メッセージバッファーのvector行列に受信したbufferのmsg分を追加
			message_buffer.insert(message_buffer.end(), buffer, buffer + bytes_read);
			//受信したメッセージを文字列に変換
			std::string recv_msg(message_buffer.begin(), message_buffer.end());
			//完全なメッセージが解析された場合は中身を表示
			LOG_DEBUG("[ProtocolHandler] message receive success, received message size : {} message_content : {}", bytes_read, recv_msg);
			recv_flag = true;
			return std::make_tuple(recv_msg, disconnect_flag, recv_flag);

		}
		//受信したバイト数が0の場合はクライアントを切断する処理に移行する
		else if (bytes_read == 0) {
			disconnect_flag = true;
			return std::make_tuple(std::string(), disconnect_flag, recv_flag);

		}
		//上記の判定以外の場合の処理
		else
		{
			int recv_error = SSL_get_error(client_ssl, bytes_read);
			if (recv_error == SSL_ERROR_WANT_READ || recv_error == SSL_ERROR_WANT_WRITE) {
				continue;
			}
			//0がリターンされてきた場合の処理について
			else if (recv_error == SSL_ERROR_ZERO_RETURN) {
				disconnect_flag = true;
				return std::make_tuple(std::string(), disconnect_flag, recv_flag);

			}
			//システムコール関連のエラーが発生した場合
			else if (recv_error == SSL_ERROR_SYSCALL) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					LOG_ERROR("[ProtocolHandler] message receive failed, SSL read timeout occured.");
					return std::make_tuple(std::string(), disconnect_flag, recv_flag);
				}
				else {
					LOG_ERROR("[ProtocolHandler] message receive failed, SSL read syscall error occured.");
					return std::make_tuple(std::string(), disconnect_flag, recv_flag);
				}
			}
			//それ以外の理由
			else {
				LOG_ERROR("[ProtocolHandler] message receive failed. error code : {}", recv_error);
				return std::make_tuple(std::string(), disconnect_flag, recv_flag);
			}

		}
	}
}

std::tuple<json, bool, bool> ProtocolHandler::receive_message_json(SSL *client_ssl)
{
	json recv_msg_json;
	char buffer[MESSAGE_SIZE];
	int message_size = 0;
	std::vector<uint8_t> message_buffer;
	bool disconnect_flag = false;	//接続断の判定フラグ
	bool recv_flag = false;	//受信完了の判定フラグ
	int bytes_read = 0;

	if (client_ssl == nullptr || SSL_is_init_finished(client_ssl) == false) {
		LOG_ERROR("SSL is not initialized, cannot send message.");
		disconnect_flag = true;
		return std::make_tuple(json(), disconnect_flag, recv_flag);
	}

	while (true)
	{
		bytes_read = SSL_read(client_ssl, buffer, sizeof(buffer));
		message_size += bytes_read;

		//受信したバイト数が0より大きい場合msgの解析を実施する。
		if (bytes_read > 0) {
			//メッセージバッファーのvector行列に受信したbufferのmsg分を追加
			message_buffer.insert(message_buffer.end(), buffer, buffer + bytes_read);
			try
			{
				//jsonメッセージの解析
				recv_msg_json = json::from_msgpack(message_buffer);
				//完全なメッセージが解析された場合は中身を表示
				LOG_DEBUG("[ProtocolHandler] message receive success, received message size : {} message content : {}", message_size, recv_msg_json.dump(4));
				
				// JSON解析が成功したので即座にreturn
				recv_flag = true;
				disconnect_flag = false;  // 明示的にfalse設定
				LOG_DEBUG("[ProtocolHandler] Returning SUCCESS: recv_flag=true, disconnect_flag=false, json_size={}", recv_msg_json.dump().length());
				return std::make_tuple(recv_msg_json, disconnect_flag, recv_flag);
			}
			catch (const json::parse_error& e)
			{
				//jsonが完全ではない場合、再度受信を繰り返す
				continue;
			}
			// この行は削除：JSON解析成功時は上でreturnされるため到達しない
			// recv_flag = true;
			// return std::make_tuple(recv_msg_json, disconnect_flag, recv_flag);

		}
		//受信したバイト数が0の場合はクライアントを切断する処理に移行する
		else if (bytes_read == 0) {
			disconnect_flag = true;
			return std::make_tuple(json(), disconnect_flag, recv_flag);

		}
		//上記の判定以外の場合の処理
		else
		{
			int recv_error = SSL_get_error(client_ssl, bytes_read);
			if (recv_error == SSL_ERROR_WANT_READ || recv_error == SSL_ERROR_WANT_WRITE) {
				continue;
			}
			//0がリターンされてきた場合の処理について
			else if (recv_error == SSL_ERROR_ZERO_RETURN) {
				disconnect_flag = true;
				return std::make_tuple(json(), disconnect_flag, recv_flag);

			}
			//システムコール関連のエラーが発生した場合
			else if (recv_error == SSL_ERROR_SYSCALL) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					LOG_ERROR("[ProtocolHandler] message receive failed, SSL read timeout occured.");
					return std::make_tuple(json(), disconnect_flag, recv_flag);
				}
				else {
					LOG_ERROR("[ProtocolHandler] message receive failed, SSL read syscall error occured.");
					return std::make_tuple(json(), disconnect_flag, recv_flag);
				}
			}
			//それ以外の理由
			else {
				LOG_ERROR("[ProtocolHandler] message receive failed. error code : {}", recv_error);
				return std::make_tuple(json(), disconnect_flag, recv_flag);
			}

		}
	}
}

bool ProtocolHandler::send_message_json(SSL* client_ssl, const json& message)
{
	LOG_TRACE;
	std::vector<uint8_t> message_buffer = json::to_msgpack(message);
	size_t message_size = message_buffer.size();
	size_t total_sent = 0;
	size_t chunk_size = 16384;  // 最大TLSレコードサイズ
	int ret = 0;

	if (client_ssl == nullptr || SSL_is_init_finished(client_ssl) == false) {
		LOG_ERROR("SSL is not initialized, cannot send message.");
		return false;
	}
	while (total_sent < message_size) {
		size_t to_send = (message_size - total_sent > chunk_size) ? chunk_size : (message_size - total_sent);
		ret = SSL_write(client_ssl, message_buffer.data() + total_sent, static_cast<int>(to_send));
		if (ret > 0) {
			total_sent += ret;
			continue;
		}
		// エラー発生時
		int err = SSL_get_error(client_ssl, ret);
		if (err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ) {
			// 一時的なエラー、リトライ
			this->print_ssl_error(client_ssl);
			spdlog::info("Temporary error (WANT_WRITE/WANT_READ), retrying...");
			continue;
		}
		else {
			// 致命的なエラー
			this->print_ssl_error(client_ssl);
			LOG_ERROR("SSL_write failed: {:d}", err);
			SSL_shutdown(client_ssl);
			return false;
		}
	}
	spdlog::info("SSL write success!!");
	if (!message.is_object() || message.is_null()) {
		spdlog::error("Invalid JSON: message is either null or not an object.");
	}
	else {
		spdlog::debug("message json content : {}", message.dump(4));
	}
	return true;
}

bool ProtocolHandler::shutdown_ssl(SSL* client_ssl)
{
	int ret = SSL_shutdown(client_ssl);
	if (ret == 1) {
		spdlog::debug("SSL shutdown success.");
	}
	else
	{
		this->print_ssl_error(client_ssl);
		return false;
	}
	SSL_free(client_ssl);
	return true;
}



const std::string ProtocolHandler::get_cipher_suite(SSL* ssl)
{
	return std::string(SSL_get_cipher(this->_server_ssl));
}

const std::tuple<std::string, std::string, std::string, std::string>  ProtocolHandler::get_peer_cetificate(SSL* ssl)
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

#ifdef _WIN32
	SSL_X509* server_cert = SSL_get_peer_certificate(ssl);
	if (!server_cert) {
		spdlog::warn("No client certificate received.");
		return std::make_tuple(cert_subject_name, cert_issuer_name, cert_not_before, cert_not_after);
	}
	// 証明書のサブジェクト名（発行対象）
	SSL_X509_NAME_oneline(SSL_X509_get_subject_name(server_cert), subject_name_buffer, sizeof(subject_name_buffer));
	cert_subject_name = std::string(subject_name_buffer);

	// 証明書の発行者（CA）
	SSL_X509_NAME_oneline(SSL_X509_get_issuer_name(server_cert), issuer_name_buffer, sizeof(issuer_name_buffer));
	cert_issuer_name = std::string(issuer_name_buffer);


	// 証明書の有効期限（開始日）
	auto cert_not_before_time = SSL_X509_get_notBefore(server_cert);
	auto before_time = SSL_ASN1_TIME_get_data(cert_not_before_time);
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
	auto cert_not_after_time = SSL_X509_get_notAfter(server_cert);
	auto after_time = SSL_ASN1_TIME_get_data(cert_not_after_time);
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
	SSL_X509_free(server_cert);
	spdlog::info(" * Server Certificate Subject Name : {:s}", cert_subject_name);
	spdlog::info(" * Server Certificate Issuer Name : {:s}", cert_issuer_name);
	spdlog::info(" * Server Certificate Not Before : {:s}", cert_not_before);
	spdlog::info(" * Server Certificate Not After : {:s}", cert_not_after);
	return std::make_tuple(cert_subject_name, cert_issuer_name, cert_not_before, cert_not_after);

#endif
	return std::make_tuple(cert_subject_name, cert_issuer_name, cert_not_before, cert_not_after);
}

void ProtocolHandler::print_json_message(const json &json_message)
{
	if (json_message.is_null() || json_message.empty())
	{
		LOG_ERROR("Invalid JSON object.");
		return;
	}
	spdlog::debug("Receive JSON DATA :\n{:s}", json_message.dump(4));
}

void ProtocolHandler::print_received_message_info(const uint64_t message_id, const std::string &sender_id, const std::string &room_id, const std::string &sender_ipv4, const socket_t socket_num, const json &json_message)
{
	if (json_message.is_null() || json_message.empty())
	{
		LOG_ERROR("Invalid JSON object.");
		return;
	}
	spdlog::debug("server received Message(message id:{}) from sender(id:{}, ip:{}, socket:{}).\nReceive JSON DATA :\n{}", message_id, sender_id, sender_ipv4, socket_num, json_message.dump(4));
}

void ProtocolHandler::hot_reload_certificates(int interval_seconds, const std::string& cert_file_pass, const std::string& private_key_file_pass)
{
	
	spdlog::info("Starting certificate hot reload thread with interval: {} seconds", interval_seconds);
	while (this->_running)  // サーバーが実行中の間ループ
	{
		std::unique_lock<std::mutex> lock(_reload_mutex);
		// interval_seconds 秒待機（途中で停止通知があれば即座に戻る）
		bool stopped = _reload_cv.wait_for(lock, std::chrono::seconds(interval_seconds), [this](){ return !_running.load(); });
		if (stopped) {
			// _running == false になったらループ終了
			break;
		}
		lock.unlock(); // ロックを早期解除

		bool cert_changed, key_changed;
		{
			std::lock_guard<std::mutex> mtime_lock(_mtime_mutex);
			cert_changed = modify_file(cert_file_pass, _cert_mtime);
			key_changed = modify_file(private_key_file_pass, _private_key_mtime);
		}
		if (cert_changed || key_changed)
		{
			LOG_DEBUG("[ProtocolHandler] Certificate or private key file changed, reloading...");
			auto new_ctx = std::shared_ptr<SSL_CTX>(SSL_CTX_new(TLS_server_method()), SSL_CTX_Deleter{});
			if (!new_ctx)
			{
				LOG_ERROR("[ProtocolHandler] Failed to create new SSL_CTX.");
				continue;
			}
			if (!reload_certificates(new_ctx.get(), cert_file_pass, private_key_file_pass))
			{
				LOG_ERROR("[ProtocolHandler] Failed to reload certificates.");
				std::this_thread::sleep_for(std::chrono::seconds(5));  // 5秒待機してから再試行
				continue;
			}
			{
				std::lock_guard<std::mutex> lock(this->_ctx_mutex);
				this->_server_ctx = std::move(new_ctx);	//新しいCTXを設定
				LOG_DEBUG("[ProtocolHandler] SSL_CTX reloaded successfully.");
			}
		}
	}
}

void ProtocolHandler::stop_hot_reload()
{
	this->_running = false; // サーバーを停止するフラグを設定
	_reload_cv.notify_all(); // 待機中のスレッドに通知して即座に終了させる
	LOG_DEBUG("[ProtocolHandler] Certificate hot reload thread stopped.");
}

// SSL通信で発生するエラーの表示
void ProtocolHandler::print_ssl_error(SSL* client_ssl)
{
	int err = 0;
	char buffer[80];
	err = SSL_get_error(client_ssl, 0);
	ERR_error_string(err, buffer);
	LOG_ERROR("[ProtocolHandler] SSL error content : {:s} (code : {:d}.)", std::string(buffer), err);
}


const SSL_METHOD* ProtocolHandler::set_server_method()
{
#ifdef SERVER_METHOD_SSLV23
	//TLS接続時のバージョン指定(v23＝サーバー及びクライアントがサポートする最も高いバージョンを指定)
	const auto server_method = TLS_server_method();
	if (server_method == NULL) {
		LOG_ERROR("[ProtocolHandler] TLS server method, failed, unable to get method.");
		exit(1);
	}
	else
	{
		LOG_DEBUG("[ProtocolHandler] server_method set v23 for TLS context.");
		return server_method;
	}
#endif // SERVER_METHOD_SSLV23

#ifdef SERVER_METHOD_TLS_1_3
	//TLS接続時のバージョン指定(v23＝サーバー及びクライアントがサポートする最も高いバージョンを指定)
	const auto server_method = TLSv1_3_server_method();
	if (server_method == NULL) {
		LOG_ERROR("[ProtocolHandler] TLS v1.3 server method, failed, unable to get method.");
		exit(1);
	}
	else
	{
		LOG_DEBUG("[ProtocolHandler] server_method set TLS 1.3  context.");
		return server_method;
	}

#endif // server_method_TLS1_3

#ifdef SERVER_METHOD_TLS_1_2
	//TLS接続時のバージョン指定(v23＝サーバー及びクライアントがサポートする最も高いバージョンを指定)
	const auto server_method = TLSv1_2_server_method();
	if (server_method == NULL) {
		LOG_ERROR("[ProtocolHandler] TLS v1.2 server method, failed, unable to get method.");
		exit(1);
	}
	else
	{
		LOG_DEBUG("[ProtocolHandler] server_method set TLS 1.2 context.");
		return server_method;
	}

#endif // SERVER_METHOD_TLS_1_2

#ifdef SERVER_METHOD_TLS_1_1
	//TLS接続時のバージョン指定(v23＝サーバー及びクライアントがサポートする最も高いバージョンを指定)
	const auto server_method = TLSv1_1_server_method();
	if (server_method == NULL) {
		LOG_ERROR("[ProtocolHandler] TLS v1.1 server method, failed, unable to get method.");
		exit(1);
	}
	else
	{
		LOG_DEBUG("[ProtocolHandler] server_method set TLS 1.1 context.");
		return server_method;
	}
#endif // SERVER_METHOD_TLS_1_1

	//どれも定義されていない場合の安全対策
	LOG_ERROR("[ProtocolHandler] No valid TLS method defined. Please define SERVER_METHOD_* macro.");
	exit(1);  // 明示的に終了して UB を防ぐ
	return nullptr;  // これは念のため
}

void ProtocolHandler::generate_ssl_context(const SSL_METHOD* method)
{
	SSL_CTX* raw_ctx = SSL_CTX_new(method);
    if (!raw_ctx) {
        LOG_ERROR("[ProtocolHandler] SSL context creation failed.");
        exit(1);
    }
    this->_server_ctx = std::shared_ptr<SSL_CTX>(raw_ctx, SSL_CTX_Deleter{});
}


void ProtocolHandler::load_cert_file(const std::string &cert_file_pass)
{
		//サーバ証明書のロード
	if (SSL_CTX_use_certificate_file(_server_ctx.get(), cert_file_pass.c_str(), SSL_FILETYPE_PEM) == 0) {
		LOG_ERROR("[ProtocolHandler] Failed to load server certificate file: {}", cert_file_pass);
		exit(1);
	}
}

void ProtocolHandler::load_private_key(const std::string &private_key_pass)
{
		//プライベート鍵のロード
	if (SSL_CTX_use_PrivateKey_file(_server_ctx.get(), private_key_pass.c_str(), SSL_FILETYPE_PEM) == 0) {
		LOG_ERROR("[ProtocolHandler] Failed to load server private key file: {}", private_key_pass);
		exit(1);
	}
}

//証明書の変更を監視
bool ProtocolHandler::modify_file(const std::string& cert_file_pass, time_t &last_mtime)
{ 
	struct stat st;
    if (stat(cert_file_pass.c_str(), &st) != 0) {
        consecutive_errors++;
        if (consecutive_errors > 3) {
            LOG_ERROR("[ProtocolHandler] Persistent failure accessing certificate file '{}': errno={}, {}", cert_file_pass, errno, std::strerror(errno));
        } else {
            LOG_WARNING("[ProtocolHandler] Temporary failure accessing certificate file '{}': errno={}, {}", cert_file_pass, errno, std::strerror(errno));
        }
        return false;
    }
    // 成功したら連続エラーカウントをリセット
    consecutive_errors.store(0);

    if (st.st_mtime > last_mtime) {
        last_mtime = st.st_mtime;
        return true;
    }
    return false;
}

bool ProtocolHandler::reload_certificates(SSL_CTX* ctx, const std::string &cert_file, const std::string &key_file)
{
	//証明書の再ロード
    if (SSL_CTX_use_certificate_file(ctx, cert_file.c_str(), SSL_FILETYPE_PEM) <= 0){
		LOG_ERROR("[ProtocolHandler] Failed to reload server certificate file: {}", cert_file);
		return false;
	}
	//秘密鍵の再ロード
    if (SSL_CTX_use_PrivateKey_file(ctx, key_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
		LOG_ERROR("[ProtocolHandler] Failed to reload server private key file: {}", key_file);
		return false;
	}
	//秘密鍵とCTXにロードされた証明書との整合性をチェック
    if (!SSL_CTX_check_private_key(ctx)) {
		LOG_ERROR("[ProtocolHandler] Server private key does not match the certificate public key.");
		return false;
	}
	LOG_DEBUG("[ProtocolHandler] Server certificate and private key reloaded successfully.");
    return true;
}
