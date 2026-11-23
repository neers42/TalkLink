#include "AuthenticationServiceFacade.hpp"
#include "Infrastructure/RedisAdapter.hpp"
#include "Infrastructure/FirebaseTokenAdapter.hpp"
#include "Infrastructure/SessionGrpcAdapter.hpp"
#include "Domain/SessionTokenHandler.hpp"
#include "Application/UseCases/AuthenticationUseCase.hpp"
#include "EnvHelper.hpp"
#include "inicpp.h"
#include <grpcpp/grpcpp.h>
#include <cstdlib>

AuthenticationServiceFacade::AuthenticationServiceFacade()
{
    ////ログレベルの設定
	// NDEBUGはCMakeがRelease/MinSizeRelビルドで自動的に定義するマクロ。
	// Debugビルド(NDEBUG未定義)ではdebug、それ以外(本番相当)ではinfoにする。
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#else
	spdlog::set_level(spdlog::level::info);
#endif // NDEBUG
	////ログのフォーマット設定
	// Use fixed-width level field so info/debug align visually.
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread id : %t] [%^%-5l%$] %v");
	LOG_INFO("Starting TalkLink Auth-service version {}!!", TALKLINK_VERSION);
	_cert_path = get_env("TLS_CERT_PATH");
	_key_path  = get_env("TLS_KEY_PATH");
	_socket_handler = std::make_unique<SocketHandler>(SOCKET_CONFIG_FILE_PATH);
	_thread_pool_handler = std::make_unique<ThreadPoolHandler>(10);
	_tlscon_handler = std::make_unique<TLSConnectionHandler>(_cert_path, _key_path);

	// auth.confの読み込み(Firebaseプロジェクト設定)
	ini::IniFile auth_config;
	auth_config.load(AUTH_CONFIG_FILE_PATH);
	const std::string firebase_project_id = auth_config["Firebase"]["project_id"].as<std::string>();

	// JWT検証->セッション発行のオーケストレーションはAuthenticationUseCaseに委譲する。
	// (本番用の実装をここで組み立てる。テストではAuthenticationUseCaseを
	//  フェイク実装と直接構築することで、本物のRedis/Firebase証明書無しに
	//  検証ロジックだけをテストできる)
	// _redis_adapterはAuthenticationUseCaseとSessionGrpcAdapter(ChatService向け
	// セッション検証API)の両方から共有される、唯一のセッションストアへの入口。
	_redis_adapter = std::make_shared<RedisAdapter>();
	_authentication_use_case = std::make_shared<AuthenticationUseCase>(
		std::make_shared<FirebaseTokenAdapter>(firebase_project_id),
		_redis_adapter,
		std::make_shared<SessionTokenHandler>()
	);

	if (_socket_handler == nullptr
		|| _thread_pool_handler == nullptr
		|| _tlscon_handler == nullptr
		|| _authentication_use_case == nullptr)
	{
		LOG_ERROR("Failed to initialize AuthenticationServiceFacade components. Please check the configuration and try again.");
		exit(1);
	}

	start_grpc_server();
}

AuthenticationServiceFacade::~AuthenticationServiceFacade()
{
	if (_grpc_server) {
		_grpc_server->Shutdown();
	}
	if (_grpc_server_thread.joinable()) {
		_grpc_server_thread.join();
	}
	this->_ssl_list.clear();
}

void AuthenticationServiceFacade::start_grpc_server()
{
	// ChatServiceはこのポート経由でのみセッション情報にアクセスする(Redis直接参照はしない)。
	// ポートは内部ネットワーク限定で、docker-compose上もホストには公開しない想定。
	const char* env_port = std::getenv("AUTH_GRPC_PORT");
	const std::string grpc_port = env_port ? env_port : "50010";
	const std::string server_address = "0.0.0.0:" + grpc_port;

	_session_grpc_adapter = std::make_unique<SessionGrpcAdapter>(_redis_adapter);

	grpc::ServerBuilder builder;
	// TODO: 本番ではgrpc::SslServerCredentials(既存のTLS証明書)に切り替える。
	// 現状はInsecureServerCredentials(内部ネットワーク限定の想定)。
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(_session_grpc_adapter.get());

	_grpc_server = builder.BuildAndStart();
	if (!_grpc_server) {
		LOG_ERROR("Failed to start SessionService gRPC server on {}", server_address);
		exit(1);
	}

	LOG_INFO("SessionService gRPC server listening on {}", server_address);

	_grpc_server_thread = std::thread([this]() {
		_grpc_server->Wait();
	});
}

void AuthenticationServiceFacade::run() {
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
	this->_thread_pool_handler->enqueue_task([this](){ _tlscon_handler->hot_reload_certificates(60, _cert_path, _key_path); });
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


void AuthenticationServiceFacade::allocate_process(const socket_t socket) {
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

bool AuthenticationServiceFacade::accept_client_socket_process() {
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
		LOG_INFO("accept success!! client socket :{}", new_client_socket);
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
		LOG_INFO("client ip : {} , success, server accept client(socket:{}).", new_client_ipv4_addr, new_client_socket);
#ifdef USE_POLL
		//アクセプトしたクライアントの通知を検知できるようにepollの設定を実施
		this->_socket_handler->add_epoll_ctrl(new_client_socket);
#endif
		_ssl_list.insert(std::make_pair(new_client_socket, new_client_ssl));
		return ACCEPT_SUCCEEDED;
	}
}

void AuthenticationServiceFacade::auth_handle_process(const socket_t socket) 
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
			verify_result = this->_authentication_use_case->verify_auth_process(receive_token, uid, expire, ttl, session_token);
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


void AuthenticationServiceFacade::handle_disconnect_process(const socket_t socket) 
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
	LOG_INFO("User with socket {} has been disconnected and cleaned up.", socket);
	_ssl_list.erase(it);
}


json AuthenticationServiceFacade::make_auth_response(const std::string &status, const std::uint64_t ttl)
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

