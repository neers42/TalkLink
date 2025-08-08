#ifndef CLIENT_HANDLER_H_
#define CLIENT_HANDLER_H_
#ifdef _WIN32
	#include <WinSock2.h>
	#include <ws2tcpip.h>
	using socket_t = SOCKET;	
	#define INVALD_SOCKET_FD INVALID_SOCKET
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <sys/select.h>
	#include <netdb.h>
	using socket_t = int;
	#include <uuid/uuid.h>
	#define INVALD_SOCKET_FD -1
#endif	//_WIN32
#include "Utils/common.hpp"
#include "Chat/Client.hpp"
#define MAX_CONNECT_NUM 50000
#define NOT_EXIST_CLIENT_ID -1

//#define DEBUG
#define DEBUG_UUID
class ClientHandler
{
public:
	mutable std::shared_mutex shared_mtx; // for read/write lock
	//初期化用のクライアント
	const Client init_client = Client();

	using SendFunction = std::function<bool(SSL*, const json&)>;
	using ReceiveFunction = std::function<std::tuple<json, bool, bool>(SSL*)>;
	using SSLShutdownFunction = std::function<bool(SSL*)>;

private:

	//接続中のクライアントリスト
	std::vector<std::shared_ptr<Client>> connected_clients;

	unsigned short connect_client_num = connected_clients.size();
	
	//private関数
	std::string generate_client_id();
	SendFunction _send_func;
	ReceiveFunction _recv_func;
	SSLShutdownFunction _ssl_shutdown_func;
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
	void change_client_auth_status(std::shared_ptr<Client> client);
	void change_client_auth_status(socket_t socket);
	std::shared_ptr<Client> get_client_by_index(const uint32_t index);
	std::shared_ptr<Client> get_client_by_socket(const socket_t socket);
	std::vector<std::shared_ptr<Client>> get_connected_clients_snapshot();
	std::shared_ptr<Client> get_client_by_cid(const std::string& cid);
	std::vector<std::shared_ptr<Client>>::iterator find_client(const std::shared_ptr<Client>& client);
	socket_t get_client_socket_by_cid(const std::string& cid);

	//clientの内部データへのgetter
	std::string get_client_id(const socket_t socket);
	std::string get_client_name(const socket_t socket);
	std::string get_client_mail_address(const socket_t socket);
	struct sockaddr_in get_client_sockaddr_in(const socket_t socket);
	std::string get_client_ipv4_address(const socket_t socket);
	std::string get_client_password(const socket_t socket);
	int get_client_heart_beat_count(const socket_t socket);
	bool get_client_auth_status(const socket_t socket);
	SSL* get_client_ssl(const socket_t socket);
	Client::ConnectState get_client_connect_state(const socket_t socket);
	
	//clientの内部データへのsetter
	void set_client_id(std::string id, uint32_t list_num);
	void set_client_id(const std::shared_ptr<Client>& client, const std::string& id);
	void set_client_id(const socket_t socket, const std::string& id);
	void set_client_name(std::string name, uint32_t list_num);
	void set_client_name(const std::shared_ptr<Client>& client, const std::string& name);
	void set_client_name(const socket_t socket, const std::string& name);
	void set_client_mail_addr(std::string mail_addr, uint32_t list_num);
	void set_client_mail_addr(const std::shared_ptr<Client>& client, const std::string& mail_addr);
	void set_client_mail_addr(const socket_t socket, const std::string& mail_addr);
	void set_client_password(const std::shared_ptr<Client>& client, const std::string& password);
	void set_client_password(const socket_t socket, const std::string& password);
	void set_client_sockaddr_in(struct sockaddr_in sock_in, uint32_t list_num);
	void set_client_sockaddr_in(const struct sockaddr_in sock_in, const socket_t socket);
	void set_client_ipv4_addr(std::string ipv4_addr, uint32_t list_num);
	void set_client_ipv4_addr(const std::string ipv4_addr, const socket_t socket);
	void incliment_heart_beat_count(std::shared_ptr<Client>& client);
	void incliment_heart_beat_count(const socket_t socket);
	void clear_heart_beat_count(uint32_t list_num);
	void clear_heart_beat_count(const socket_t socket);
	void all_clear_heart_beat_count();
	void set_conncet_status(const socket_t socket, Client::ConnectState status);
	void set_authentication_info(const socket_t socket, const std::string& id, const std::string& name, const std::string& mail_addr);
	
	

	//other
	bool add_client(std::shared_ptr<Client> client);
	void delete_client(const std::uint32_t delete_num);
	void delete_client(std::vector<std::shared_ptr<Client>>::iterator it);
	void delete_client(const std::shared_ptr<Client> client);
	void delete_client(const socket_t socket);
	int search_cid_from_clients(const std::string& search_client_id);
	int search_socket_from_clients(const socket_t search_socket);
	SSL* search_client_ssl_from_clients(const std::string& cid);
	void show_client_info(const std::shared_ptr<Client>& client);
	std::vector<SSL*> get_client_ssl_list(const std::vector<std::shared_ptr<Client>> clients);
	std::vector<socket_t> monitor_heartbeat();
	std::tuple<json, bool, bool> client_message_receive(socket_t socket);
	bool client_message_send(socket_t socket, const json& send_data);
	bool client_ssl_shutdown(socket_t socket);
	void set_send_function(SendFunction func);
	void set_receive_function(ReceiveFunction func);
	void set_ssl_shutdown_function(SSLShutdownFunction func);
	
	
};

#endif // !CLIENT_HANDLER_H_


