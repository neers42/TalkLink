#include "ClientHandler.h"

ClientHandler::Client::Client():id{""}
							, name{""}
							, mail_address{""}
							, socket{INVALID_SOCKET}
							, ipv4_address{""}
							, password{""}
							, heart_beat_count{0}
{
	this -> sockaddr_in.sin_family = AF_INET;
	this->sockaddr_in.sin_port = 0;
	this->sockaddr_in.sin_addr.S_un.S_addr = INADDR_ANY;
}

ClientHandler::Client::Client(const std::string id, const std::string name, const std::string mail_addr, const SOCKET socket, const struct sockaddr_in sockaddr_in, const std::string client_ipv4_addr, WOLFSSL* ssl)
	:id{id},
	name{name},
	mail_address{mail_addr},
	socket{socket},
	sockaddr_in{sockaddr_in},
	firebase_auth{nullptr},
	ipv4_address{client_ipv4_addr},
	heart_beat_count{ 0 },
	ssl{ssl}
{

}

ClientHandler::Client::Client(const SOCKET socket, const struct sockaddr_in sockaddr_in, const socklen_t length, std::string& ipv4_addr, WOLFSSL* ssl)
	:id{ "" },
	name{ "" },
	mail_address{ "" },
	socket{ socket },
	sockaddr_in{ sockaddr_in },
	firebase_auth{nullptr},
	ipv4_address{ ipv4_addr },
	heart_beat_count{ 0 },
	ssl{ssl}
{
}

ClientHandler::Client::~Client()
{

}



//std::string ClientHandler::Client::get_client_mail_addr()
//{
//	return this->mail_address;
//}
//
//const std::string ClientHandler::Client::get_client_mail_addr() const
//{
//	return this->mail_address;
//}
//
//SOCKET ClientHandler::Client::get_client_socket()
//{
//	return this->socket;
//}
//
//const SOCKET ClientHandler::Client::get_client_socket() const
//{
//	return this->socket;
//}
//
//sockaddr_in ClientHandler::Client::get_sockaddr_in()
//{
//	return this->sockaddr_in;
//}
//
//const sockaddr_in ClientHandler::Client::get_sockaddr_in() const
//{
//	return this->sockaddr_in;
//}
//
//socklen_t ClientHandler::Client::get_sockaddr_in_length()
//{
//	return this->socket_address_in_length;
//}
//
//const socklen_t ClientHandler::Client::get_sockaddr_in_length() const
//{
//	return this->socket_address_in_length;
//}
//
//std::string ClientHandler::Client::get_client_ipv4_addr()
//{
//	return this->ipv4_address;
//}
//
//const std::string ClientHandler::Client::get_client_ipv4_addr() const
//{
//	return this->ipv4_address;
//}
//
//std::string ClientHandler::Client::get_client_password()
//{
//	return this->password;
//}
//
//const std::string ClientHandler::Client::get_client_password() const
//{
//	return this->password;
//}
//
//firebase::auth::Auth* ClientHandler::Client::get_client_auth()
//{
//	return this->firebase_auth;
//}
//
//int ClientHandler::Client::get_client_heart_beat_count()
//{
//	return this->heart_beat_count;
//}
//
//const int ClientHandler::Client::get_client_heart_beat_count() const
//{
//	return this->heart_beat_count;
//}
//
//bool ClientHandler::Client::get_auth_status()
//{
//	return this->auth_status;
//}
//
//const bool ClientHandler::Client::get_auth_status() const
//{
//	return this->auth_status;
//}
//
//WOLFSSL* ClientHandler::Client::get_ssl()
//{
//	return this->ssl;
//}
//
//WOLFSSL* ClientHandler::Client::get_ssl() const
//{
//	return this->ssl;
//}

//void ClientHandler::Client::set_client_id(std::string id)
//{
//	this->id = id;
//}
//
//void ClientHandler::Client::set_client_name(std::string name)
//{
//	this->name = name;
//}
//
//void ClientHandler::Client::set_client_mail_addr(std::string mail_addr)
//{
//	this->mail_address = mail_addr;
//}
//
//void ClientHandler::Client::set_client_socket(SOCKET socket)
//{
//	this->socket = socket;
//}
//
//void ClientHandler::Client::set_client_sockaddr_in(struct sockaddr_in sock_in)
//{
//	this->sockaddr_in = sock_in;
//}
//
//void ClientHandler::Client::set_socket_addr_in_length(socklen_t length)
//{
//	this->socket_address_in_length = length;
//}
//
//void ClientHandler::Client::set_client_ipv4_addr(std::string ipv4_addr)
//{
//	this->ipv4_address = ipv4_addr;
//}
//
//void ClientHandler::Client::set_client_password(std::string password)
//{
//	this->password = password;
//}

