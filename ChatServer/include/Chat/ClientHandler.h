#ifndef CLIENT_HANDLER_H_
#define CLIENT_HANDLER_H_

#include <iostream>
#include <string>
#include <vector>
#ifdef _WIN32
	#include <WinSock2.h>
	#include <ws2tcpip.h>
	using socket_t = SOCKET;
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <sys/select.h>
	#include <netdb.h>
	using socket_t = int;
#endif	//_WIN32
#include <memory>
#include <rpcdce.h>
#include "AppException.h"
#include "StringFormat.h"
#include "ServerTime.h"
#include "Authentication/FirebaseAuthHandler.h"
#include "Authentication/firebase/auth.h"
#include "Utils/common.h"
#include "wolfssl/ssl.h"
#include <shared_mutex>
#include "Chat/Client.h"
#define MAX_CONNECT_NUM 50000
#define NOT_EXIST_CLIENT_ID -1

//#define DEBUG
#define DEBUG_UUID
class ClientHandler
{
private:

	//接続中のクライアントリスト
	std::vector<std::shared_ptr<Client>> connected_clients;

	unsigned short connect_client_num = connected_clients.size();
	
	//private関数
	std::string generate_client_id();

public:
	mutable std::shared_mutex shared_mtx; // for read/write lock
	//初期化用のクライアント
	const Client init_client = Client();


public:
	ClientHandler();
	~ClientHandler();

	//getter
	size_t get_connect_clients_num() const;

	std::vector<std::shared_ptr<Client>>& get_connected_client_list();
	const std::vector<std::shared_ptr<Client>>& get_connected_client_list() const;
	const std::vector<socket_t> get_all_connected_sockets() const;
	const Client& get_init_client();
	const Client& get_init_client() const;
	bool get_client_auth_status(const int client_num);
	void change_client_auth_status(std::shared_ptr<Client> client);
	std::shared_ptr<Client> get_client_by_index(const uint32_t index);
	std::shared_ptr<Client>& get_client_ref_by_index(const uint32_t index);
	std::shared_ptr<Client> get_client_by_socket(const socket_t socket);
	std::vector<std::shared_ptr<Client>> get_connected_clients_snapshot();
	WOLFSSL* get_client_ssl(const int index);
	std::shared_ptr<Client> get_client_by_cid(const std::string& cid);
	std::vector<std::shared_ptr<Client>>::iterator find_client(const std::shared_ptr<Client>& client);

	
	//setter
	void set_client_id(std::string id, uint32_t list_num);
	void set_client_id(const std::shared_ptr<Client>& client, const std::string& id);
	void set_client_name(std::string name, uint32_t list_num);
	void set_client_name(const std::shared_ptr<Client>& client, const std::string& name);
	void set_client_mail_addr(std::string mail_addr, uint32_t list_num);
	void set_client_mail_addr(const std::shared_ptr<Client>& client, const std::string& mail_addr);
	void set_client_password(const std::shared_ptr<Client>& client, const std::string& password);
	void set_clietn_socket(const socket_t socket, uint32_t list_num);
	void set_client_sockaddr_in(struct sockaddr_in sock_in, uint32_t list_num);
	void set_client_ipv4_addr(std::string ipv4_addr, uint32_t list_num);
	void incliment_heart_beat_count(std::shared_ptr<Client>& client);
	void clear_heart_beat_count(uint32_t list_num);
	void all_clear_heart_beat_count();
	

	//other
	bool add_client(std::shared_ptr<Client> client);
	void delete_client(const std::uint32_t delete_num);
	void delete_client(std::vector<std::shared_ptr<Client>>::iterator it);
	void delete_client(const std::shared_ptr<Client> client);
	int search_cid_from_clients(const std::string& search_client_id);
	int search_socket_from_clients(const socket_t search_socket);
	WOLFSSL* search_client_ssl_from_clients(const std::string& cid);
	void show_client_info(const std::shared_ptr<Client>& client);
	std::vector<WOLFSSL*> get_client_ssl_list(const std::vector<std::shared_ptr<Client>> clients);
	
	
};




#endif // !CLIENT_HANDLER_H_


