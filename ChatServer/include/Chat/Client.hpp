#ifndef CLIENT_HPP
#define CLIENT_HPP
#ifdef _WIN32
	#include <WinSock2.h>
	#include <ws2tcpip.h>
	using socket_t = SOCKET;
	#define INVALID_SOCKET_FD INVALID_SOCKET
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <sys/select.h>
	#include <netdb.h>
	using socket_t = int;
	#define INVALID_SOCKET_FD -1
#endif // _WIN32
#include "Utils/common.hpp"

class Client
{
public:
	enum class ConnectState{
		ACTIVE,
		DISCONNECTING,
		DISCONNECT
	};

	//Client private変数
private:
	std::string id;	//クライアントID
	std::string name;	//クライアントネーム
	std::string mail_address;	//クライアントのメールアドレス
	socket_t socket;		//クライアントのソケット
	struct sockaddr_in sockaddr_in;	//クライアントのsockaddr_in情報
	socklen_t socket_address_in_length = sizeof(this->sockaddr_in);
	std::string ipv4_address = "";		//クライアントの端末ipv4アドレス
	std::string password = "";			//クライアントのパスワード
	std::atomic<int> heart_beat_count;		//クライアントのハートビート回数
	std::atomic<bool> auth_status = false;	//クライアントの認証状態
	SSL* ssl;				//クライアントのソケットにアタッチされたsslオブジェクト
	std::atomic<ConnectState> c_state;

public:
	mutable std::mutex mtx;

public:
	explicit Client();
	explicit Client(const std::string id, const std::string name, const std::string mail_addr, const socket_t socket, const struct sockaddr_in sockaddr_in, const std::string client_ipv4_addr, SSL* ssl);
	explicit Client(const socket_t socket, const struct sockaddr_in sockaddr_in, const socklen_t length, std::string& ipv4_addr, SSL* ssl);
	~Client();


	//getter
	std::string get_id() const;
	std::string get_name() const;
	std::string get_mail_address() const;
	socket_t get_socket() const;
	struct sockaddr_in get_sockaddr_in() const;
	socklen_t get_socket_address_in_length() const;
	std::string get_ipv4_address() const;
	std::string get_password() const;
	int get_heart_beat_count() const;
	bool get_auth_status() const;
	SSL* get_ssl();
	ConnectState get_connect_state() const;


	//setter
	void set_id(const std::string& id);
	void set_name(const std::string& name);
	void set_mail_address(const std::string& mail_address);
	void set_socket(const socket_t socket);
	void set_sockaddr_in(const struct sockaddr_in sockaddr_in);
	void set_ipv4_address(const std::string& ipv4_addr);
	void set_password(const std::string& password);
	void set_heart_beat_count(const int count);
	void set_auth_status(const bool status);
	void set_ssl(SSL* ssl);
	void incliment_heart_beat_count();
	void clear_heart_beat_count();
	void change_auth_status();
	bool operator==(const Client& other_client)const;
	Client& operator=(const Client& other_client);
	void set_conncet_state(ConnectState state);

	//other
	bool isActive() const noexcept {return c_state == ConnectState::ACTIVE;}
	bool compare_and_set_state(ConnectState& expected, ConnectState desired);
};
#endif // !CLIENT_HPP



