#include "ClientHandler.h"

ClientHandler::Client::Client():_id{""}
							, _name{""}
							, _mail_address{""}
							, _socket{INVALID_SOCKET}
							, _ipv4_address{""}
							, _password{""}
							, _heart_beat_count{0}
{
	this -> _sockaddr_in.sin_family = AF_INET;
	this->_sockaddr_in.sin_port = 0;
	this->_sockaddr_in.sin_addr.S_un.S_addr = INADDR_ANY;
}

ClientHandler::Client::Client(const std::string id, const std::string name, const std::string mail_addr, const SOCKET socket, const sockaddr_in sockaddr_in, const std::string client_ipv4_addr, WOLFSSL* ssl)
	:_id{id},
	_name{name},
	_mail_address{mail_addr},
	_socket{socket},
	_sockaddr_in{sockaddr_in},
	_firebase_auth{nullptr},
	_ipv4_address{client_ipv4_addr},
	_heart_beat_count{ 0 },
	_ssl{ssl}
{

}

ClientHandler::Client::Client(const SOCKET socket, const sockaddr_in sockaddr_in, const socklen_t length, std::string& ipv4_addr, WOLFSSL* ssl)
	:_id{ "" },
	_name{ "" },
	_mail_address{ "" },
	_socket{ socket },
	_sockaddr_in{ sockaddr_in },
	_firebase_auth{nullptr},
	_ipv4_address{ ipv4_addr },
	_heart_beat_count{ 0 },
	_ssl{ssl}
{
}

ClientHandler::Client::~Client()
{

}

std::string ClientHandler::Client::get_client_id()
{
	return this->_id;
}

const std::string ClientHandler::Client::get_client_id() const
{
	return this->_id;
}

std::string ClientHandler::Client::get_client_name()
{
	return this->_name;
}

const std::string ClientHandler::Client::get_client_name() const
{
	return this->_name;
}

std::string ClientHandler::Client::get_client_mail_addr()
{
	return this->_mail_address;
}

const std::string ClientHandler::Client::get_client_mail_addr() const
{
	return this->_mail_address;
}

SOCKET ClientHandler::Client::get_client_socket()
{
	return this->_socket;
}

const SOCKET ClientHandler::Client::get_client_socket() const
{
	return this->_socket;
}

sockaddr_in ClientHandler::Client::get_sockaddr_in()
{
	return this->_sockaddr_in;
}

const sockaddr_in ClientHandler::Client::get_sockaddr_in() const
{
	return this->_sockaddr_in;
}

socklen_t ClientHandler::Client::get_sockaddr_in_length()
{
	return this->_socket_address_in_length;
}

const socklen_t ClientHandler::Client::get_sockaddr_in_length() const
{
	return this->_socket_address_in_length;
}

std::string ClientHandler::Client::get_client_ipv4_addr()
{
	return this->_ipv4_address;
}

const std::string ClientHandler::Client::get_client_ipv4_addr() const
{
	return this->_ipv4_address;
}

std::string ClientHandler::Client::get_client_password()
{
	return this->_password;
}

const std::string ClientHandler::Client::get_client_password() const
{
	return this->_password;
}

firebase::auth::Auth* ClientHandler::Client::get_client_auth()
{
	return this->_firebase_auth;
}

int ClientHandler::Client::get_client_heart_beat_count()
{
	return this->_heart_beat_count;
}

const int ClientHandler::Client::get_client_heart_beat_count() const
{
	return this->_heart_beat_count;
}

bool ClientHandler::Client::get_auth_status()
{
	return this->_auth_status;
}

const bool ClientHandler::Client::get_auth_status() const
{
	return this->_auth_status;
}

WOLFSSL* ClientHandler::Client::get_ssl()
{
	return this->_ssl;
}

WOLFSSL* ClientHandler::Client::get_ssl() const
{
	return this->_ssl;
}

void ClientHandler::Client::set_client_id(std::string id)
{
	this->_id = id;
}

void ClientHandler::Client::set_client_name(std::string name)
{
	this->_name = name;
}

void ClientHandler::Client::set_client_mail_addr(std::string mail_addr)
{
	this->_mail_address = mail_addr;
}

void ClientHandler::Client::set_client_socket(SOCKET socket)
{
	this->_socket = socket;
}

void ClientHandler::Client::set_client_sockaddr_in(sockaddr_in sock_in)
{
	this->_sockaddr_in = sock_in;
}

void ClientHandler::Client::set_socket_addr_in_length(socklen_t length)
{
	this->_socket_address_in_length = length;
}

void ClientHandler::Client::set_client_ipv4_addr(std::string ipv4_addr)
{
	this->_ipv4_address = ipv4_addr;
}

void ClientHandler::Client::set_client_password(std::string password)
{
	this->_password = password;
}

void ClientHandler::Client::incliment_heart_beat_count()
{
	this->_heart_beat_count++;
}

void ClientHandler::Client::clear_heart_beat_count()
{
	this->_heart_beat_count = 0;
}

void ClientHandler::Client::change_auth_status()
{
	if (_auth_status == true) {
		_auth_status = false;
	}
	else {
		_auth_status = true;
	}
}

bool ClientHandler::Client::operator==(const Client& other_client) const
{
	return this->_id == other_client._id;
}

