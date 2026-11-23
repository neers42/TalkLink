#ifndef AUNTHENTICATIONSERVERFACADE_HPP
#define AUNTHENTICATIONSERVERFACADE_HPP
#include "common.h"
#include "Domain/Messages/AuthRequestMessage.hpp"
#include "Domain/Messages/AuthResponseMessage.hpp"
#include <thread>
#include <memory>

namespace grpc { class Server; }

#define DISCONNECT_SOCKET 0
#define ACCEPT_FAILED 0
#define ACCEPT_SUCCEEDED 1
#define SOCKET_CONFIG_FILE_PATH "/app/AuthenticationServer/conf/socket_setting.conf"
#define AUTH_CONFIG_FILE_PATH "/app/AuthenticationServer/conf/auth.conf"

class RedisAdapter;
class FirebaseTokenAdapter;
class SessionTokenHandler;
class AuthenticationUseCase;
class SessionGrpcAdapter;

class AuthenticationServerFacade
{
//メンバー変数
private:
	std::unique_ptr<SocketHandler> _socket_handler;
	std::unique_ptr<ThreadPoolHandler> _thread_pool_handler;
	std::unique_ptr<TLSConnectionHandler> _tlscon_handler;
	std::shared_ptr<RedisAdapter> _redis_adapter; // セッション保存用(AuthenticationUseCaseとSessionGrpcAdapterで共有)
	std::shared_ptr<AuthenticationUseCase> _authentication_use_case; // JWT検証->セッション発行のオーケストレーション
	std::unique_ptr<SessionGrpcAdapter> _session_grpc_adapter; // ChatServer向けセッション検証/削除APIの実装
	std::unique_ptr<grpc::Server> _grpc_server;
	std::thread _grpc_server_thread;
	std::map<socket_t, SSL*> _ssl_list;
	std::string _cert_path; // 環境変数TLS_CERT_PATHから取得した証明書パス
	std::string _key_path;  // 環境変数TLS_KEY_PATHから取得した秘密鍵パス

public:
	std::shared_mutex smtx;


//メンバー関数
public:
	AuthenticationServerFacade();
	~AuthenticationServerFacade();
	void run();


private:
	//シーケンス関連
	void start_grpc_server();
	void allocate_process(const socket_t socket);
	bool accept_client_socket_process();
	void auth_handle_process(const socket_t socket);
	void handle_disconnect_process(const socket_t socket);
	json make_auth_response(const std::string& status, const std::uint64_t ttl);
};



#endif // !AUNTHENTICATIONSERVERFACADE_HPP

