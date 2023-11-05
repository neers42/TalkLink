#ifndef CLIENT_HANDLER_H_
#define CLIENT_HANDLER_H_

#include <iostream>
#include <string>
#include <vector>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <memory>
#define MAX_CONNECT_NUM 50000

class ClientHandler
{
private:

	class Client
	{

	private:
		std::string client_id;
		std::string client_name;
		std::string client_gmail;
		SOCKET client_socket;
		struct sockaddr_in client_sockaddr_in;
		socklen_t socket_addr_in_length = sizeof(client_sockaddr_in);
		std::string client_ipv4_addr = "";
		std::string generate_client_id();

	public:
		explicit Client();
		explicit Client(const std::string id, const std::string name, const std::string gmail, const SOCKET socket, const struct sockaddr_in sockaddr_in, const std::string client_ipv4_addr);
		~Client();


		//getter
		std::string get_client_id() {
			return client_id;
		}

		const std::string get_client_id() const {
			return client_id;
		}

		std::string get_client_name() {
			return client_name;
		}
		const std::string get_client_name() const{
			return client_name;
		}

		std::string get_client_gmail() {
			return client_gmail;
		}

		const std::string get_client_gmail() const{
			return client_gmail;
		}

		SOCKET get_client_socket() {
			return client_socket;
		}

		const SOCKET get_client_socket() const{
			return client_socket;
		}

		struct sockaddr_in get_sockaddr_in() {
			return client_sockaddr_in;
		}

		const struct sockaddr_in get_sockaddr_in() const {
			return client_sockaddr_in;
		}

		socklen_t get_sockaddr_in_length() {
			return socket_addr_in_length;
		}

		const socklen_t get_sockaddr_in_length() const{
			return socket_addr_in_length;
		}

		std::string get_client_ipv4_addr() {
			return client_ipv4_addr;
		}

		const std::string get_client_ipv4_addr() const{
			return client_ipv4_addr;
		}

		//setter
		void set_client_id(std::string id) {
			client_id = id;
		}
		
		void set_client_name(std::string name) {
			client_name = name;
		}

		void set_client_gmail(std::string gmail) {
			client_gmail = gmail;
		}
		void set_client_socket(SOCKET socket) {
			client_socket = socket;
		}
		void set_client_sockaddr_in(struct sockaddr_in sock_in) {
			client_sockaddr_in = sock_in;
		}

		void set_socket_addr_in_length(socklen_t length) {
			socket_addr_in_length = length;
		}
		void set_client_ipv4_addr(std::string ipv4_addr) {
			client_ipv4_addr = ipv4_addr;
		}


	};
	friend class SocketHandler;
	friend class MT001Receiver;

	


	//接続中のクライアントリスト
	std::vector<Client> connected_client_list;
	Client init_client = Client();
	Client client_for_setting = Client();

public:
	ClientHandler();
	~ClientHandler();
	void add_client(const Client& client, std::uint32_t add_num);
	void delete_client(Client client);

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

	void set_client_gmail(std::string gmail) {
		client_for_setting.set_client_gmail(gmail);
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