void ClientHandler::Client::incliment_heart_beat_count()
{
	this->heart_beat_count++;
}

void ClientHandler::Client::clear_heart_beat_count()
{
	this->heart_beat_count = 0;
}

void ClientHandler::Client::change_auth_status()
{
	if (auth_status == true) {
		auth_status = false;
	}
	else {
		auth_status = true;
	}
}

bool ClientHandler::Client::operator==(const Client& other_client) const
{
	return this->id == other_client.id;
}

ClientHandler::Client& ClientHandler::Client::operator=(const Client& other_client)
{
	id = other_client.id;
	name = other_client.name;
	mail_address = other_client.mail_address;
	socket = other_client.socket;
	sockaddr_in = other_client.sockaddr_in;
	socket_address_in_length = other_client.socket_address_in_length;
	ipv4_address = other_client.ipv4_address;
	password = other_client.password;
	return *this;
}


//ClientHandler::Client::Client(std::string id, std::string name, std::string mail_addr, SOCKET socket, sockaddr_in sockaddr_in, std::string ipv4_addr)
//	:_id{id},_name{name}, _mail_address{mail_addr}, 
//	_socket{socket}, 
//	_sockaddr_in{sockaddr_in}, 
//	_ipv4_address{ipv4_addr},
//	_heart_beat_count{0}
//{
//
//}



