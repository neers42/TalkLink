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
	private:
		std::string m_client_id;
		std::string m_client_name;
		std::string m_client_mail_addr;
		SOCKET m_client_socket;
		struct sockaddr_in m_client_sockaddr_in;
		socklen_t m_socket_addr_in_length = sizeof(m_client_sockaddr_in);
		std::string m_client_ipv4_addr = "";
		std::string m_client_password = "";
		firebase::auth::Auth* client_auth;
		int m_heart_beat_count;
		bool m_auth_status = false;


	public:
		explicit Client();
		explicit Client(const std::string id, const std::string name, const std::string mail_addr, const SOCKET socket, const struct sockaddr_in sockaddr_in, const std::string client_ipv4_addr);
		explicit Client(const SOCKET socket, const struct sockaddr_in sockaddr_in, const socklen_t length);
		~Client();


		//getter
		std::string get_client_id();
		const std::string get_client_id() const;
		std::string get_client_name();
		const std::string get_client_name() const;
		std::string get_client_mail_addr();
		const std::string get_client_mail_addr() const;
		SOCKET get_client_socket();
		const SOCKET get_client_socket() const;
		struct sockaddr_in get_sockaddr_in();
		const struct sockaddr_in get_sockaddr_in() const;
		socklen_t get_sockaddr_in_length();
		const socklen_t get_sockaddr_in_length() const;
		std::string get_client_ipv4_addr();
		const std::string get_client_ipv4_addr() const;
		std::string get_client_password();
		const std::string get_client_password() const;
		firebase::auth::Auth* get_client_auth();
		int get_client_heart_beat_count();
		const int get_client_heart_beat_count() const;
		bool get_auth_status();
		const bool get_auth_status() const;

		//setter
		void set_client_id(std::string id);
		void set_client_name(std::string name);
		void set_client_mail_addr(std::string mail_addr);
		void set_client_socket(SOCKET socket);
		void set_client_sockaddr_in(struct sockaddr_in sock_in);
		void set_socket_addr_in_length(socklen_t length);
		void set_client_ipv4_addr(std::string ipv4_addr);
		void set_client_password(std::string password);
		void incliment_heart_beat_count();
		void clear_heart_beat_count();
		void change_auth_status();
		bool operator==(const Client& other_client)const;
		Client &operator=(const Client& other_client);

	};
	friend class SocketHandler;
	friend class Receiver;
	friend class MT002Receiver;
	friend class Sender;
	friend class ChatroomHandler;
	friend class LoginHandler;
	friend class ServerControlleFacade;

	//接続中のクライアントリスト
	std::vector<Client> connected_client_list;

	unsigned short connect_client_num;

	//ハートビート用のソケットリスト
	std::vector<std::tuple<SOCKET, struct sockaddr_in, int, std::string>> heart_beat_list;

	//初期化用のクライアント
	Client init_client = Client();

	//private関数
	std::string generate_client_id();


public:
	ClientHandler();
	~ClientHandler();

	//getter
	std::vector<Client> get_connected_client_list();
	const std::vector<Client> get_connected_client_list() const;
	Client get_init_client();
	const Client get_init_client() const;
	bool get_client_auth_status(const int client_num);
	void change_client_auth_status(const int client_num);

	
	//setter
	void set_client_id(std::string id, uint32_t list_num);
	void set_client_name(std::string name, uint32_t list_num);
	void set_client_mail_addr(std::string mail_addr, uint32_t list_num);
	void set_clietn_socket(SOCKET socket, uint32_t list_num);
	void set_client_sockaddr_in(struct sockaddr_in sock_in, uint32_t list_num);
	void set_socket_addr_in_length(socklen_t length, uint32_t list_num);
	void set_client_ipv4_addr(std::string ipv4_addr, uint32_t list_num);
	void incliment_heart_beat_count(uint32_t list_num);
	void clear_heart_beat_count(uint32_t list_num);
	void set_count_hb_list(int list_num);
	void set_client_id_to_hb_list(const std::string& id, int list_num);
	

	//other
	bool add_client(const Client& client);
	void delete_client_from_con_list(const std::uint32_t delete_num);
	void delete_client_hbsock_from_hb_list(const std::uint32_t delete_num);
	int check_next_invalid_socket();
	int search_client_id_from_con_list(const std::string& search_client_id);
	void show_client_info(const int& client_num);
	std::vector<SOCKET> extract_conneted_sockets();
	
};




#endif // !CLIENT_HANDLER_H_


