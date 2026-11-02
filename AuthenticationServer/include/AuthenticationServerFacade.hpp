#ifndef AUNTHENTICATIONSERVERFACADE_HPP
#define AUNTHENTICATIONSERVERFACADE_HPP
#include "common.h"
#include "Domain/Entity/Messages/AuthRequestMessage.hpp"
#include "Domain/Entity/Messages/AuthResponseMessage.hpp"
#define DISCONNECT_SOCKET 0
#define ACCEPT_FAILED 0
#define ACCEPT_SUCCEEDED 1
#define SOCKET_CONFIG_FILE_PATH "/app/AuthenticationServer/conf/socket_setting.conf"
#define AUTH_CONFIG_FILE_PATH "/app/AuthenticationServer/conf/auth.conf"

class RedisAdapter;
class TokenHandler;
class SessionTokenHandler;
class AuthenticationService;

class AuthenticationServerFacade
{
//メンバー変数
private:
	std::unique_ptr<SocketHandler> _socket_handler;
	std::unique_ptr<ThreadPoolHandler> _thread_pool_handler;
	std::unique_ptr<TLSConnectionHandler> _tlscon_handler;
	std::shared_ptr<AuthenticationService> _authentication_service; // JWT検証->セッション発行のオーケストレーション
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
	void allocate_process(const socket_t socket);
	bool accept_client_socket_process();
	void auth_handle_process(const socket_t socket);
	void handle_disconnect_process(const socket_t socket);
	json make_auth_response(const std::string& status, const std::uint64_t ttl);
};



#endif // !AUNTHENTICATIONSERVERFACADE_HPP

