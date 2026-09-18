#include "AunthenticationServerFacade.hpp"
#include "RedisAdapter.hpp"
#include "TokenHandler.hpp"
#include "SessionTokenHandler.hpp"
#include "EnvHelper.hpp"

AunthenticationServerFacade::AunthenticationServerFacade()
{
    ////ログのフォーマット設定
	spdlog::set_level(spdlog::level::debug);
	// Use fixed-width level field so info/debug align visually.
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread id : %t] [%^%-5l%$] %v");
	LOG_INFO("Starting TalkLink Auth-server version {}!!", TALKLINK_VERSION);
	const std::string cert_path = get_env("TLS_CERT_PATH");
	const std::string key_path  = get_env("TLS_KEY_PATH");
	_socket_handler = std::make_unique<SocketHandler>(SOCKET_CONFIG_FILE_PATH);
	_thread_pool_handler = std::make_unique<ThreadPoolHandler>(10);
	_tlscon_handler = std::make_unique<TLSConnectionHandler>(cert_path, key_path);
	_redis_adapter = std::make_unique<RedisAdapter>();
	_token_handler = std::make_unique<TokenHandler>();
	_session_token_handler = std::make_unique<SessionTokenHandler>();
	
	if (_socket_handler == nullptr
		|| _thread_pool_handler == nullptr
		|| _tlscon_handler == nullptr
		|| _redis_adapter == nullptr
		|| _token_handler == nullptr
		|| _session_token_handler == nullptr)
	{
		LOG_ERROR("Failed to initialize AunthenticationServerFacade components. Please check the configuration and try again.");
		exit(1);
	}
}

AunthenticationServerFacade::~AunthenticationServerFacade()
{
	this->_ssl_list.clear();
}

void AunthenticationServerFacade::run() {
	LOG_TRACE;       
	//リッスンソケットの生成
	if (_socket_handler->open_tcp_listen_socket() == false) {
		LOG_ERROR("The TCP listening socket for message could not be opened properly, so the program is forcibly terminated. Please set it again.");
		exit(1);
	}

	if (_socket_handler->open_udp_multicast_tx_socket() == false) {
		LOG_ERROR("The program terminates because it failed to create a socket for multicast transmission. Please reset the settings.");
		exit(1);
	}
	this->_thread_pool_handler->enqueue_task([this](){ _tlscon_handler->hot_reload_certificates(60, SERVER_CERT_FILE_PATH, SERVER_PRIVATEKEY_FILE_PATH); });
	LOG_INFO("Start monitoring requests from clients.");
	//ディスクリプション監視スタート
	while (true) {
		// クライアントからの接続を待機
#ifdef USE_POLL
		_socket_handler->monitor_socket_descriptor([this](socket_t changed_socket)
												   { allocate_process(changed_socket); });
#endif
	}
}


void AunthenticationServerFacade::allocate_process(const socket_t socket) {
	// socketがINVALIDの場合は処理をしない
	if (socket == INVALID_SOCKET_FD)
	{
		return;
	}
	if (socket == this->_socket_handler->get_listen_socket())
	{
		//this->accept_client_socket_process();
		this->_thread_pool_handler->enqueue_task([this]()
												 { accept_client_socket_process(); });
	}
	else
	{
		// 既存クライアントのデータを処理
		this->_thread_pool_handler->enqueue_task([this, socket]()
												 { auth_handle_process(socket); });
	}
}

bool AunthenticationServerFacade::accept_client_socket_process() {
	LOG_TRACE;
	socket_t new_client_socket = INVALID_SOCKET_FD;
	struct sockaddr_in new_client_sockaddr_in = {0};
	socklen_t new_client_socklen_t = 0;
	std::string new_client_ipv4_addr;

	// TCPソケットのアクセプト処理
	std::tie(new_client_socket, new_client_sockaddr_in) = _socket_handler->accept_user_socket(sizeof(new_client_sockaddr_in));
	if (new_client_socket == INVALID_SOCKET_FD)
	{
		LOG_ERROR("The accepted socket for message could not be added to the list because it was an INVALID SOCKET");
		return ACCEPT_FAILED;
	}
	else
	{
		spdlog::info("accept success!! client socket :{}", new_client_socket);
		new_client_ipv4_addr = this->_socket_handler->extraction_user_ip_address(new_client_sockaddr_in, sizeof(new_client_ipv4_addr));
	}
#ifdef USE_POLL
	// ノンブロッキング設定（fcntl利用例）
	int flags = fcntl(new_client_socket, F_GETFL, 0);
	if (flags == -1)
	{
		LOG_ERROR("fcntl get flags failed");
		return ACCEPT_FAILED;
	}
#endif

	// SSLアクセプト処理
	SSL* new_client_ssl = this->_tlscon_handler->ssl_accept(new_client_socket);
	if (new_client_ssl == nullptr)
	{
		LOG_ERROR("client ssl is not accepted(socket:{})", new_client_socket);
		SSL_free(new_client_ssl);
		return ACCEPT_FAILED;
	}
	else
	{
		this->_tlscon_handler->get_peer_cetificate(new_client_ssl);
		spdlog::info("client ip : {} , success, server accept client(socket:{}).", new_client_ipv4_addr, new_client_socket);
#ifdef USE_POLL
		//アクセプトしたクライアントの通知を検知できるようにepollの設定を実施
		this->_socket_handler->add_epoll_ctrl(new_client_socket);
#endif
		_ssl_list.insert(std::make_pair(new_client_socket, new_client_ssl));
		return ACCEPT_SUCCEEDED;
	}
}