std::string ClientHandler::generate_client_id()
{
	UUID uuid;
	try
	{
		auto uuid_create_result = UuidCreate(&uuid);

		switch (uuid_create_result)
		{
		case RPC_S_OK:
			break;

		case RPC_S_UUID_LOCAL_ONLY:
			THROW_APP_EXCEPTION("The UUID is guaranteed to be unique to this computer only.");
			break;

		case RPC_S_UUID_NO_ADDRESS:
			THROW_APP_EXCEPTION("Cannot get Ethernet or token-ring hardware address for this computer.");
			break;

		default:
			break;
		}

	}
	catch (const exception::ExceptionBase& ex)
	{
		spdlog::error(ex.what_for_str());
		return "";
	}
	
	RPC_CSTR uuid_str;
	try
	{
		auto uuid_str_result = UuidToStringA(&uuid, (RPC_CSTR*)&uuid_str);
		switch (uuid_str_result)
		{
		case RPC_S_OK:
			break;
		case RPC_S_OUT_OF_MEMORY:
			THROW_APP_EXCEPTION("The system is out of memory.");
			break;
		default:
			break;
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		LOG_ERROR(ex.what_for_str());
		return "";
	}

	std::string converted_uuid_str(reinterpret_cast<char*>(uuid_str));
#ifdef DEBUG_UUID
	spdlog::debug("The UUID has been successfully generated. UUID：{:s}", converted_uuid_str);
#endif // DEBUG

	RpcStringFreeA(&uuid_str);

	return converted_uuid_str;
}

ClientHandler::ClientHandler() :connect_client_num{0}
{
	//try
	//{
	//	this->connected_client_list.reserve(MAX_CONNECT_NUM);
	//	for (int i = 0; i < MAX_CONNECT_NUM; i++) {
	//		connected_client_list.push_back(init_client);
	//	}

	//}
	//catch (const std::length_error& le) {
	//	THROW_APP_EXCEPTION(StringFormat("Length Error：%s", le.what()));
	//}
	//catch (const exception::ExceptionBase& ex)
	//{
	//	std::cerr << StringFormat("%s\033[31m[ERORR]%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
	//}
	
	

}

ClientHandler::~ClientHandler()
{

}

bool ClientHandler::add_client(const Client& client)
{
	try
	{
		if (this->connected_clients.size() >= MAX_CONNECT_NUM) {
			THROW_APP_EXCEPTION("The maximum number of client connections has been reached and the connection will be abandoned");
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		LOG_ERROR(ex.what_for_str());
		return false;
	}

	this->connected_clients.push_back(client);
	this->connect_client_num++;
	return true;
}


void ClientHandler::delete_client(const uint32_t delete_num)
{
	if (!this->connected_clients.empty() && delete_num >= 0 && delete_num < this->connected_clients.size()) {
		auto it = this->connected_clients.begin() + delete_num;
		this->connected_clients.erase(it);
	}
	else {
		LOG_ERROR("Invalid delete_num: {:d}", delete_num);
	}
	this->connect_client_num--;
}

void ClientHandler::delete_client(std::vector<Client>::iterator it)
{
	if (this->connected_clients.empty() == true) {
		LOG_ERROR("Client list is empty.");
		return;
	}
	else {
		this->connected_clients.erase(it);
		this->connect_client_num--;
	}
}

int ClientHandler::check_next_invalid_socket()
{
	for (int n_invalid_socket = 0; n_invalid_socket < MAX_CONNECT_NUM; n_invalid_socket++) {

		if (this->connected_clients[n_invalid_socket].socket == INVALID_SOCKET)
			return n_invalid_socket;
	}
	return -1;
}

void ClientHandler::delete_client_hbsock_from_hb_list(const std::uint32_t delete_num)
{
	this->heart_beat_list.erase(this->heart_beat_list.begin() + delete_num);
}

bool ClientHandler::get_client_auth_status(const int client_num)
{
	return this->connected_clients[client_num].auth_status;
}


void ClientHandler::change_client_auth_status(Client& client)
{
	client.change_auth_status();
}

void ClientHandler::set_client_id(std::string id, uint32_t list_num)
{
	this->connected_clients[list_num].id = id;
}

void ClientHandler::set_client_name(std::string name, uint32_t list_num)
{
	this->connected_clients[list_num].name = name;
}

void ClientHandler::set_client_mail_addr(std::string mail_addr, uint32_t list_num)
{
	this->connected_clients[list_num].mail_address = mail_addr;
}

void ClientHandler::set_clietn_socket(SOCKET socket, uint32_t list_num)
{
	this->connected_clients[list_num].socket = socket;
}

void ClientHandler::set_client_sockaddr_in(sockaddr_in sock_in, uint32_t list_num)
{
	this->connected_clients[list_num].sockaddr_in = sock_in;
}

void ClientHandler::set_socket_addr_in_length(socklen_t length, uint32_t list_num)
{
	this->connected_clients[list_num].socket_address_in_length = length;
}

void ClientHandler::set_client_ipv4_addr(std::string ipv4_addr, uint32_t list_num)
{
	this->connected_clients[list_num].ipv4_address = ipv4_addr;
}

int ClientHandler::search_cid_from_clients(const std::string& search_client_id)
{
	for (int search_result = 0; search_result < this->connected_clients.size(); search_result++) {

		if (this->connected_clients[search_result].id == search_client_id) {
			return search_result;
		}
	}
	
	return NOT_EXIST_CLIENT_ID;
}

int ClientHandler::search_socket_from_clients(const SOCKET& search_socket)
{
	for (int search_result = 0; search_result < this->connected_clients.size(); search_result++) {
		if (this->connected_clients[search_result].socket == search_socket) {
			return search_result;
		}
	}
	return -1;
}

WOLFSSL* ClientHandler::search_client_ssl_from_clients(const std::string& cid)
{
	for (int search_result = 0; search_result < this->connected_clients.size(); search_result++) {
		if (this->connected_clients[search_result].id == cid) {
			return this->connected_clients[search_result].ssl;
		}
	}
	return nullptr;
}

void ClientHandler::show_client_info(const Client& client)
{
	spdlog::debug("Client  ID : {:s}", client.id);
	spdlog::debug("Client  Name : {:s}", client.name);
	spdlog::debug("Client  Mail : {:s}", client.mail_address);
	spdlog::debug("Client  Password : {:s}", client.password);
}

std::vector<WOLFSSL*> ClientHandler::get_client_ssl_list(const std::vector<Client> clients)
{
	std::vector<WOLFSSL*> client_ssl_list;
	for (const auto& client : clients) {
		if (client.ssl != nullptr) {
			client_ssl_list.push_back(client.ssl);
		}
	}
	return client_ssl_list;
}

ClientHandler:: Client& ClientHandler::get_client_by_cid(const std::string& cid)
{
	return this->connected_clients.at(this->search_cid_from_clients(cid));
}



std::vector<ClientHandler::Client> ClientHandler::get_connected_client_list()
{
	return this->connected_clients;
}

const std::vector<ClientHandler::Client> ClientHandler::get_connected_client_list() const
{
	return this->connected_clients;
}

const std::vector<SOCKET> ClientHandler::get_all_connected_sockets() const
{
	std::vector<SOCKET> socket_list;
	{
		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);
		for (const auto& client : connected_clients) {
			if (client.socket != INVALID_SOCKET) {
				socket_list.push_back(client.socket);
			}
		}
	}
	
	return socket_list;
}

ClientHandler::Client ClientHandler::get_init_client()
{
	return this->init_client;
}

const ClientHandler::Client ClientHandler::get_init_client() const
{
	return this->init_client;
}




void ClientHandler::incliment_heart_beat_count(Client& client)
{
	client.incliment_heart_beat_count();
}

void ClientHandler::clear_heart_beat_count(uint32_t list_num)
{
	this->connected_clients[list_num].clear_heart_beat_count();
}

void ClientHandler::all_clear_heart_beat_count()
{
	for (auto& client : this->connected_clients) {
		client.clear_heart_beat_count();
	}
}