ClientHandler::Client& ClientHandler::Client::operator=(const Client& other_client)
{
	_id = other_client._id;
	_name = other_client._name;
	_mail_address = other_client._mail_address;
	_socket = other_client._socket;
	_sockaddr_in = other_client._sockaddr_in;
	_socket_address_in_length = other_client._socket_address_in_length;
	_ipv4_address = other_client._ipv4_address;
	_password = other_client._password;
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
		LogHandler::get_instance().logging_error(ex.what_for_str());
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
		LogHandler::get_instance().logging_error(ex.what_for_str());
		return "";
	}

	std::string converted_uuid_str(reinterpret_cast<char*>(uuid_str));
#ifdef DEBUG_UUID
	LogHandler::get_instance().logging_debug(StringFormat("The UUID has been successfully generated. UUIDF%s", converted_uuid_str));
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
	//	THROW_APP_EXCEPTION(StringFormat("Length ErrorF%s", le.what()));
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
		LogHandler::get_instance().logging_error(ex.what_for_str());
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
		LogHandler::get_instance().logging_error(StringFormat("Invalid delete_num:%d", delete_num));
		std::cerr << "Invalid delete_num: " << delete_num << std::endl;
	}
	//this->connected_client_list.erase(this->connected_client_list.begin() + delete_num);
	this->connect_client_num--;
}

void ClientHandler::delete_client(std::vector<Client>::iterator it)
{
	if (this->connected_clients.empty() == true) {
		LogHandler::get_instance().logging_error("Client list is empty.");
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

		if (this->connected_clients[n_invalid_socket].get_client_socket() == INVALID_SOCKET)
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
	return this->connected_clients[client_num].get_auth_status();
}


void ClientHandler::change_client_auth_status(const int client_num)
{
	this->connected_clients[client_num].change_auth_status();
}

int ClientHandler::search_client_id_from_con_list(const std::string& search_client_id)
{
	for (int search_result = 0; search_result < this->connected_clients.size(); search_result++) {

		if (this->connected_clients[search_result].get_client_id() == search_client_id) {
			return search_result;
		}
	}
	LogHandler::get_instance().logging_error(StringFormat("search client id, failed, Not exits client id(%s)", search_client_id));
	return NOT_EXIST_CLIENT_ID;
}

void ClientHandler::show_client_info(const int& client_num)
{
	LogHandler::get_instance().logging_info(StringFormat("Client list[%d] ID : %s", client_num, this->connected_clients[client_num].get_client_id()));
	LogHandler::get_instance().logging_info(StringFormat("Client list[%d] Name : %s", client_num, this->connected_clients[client_num].get_client_name()));
	LogHandler::get_instance().logging_info(StringFormat("Client list[%d] Mail : %s", client_num, this->connected_clients[client_num].get_client_mail_addr()));
	LogHandler::get_instance().logging_info(StringFormat("Client list[%d] Password : %s", client_num, this->connected_clients[client_num].get_client_password()));
}

std::vector<SOCKET> ClientHandler::extract_conneted_sockets()
{
	std::vector<SOCKET> connected_socket_list;
	for (int i = 0; i < MAX_CONNECT_NUM; i++) {
		if (this->connected_clients[i].get_client_socket() != NULL) {
			connected_socket_list.push_back(this->connected_clients[i].get_client_socket());
		}
	}
	return connected_socket_list;
}


std::vector<ClientHandler::Client> ClientHandler::get_connected_client_list()
{
	return this->connected_clients;
}

const std::vector<ClientHandler::Client> ClientHandler::get_connected_client_list() const
{
	return this->connected_clients;
}

ClientHandler::Client ClientHandler::get_init_client()
{
	return this->init_client;
}

const ClientHandler::Client ClientHandler::get_init_client() const
{
	return this->init_client;
}


void ClientHandler::set_client_id(std::string id, uint32_t list_num)
{
	this->connected_clients[list_num].set_client_id(id);
}

void ClientHandler::set_client_name(std::string name, uint32_t list_num)
{
	this->connected_clients[list_num].set_client_name(name);
}

void ClientHandler::set_client_mail_addr(std::string mail_addr, uint32_t list_num)
{
	this->connected_clients[list_num].set_client_mail_addr(mail_addr);
}

void ClientHandler::set_clietn_socket(SOCKET socket, uint32_t list_num)
{
	this->connected_clients[list_num].set_client_socket(socket);
}

void ClientHandler::set_client_sockaddr_in(sockaddr_in sock_in, uint32_t list_num)
{
	this->connected_clients[list_num].set_client_sockaddr_in(sock_in);
}

void ClientHandler::set_socket_addr_in_length(socklen_t length, uint32_t list_num)
{
	this->connected_clients[list_num].set_socket_addr_in_length(length);
}

void ClientHandler::set_client_ipv4_addr(std::string ipv4_addr, uint32_t list_num)
{
	this->connected_clients[list_num].set_client_ipv4_addr(ipv4_addr);
}


void ClientHandler::set_count_hb_list(int list_num)
{
	int& count_ = std::get<2>(this->heart_beat_list[list_num]);
	count_++;
}

void ClientHandler::incliment_heart_beat_count(uint32_t list_num)
{
	this->connected_clients[list_num].incliment_heart_beat_count();
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



void ClientHandler::set_client_id_to_hb_list(const std::string& id, int list_num) 
{
	std::string& change_id = std::get<3>(this->heart_beat_list[list_num]);
	change_id = id;
}



