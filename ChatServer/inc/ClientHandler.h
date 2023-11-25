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
#define MAX_CONNECT_NUM 50000

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


	public:
		explicit Client();
		explicit Client(const std::string id, const std::string name, const std::string gmail, const SOCKET socket, const struct sockaddr_in sockaddr_in, const std::string client_ipv4_addr);
		explicit Client(const SOCKET socket, const struct sockaddr_in sockaddr_in, const socklen_t length);
		~Client();


		//getter
		std::string get_client_id() {
			return m_client_id;
		}

		const std::string get_client_id() const {
			return m_client_id;
		}

		std::string get_client_name() {
			return m_client_name;
		}
		const std::string get_client_name() const{
			return m_client_name;
		}

		std::string get_client_mail_addr() {
			return m_client_mail_addr;
		}

		const std::string get_client_mail_addr() const{
			return m_client_mail_addr;
		}

		SOCKET get_client_socket() {
			return m_client_socket;
		}

		const SOCKET get_client_socket() const{
			return m_client_socket;
		}

		struct sockaddr_in get_sockaddr_in() {
			return m_client_sockaddr_in;
		}

		const struct sockaddr_in get_sockaddr_in() const {
			return m_client_sockaddr_in;
		}

		socklen_t get_sockaddr_in_length() {
			return m_socket_addr_in_length;
		}

		const socklen_t get_sockaddr_in_length() const{
			return m_socket_addr_in_length;
		}

		std::string get_client_ipv4_addr() {
			return m_client_ipv4_addr;
		}

		const std::string get_client_ipv4_addr() const{
			return m_client_ipv4_addr;
		}

		//setter
		void set_client_id(std::string id) {
			m_client_id = id;
		}
		
		void set_client_name(std::string name) {
			m_client_name = name;
		}

		void set_client_gmail(std::string mail) {
			m_client_mail_addr = mail;
		}
		void set_client_socket(SOCKET socket) {
			m_client_socket = socket;
		}
		void set_client_sockaddr_in(struct sockaddr_in sock_in) {
			m_client_sockaddr_in = sock_in;
		}

		void set_socket_addr_in_length(socklen_t length) {
			m_socket_addr_in_length = length;
		}
		void set_client_ipv4_addr(std::string ipv4_addr) {
			m_client_ipv4_addr = ipv4_addr;
		}

		bool operator==(const Client& other_client)const {
			return this->m_client_id == other_client.m_client_id;
		}
		

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
	Client init_client = Client();
	Client client_for_setting = Client();

	//Clientprivate関数
	std::string generate_client_id();


public:
	ClientHandler();
	~ClientHandler();
	bool add_client(const Client& client, std::uint32_t add_num);
	bool delete_client(const Client& client);

	std::vector<Client> get_client_list() {
		return connected_client_list;
	}
	const std::vector<Client> get_client_list() const{
		return connected_client_list;
	}

	const Client get_init_client() const {
		return init_client;
	}

	//setter
	void set_client_id(std::string id) {

		client_for_setting.set_client_id(id);
	}

	void set_client_name(std::string name) {
		client_for_setting.set_client_name(name);
	}

	void set_client_gmail(std::string mail) {
		client_for_setting.set_client_gmail(mail);
	}
	void set_clietn_socket(SOCKET socket) {
		client_for_setting.set_client_socket(socket);
	}
	void set_client_sockaddr_in(struct sockaddr_in sock_in) {
		client_for_setting.set_client_sockaddr_in(sock_in);
	}

	void set_socket_addr_in_length(socklen_t length) {
		client_for_setting.set_socket_addr_in_length(length);
	}
	void set_client_ipv4_addr(std::string ipv4_addr) {
		client_for_setting.set_client_ipv4_addr(ipv4_addr);
	}

	
};




#endif // !CLIENT_HANDLER_H_


