#ifndef AUNTHENTICATIONSERVERFACADE_HPP
#define AUNTHENTICATIONSERVERFACADE_HPP
#include "common.h"
#include "Messages/AuthRequestMessage.hpp"
#include "Messages/AuthResponseMessage.hpp"
#define DISCONNECT_SOCKET 0
#define ACCEPT_FAILED 0
#define ACCEPT_SUCCEEDED 1
#define SOCKET_CONFIG_FILE_PATH "/app/AuthenticationServer/conf/socket_setting.conf"

class RedisAdapter;
class TokenHandler;
class SessionTokenHandler;

class AuthenticationServerFacade
{
//メンバー変数
private:
	std::unique_ptr<SocketHandler> _socket_handler;
	std::unique_ptr<ThreadPoolHandler> _thread_pool_handler;
	std::unique_ptr<TLSConnectionHandler> _tlscon_handler;
	std::unique_ptr<RedisAdapter> _redis_adapter; // Redisクライアント
	std::unique_ptr<TokenHandler> _token_handler; // JWTトークンハンドラー
	std::unique_ptr<SessionTokenHandler> _session_token_handler; //Session用のトークンハンドラー
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
	bool verify_auth_process(const std::string& jwt, std::string& uid, std::uint64_t& expire, std::uint64_t& ttl, std::string& session_token);
};



#endif // !AUNTHENTICATIONSERVERFACADE_HPP