void AunthenticationServerFacade::auth_handle_process(const socket_t socket) 
{
	std::string receive_token = "", uid = "";
	std::uint64_t expire = 0, ttl = 0;
	bool disconnect_flag = false;
	bool recv_flag = false;
	bool verify_result = false;
	json response;
	json receive_json;
	std::string _message_type = "";
	std::string session_token = "";

	try
	{
		auto it = _ssl_list.find(socket);
		if (it == _ssl_list.end()) {
			LOG_ERROR("SSL object for socket {} not found.", socket);
			return;
		}
		SSL* client_ssl = it->second;
		if (client_ssl == nullptr) {
			LOG_ERROR("User SSL is null, cannot process message.");
			return;
		}

		// メッセージの受信
		std::tie(receive_json, disconnect_flag, recv_flag) = this->_tlscon_handler->receive_message_json(client_ssl);

		//disconnect_flagがfalseの場合、epoll_ctrlにONESHOTを登録する。
		if (disconnect_flag == false)
		{
#ifdef USE_POLL
			this->_socket_handler->reactivate_epoll_ctrl(socket);
#endif
		}
		// クライアントの切断を検知した場合
		else
		{
			LOG_INFO("client(socket : {}) is disconnected, server initialize client info.", socket);
			this->handle_disconnect_process(socket);
			return;
		}

		if(receive_json.empty() == true){
			LOG_ERROR("Received empty message from client.");
			return;
		}
		// JSONメッセージの処理
		auto mt = MessageBase::parse_message_type(receive_json.at("message_type").get<std::string>());
		_message_type = receive_json["message_type"];
		if (mt == MessageBase::MessageType::AUTHENTICATION_REQUEST)
		{
			auto auth_request_instance = MessageBase::deserialize_message<AuthRequestMessage>(receive_json);
			if (auth_request_instance == nullptr) {
				LOG_ERROR("Failed to deserialize AuthRequestMessage from received JSON.");
				return;
			}
			receive_token = auth_request_instance->get_jwt();
			verify_result = this->verify_auth_process(receive_token, uid, expire, ttl, session_token);
			if(verify_result == true){
				LOG_INFO("Authentication succeeded for client(socket : {}). CID: {}, Expire: {}, TTL: {}.", socket, uid, expire, ttl);
			}else{
				LOG_ERROR("Authentication failed for client(socket : {}).", socket);
			}
			auto response_instance = std::make_unique<AuthResponseMessage>(MessageBase::CommonField(
				MessageBase::MessageType::AUTHENTICATION_RESPONSE, 
				auth_request_instance->get_correlation_id()), 
				verify_result, 
				expire, 
				session_token);
				
			if(response_instance == nullptr) {
					LOG_ERROR("Failed to create AuthResponseMessage instance for failure response.");
					return;
			}
			response = MessageBase::serialize_message(response_instance);

			// レスポンスをクライアントに送信する
			if (this->_tlscon_handler->send_message_json(client_ssl, response) == true)
			{
				LOG_INFO("Sent response to client(socket : {}).", socket);
			}
			else
			{
				LOG_ERROR("Failed to send response to client(socket : {}).", socket);
			}
		}
		else
		{
			LOG_ERROR("Invalid message type received.");
			return;
		}
		
	}
	catch (const std::runtime_error &re)
	{
		LOG_ERROR("Runtime error: {}", re.what());
	}
	catch (const std::exception &e)
	{
		LOG_ERROR(e.what());
	}
}


void AunthenticationServerFacade::handle_disconnect_process(const socket_t socket) 
{
	auto it = _ssl_list.find(socket);
		if (it == _ssl_list.end()) {
			LOG_ERROR("SSL object for socket {} not found.", socket);
			return;
		}
		SSL *client_ssl = it->second;
		if(client_ssl == nullptr) {
			LOG_ERROR("User SSL is null, cannot process message.");
			return;
		}

#ifdef USE_POLL
	this->_socket_handler->delete_epoll_ctrl(socket);
	// **ここでブロッキングモードに切り替え**
	int flags = fcntl(socket, F_GETFL, 0);
	if (flags != -1)
	{
		fcntl(socket, F_SETFL, flags & ~O_NONBLOCK);
	}
#endif
	this->_tlscon_handler->shutdown_ssl(client_ssl);
	this->_socket_handler->close_socket(socket);
	spdlog::info("User with socket {} has been disconnected and cleaned up.", socket);
	_ssl_list.erase(it);
}


json AunthenticationServerFacade::make_auth_response(const std::string &status, const std::uint64_t ttl)
{
	json response;
	response["status"] = status;
	if (ttl > 0)
	{
		response["ttl"] = ttl;
	}else{
		return json();
	}
	return response;
}

bool AunthenticationServerFacade::verify_auth_process(
	const std::string &jwt, 
	std::string &uid, 
	std::uint64_t &expire, 
	std::uint64_t& ttl, 
	std::string& session_token)
{
	try
	{
		// 検証を実施し、トークンが有効な場合の処理
		if (this->_token_handler->verify_token(jwt, uid, expire) == true)
		{
			session_token = _session_token_handler->generate_token();
			auto session_token_hash = _session_token_handler->hash_token(session_token);

			if (this->_redis_adapter->save_session(uid, session_token_hash, expire, ttl) == true)
			{
				spdlog::info("Session saved in Redis for CID: {} with TTL: {}.", uid, ttl);
				return true;
			}
			else
			{
				LOG_ERROR("Failed to save session in Redis for CID: {}.", uid);
				return false;
			}
		}
		else
		{
			LOG_ERROR("Invalid jwt received from client(socket : {}).", socket);
			return false;
		}
	}
	catch (const std::exception &e)
	{
		LOG_ERROR("Exception : {}", e.what());
		return false;
	}
}
