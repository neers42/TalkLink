#ifndef CLIENT_HANDLER_H_
#define CLIENT_HANDLER_H_

#include <iostream>
#include <string>
#include <vector>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <memory>
#include <rpcdce.h>
#include "AppException.h"
#include "StringFormat.h"
#include "ServerTime.h"
#include "FirebaseAuthHandler.h"
#include "firebase/auth.h"
#include "common.h"
#include "wolfssl/ssl.h"
#define MAX_CONNECT_NUM 50000
#define NOT_EXIST_CLIENT_ID -1

//#define DEBUG
#define DEBUG_UUID
class ClientHandler
{
private:

	class Client
	{

		//Client private変数
	public:
		std::string id;	//クライアントID
		std::string name;	//クライアントネーム
		std::string mail_address;	//クライアントのメールアドレス
		SOCKET socket;		//クライアントのソケット
		struct sockaddr_in sockaddr_in;	//クライアントのsockaddr_in情報
		socklen_t socket_address_in_length = sizeof(this->sockaddr_in);
		std::string ipv4_address = "";		//クライアントの端末ipv4アドレス
		std::string password = "";			//クライアントのパスワード
		firebase::auth::Auth* firebase_auth;	//クライアントの認証情報
		int heart_beat_count;		//クライアントのハートビート回数
		bool auth_status = false;	//クライアントの認証状態
		WOLFSSL* ssl;				//クライアントのソケットにアタッチされたsslオブジェクト


	public:
		explicit Client();
		explicit Client(const std::string id, const std::string name, const std::string mail_addr, const SOCKET socket, const struct sockaddr_in sockaddr_in, const std::string client_ipv4_addr, WOLFSSL* ssl);
		explicit Client(const SOCKET socket, const struct sockaddr_in sockaddr_in, const socklen_t length, std::string& ipv4_addr, WOLFSSL* ssl);
		~Client();


	
		//setter
		void incliment_heart_beat_count();
		void clear_heart_beat_count();
		void change_auth_status();
		bool operator==(const Client& other_client)const;
		Client &operator=(const Client& other_client);

	};
	friend class SocketHandler;
	friend class ChatroomHandler;
	friend class ServerControlleFacade;

	//接続中のクライアントリスト
	std::vector<Client> connected_clients;

	unsigned short connect_client_num;


	//ハートビート用のソケットリスト
	std::vector<std::tuple<SOCKET, struct sockaddr_in, int, std::string>> heart_beat_list;

	//初期化用のクライアント
	Client init_client = Client();

	//private関数
	std::string generate_client_id();

public:
	std::mutex mtx;

public:
	ClientHandler();
	~ClientHandler();

	//getter
	std::vector<Client> get_connected_client_list();
	const std::vector<Client> get_connected_client_list() const;
	const std::vector<SOCKET> get_all_connected_sockets() const;
	Client get_init_client();
	const Client get_init_client() const;
	bool get_client_auth_status(const int client_num);
	void change_client_auth_status(Client& client);

	
	//setter
	void set_client_id(std::string id, uint32_t list_num);
	void set_client_name(std::string name, uint32_t list_num);
	void set_client_mail_addr(std::string mail_addr, uint32_t list_num);
	void set_clietn_socket(SOCKET socket, uint32_t list_num);
	void set_client_sockaddr_in(struct sockaddr_in sock_in, uint32_t list_num);
	void set_socket_addr_in_length(socklen_t length, uint32_t list_num);
	void set_client_ipv4_addr(std::string ipv4_addr, uint32_t list_num);
	void incliment_heart_beat_count(Client& client);
	void clear_heart_beat_count(uint32_t list_num);
	void all_clear_heart_beat_count();
	

	//other
	bool add_client(const Client& client);
	void delete_client(const std::uint32_t delete_num);
	void delete_client(std::vector<Client>::iterator it);
	void delete_client_hbsock_from_hb_list(const std::uint32_t delete_num);
	int check_next_invalid_socket();
	int search_cid_from_clients(const std::string& search_client_id);
	int search_socket_from_clients(const SOCKET& search_socket);
	WOLFSSL* search_client_ssl_from_clients(const std::string& cid);
	void show_client_info(const Client& client);
	std::vector<WOLFSSL*> get_client_ssl_list(const std::vector<Client> clients);
	
};




#endif // !CLIENT_HANDLER_H_


