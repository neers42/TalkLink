#ifndef AUNTHENTICATIONSERVERFACADE_HPP
#define AUNTHENTICATIONSERVERFACADE_HPP
#include "common.h"
#include "RedisAdapter.hpp"
#include "TokenHandler.hpp"
#include "Messages/AuthRequestMessage.hpp"
#include "Messages/AuthResponseMessage.hpp"
#define DISCONNECT_SOCKET 0
#define ACCEPT_FAILED 0
#define ACCEPT_SUCCEEDED 1
#define SOCKET_CONFIG_FILE_PATH "/app/AuthenticationServer/conf/socket_setting.ini"
#define SERVER_CERT_FILE_PATH "/app/AuthenticationServer/certs/server-cert.pem"
#define SERVER_PRIVATEKEY_FILE_PATH "/app/AuthenticationServer/certs/server-key.pem"
class AunthenticationServerFacade
{
//メンバー変数
private:
	std::unique_ptr<SocketHandler> _socket_handler;
	std::unique_ptr<ThreadPoolHandler> _thread_pool_handler;
	std::unique_ptr<TLSConnectionHandler> _tlscon_handler;
	std::unique_ptr<RedisAdapter> _redis_adapter; // Redisクライアント
	std::unique_ptr<TokenHandler> _token_handler; // JWTトークンハンドラー
	std::map<socket_t, SSL*> _ssl_list;

public:
	std::shared_mutex smtx;


//メンバー関数
public:
	AunthenticationServerFacade();
	~AunthenticationServerFacade();
	void run();


private:
	//シーケンス関連
	void allocate_process(const socket_t socket);
	bool accept_client_socket_process();
	void auth_handle_process(const socket_t socket);
	void handle_disconnect_process(const socket_t socket);
	json make_auth_response(const std::string& status, const std::uint64_t ttl);
	bool verify_auth_process(const std::string& token, std::string& uid, std::uint64_t& expire, std::uint64_t& ttl);
};



#endif // !AUNTHENTICATIONSERVERFACADE_HPP

