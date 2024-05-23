#include "ClientHandler.h"

ClientHandler::Client::Client():m_client_id{""}
							, m_client_name{""}
							, m_client_mail_addr{""}
							, m_client_socket{INVALID_SOCKET}
							, m_client_ipv4_addr{""}
							, m_client_password{""}
							, m_heart_beat_count{0}
{
	this -> m_client_sockaddr_in.sin_family = AF_INET;
	this->m_client_sockaddr_in.sin_port = 0;
	this->m_client_sockaddr_in.sin_addr.S_un.S_addr = INADDR_ANY;

	
}

ClientHandler::Client::~Client()
{

}

std::string ClientHandler::Client::get_client_id()
{
	return this->m_client_id;
}

const std::string ClientHandler::Client::get_client_id() const
{
	return this->m_client_id;
}

std::string ClientHandler::Client::get_client_name()
{
	return this->m_client_name;
}

const std::string ClientHandler::Client::get_client_name() const
{
	return this->m_client_name;
}

std::string ClientHandler::Client::get_client_mail_addr()
{
	return this->m_client_mail_addr;
}

const std::string ClientHandler::Client::get_client_mail_addr() const
{
	return this->m_client_mail_addr;
}

SOCKET ClientHandler::Client::get_client_socket()
{
	return this->m_client_socket;
}

const SOCKET ClientHandler::Client::get_client_socket() const
{
	return this->m_client_socket;
}

sockaddr_in ClientHandler::Client::get_sockaddr_in()
{
	return this->m_client_sockaddr_in;
}

const sockaddr_in ClientHandler::Client::get_sockaddr_in() const
{
	return this->m_client_sockaddr_in;
}

socklen_t ClientHandler::Client::get_sockaddr_in_length()
{
	return this->m_socket_addr_in_length;
}

const socklen_t ClientHandler::Client::get_sockaddr_in_length() const
{
	return this->m_socket_addr_in_length;
}

std::string ClientHandler::Client::get_client_ipv4_addr()
{
	return this->m_client_ipv4_addr;
}

const std::string ClientHandler::Client::get_client_ipv4_addr() const
{
	return this->m_client_ipv4_addr;
}

std::string ClientHandler::Client::get_client_password()
{
	return this->m_client_password;
}

const std::string ClientHandler::Client::get_client_password() const
{
	return this->m_client_password;
}

firebase::auth::Auth* ClientHandler::Client::get_client_auth()
{
	return this->client_auth;
}

int ClientHandler::Client::get_client_heart_beat_count()
{
	return this->m_heart_beat_count;
}

const int ClientHandler::Client::get_client_heart_beat_count() const
{
	return this->m_heart_beat_count;
}

void ClientHandler::Client::set_client_id(std::string id)
{
	this->m_client_id = id;
}

void ClientHandler::Client::set_client_name(std::string name)
{
	this->m_client_name = name;
}

void ClientHandler::Client::set_client_mail_addr(std::string mail_addr)
{
	this->m_client_mail_addr = mail_addr;
}

void ClientHandler::Client::set_client_socket(SOCKET socket)
{
	this->m_client_socket = socket;
}

void ClientHandler::Client::set_client_sockaddr_in(sockaddr_in sock_in)
{
	this->m_client_sockaddr_in = sock_in;
}

void ClientHandler::Client::set_socket_addr_in_length(socklen_t length)
{
	this->m_socket_addr_in_length = length;
}

void ClientHandler::Client::set_client_ipv4_addr(std::string ipv4_addr)
{
	this->m_client_ipv4_addr = ipv4_addr;
}

void ClientHandler::Client::set_client_password(std::string password)
{
	this->m_client_password = password;
}

void ClientHandler::Client::incliment_heart_beat_count()
{
	this->m_heart_beat_count++;
}

void ClientHandler::Client::clear_heart_beat_count()
{
	this->m_heart_beat_count = 0;
}

bool ClientHandler::Client::operator==(const Client& other_client) const
{
	return this->m_client_id == other_client.m_client_id;
}

ClientHandler::Client& ClientHandler::Client::operator=(const Client& other_client)
{
	m_client_id = other_client.m_client_id;
	m_client_name = other_client.m_client_name;
	m_client_mail_addr = other_client.m_client_mail_addr;
	m_client_socket = other_client.m_client_socket;
	m_client_sockaddr_in = other_client.m_client_sockaddr_in;
	m_socket_addr_in_length = other_client.m_socket_addr_in_length;
	m_client_ipv4_addr = other_client.m_client_ipv4_addr;
	m_client_password = other_client.m_client_password;
	return *this;
}


ClientHandler::Client::Client(std::string id, std::string name, std::string mail_addr, SOCKET socket, sockaddr_in sockaddr_in, std::string ipv4_addr) :
	m_client_id{id},
	m_client_name{name}, 
	m_client_mail_addr{mail_addr}, 
	m_client_socket{socket}, 
	m_client_sockaddr_in{sockaddr_in}, 
	m_client_ipv4_addr{ipv4_addr},
	m_heart_beat_count{0}
{

}

ClientHandler::Client::Client(const SOCKET socket, const sockaddr_in sockaddr_in, const socklen_t length):
	m_client_id{""},
	m_client_name{""},
	m_client_mail_addr{""},
	m_client_socket{socket}, 
	m_client_sockaddr_in{sockaddr_in}, 
	m_socket_addr_in_length{length}, 
	m_client_ipv4_addr{""},
	m_heart_beat_count{0}
{

}

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
		std::cerr << StringFormat("%s\033[31m[ERORR]%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
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
		std::cerr << StringFormat("%s\033[31m[ERORR]%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
		return "";
	}

	std::string converted_uuid_str(reinterpret_cast<char*>(uuid_str));
#ifdef DEBUG_UUID
	std::cout << StringFormat("%s[DEBUG]The UUID has been successfully generated. UUIDF%s", ServerTime::get_current_time(), converted_uuid_str) << std::endl;
#endif // DEBUG

	RpcStringFreeA(&uuid_str);

	return converted_uuid_str;
}

ClientHandler::ClientHandler()
{
	try
	{
		this->connected_client_list.reserve(MAX_CONNECT_NUM);
		for (int i = 0; i < MAX_CONNECT_NUM; i++) {
			connected_client_list.push_back(init_client);
		}

	}
	catch (const std::length_error& le) {
		THROW_APP_EXCEPTION(StringFormat("Length ErrorF%s", le.what()));
	}
	catch (const exception::ExceptionBase& ex)
	{
		std::cerr << StringFormat("%s\033[31m[ERORR]%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
	}
	
	

}

ClientHandler::~ClientHandler()
{

}

bool ClientHandler::add_client(const Client& client, std::uint32_t add_num)
{
	this->connected_client_list[add_num] = client;
	return true;
}


bool ClientHandler::delete_client_from_con_list(const uint32_t delete_num)
{
	this->connected_client_list[delete_num] = init_client;
	if (this->connected_client_list[delete_num].get_client_id() == "" &&
		this->connected_client_list[delete_num].get_client_socket() == INVALID_SOCKET) {
		return true;
	}
	return false;
}

int ClientHandler::check_next_invalid_socket()
{
	for (int n_invalid_socket = 0; n_invalid_socket < MAX_CONNECT_NUM; n_invalid_socket++) {

		if (this->connected_client_list[n_invalid_socket].get_client_socket() == INVALID_SOCKET)
			return n_invalid_socket;
	}
	return -1;
}

void ClientHandler::delete_client_hbsock_from_hb_list(const std::uint32_t delete_num)
{
	this->heart_beat_list.erase(this->heart_beat_list.begin() + delete_num);
}

int ClientHandler::search_client_id_from_con_list(const std::string& search_client_id)
{
	for (int search_result = 0; search_result < MAX_CONNECT_NUM; search_result++) {

		if (this->connected_client_list[search_result].get_client_id() == search_client_id) {
			return search_result;
		}
	}
	return NOT_EXIST_CLIENT_ID;
}

void ClientHandler::show_client_info(const int& client_num)
{
	LogHandler::logging_info(LogHandler::LogType::INFO, std::cout, StringFormat("Client list[%d] ID : %s", client_num, this->connected_client_list[client_num].get_client_id()));
	LogHandler::logging_info(LogHandler::LogType::INFO, std::cout, StringFormat("Client list[%d] Name : %s", client_num, this->connected_client_list[client_num].get_client_name()));
	LogHandler::logging_info(LogHandler::LogType::INFO, std::cout, StringFormat("Client list[%d] Mail : %s", client_num, this->connected_client_list[client_num].get_client_mail_addr()));
	LogHandler::logging_info(LogHandler::LogType::INFO, std::cout, StringFormat("Client list[%d] Password : %s", client_num, this->connected_client_list[client_num].get_client_password()));
}

std::vector<SOCKET> ClientHandler::extract_conneted_sockets()
{

	std::vector<SOCKET> connected_socket_list;
	for (int i = 0; i < MAX_CONNECT_NUM; i++) {
		if (this->connected_client_list[i].get_client_socket() != NULL) {
			connected_socket_list.push_back(this->connected_client_list[i].get_client_socket());
		}
	}
	return connected_socket_list;
}


std::vector<ClientHandler::Client> ClientHandler::get_connected_client_list()
{
	return this->connected_client_list;
}

const std::vector<ClientHandler::Client> ClientHandler::get_connected_client_list() const
{
	return this->connected_client_list;
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
	this->connected_client_list[list_num].set_client_id(id);
}

void ClientHandler::set_client_name(std::string name, uint32_t list_num)
{
	this->connected_client_list[list_num].set_client_name(name);
}

void ClientHandler::set_client_mail_addr(std::string mail_addr, uint32_t list_num)
{
	this->connected_client_list[list_num].set_client_mail_addr(mail_addr);
}

void ClientHandler::set_clietn_socket(SOCKET socket, uint32_t list_num)
{
	this->connected_client_list[list_num].set_client_socket(socket);
}

void ClientHandler::set_client_sockaddr_in(sockaddr_in sock_in, uint32_t list_num)
{
	this->connected_client_list[list_num].set_client_sockaddr_in(sock_in);
}

void ClientHandler::set_socket_addr_in_length(socklen_t length, uint32_t list_num)
{
	this->connected_client_list[list_num].set_socket_addr_in_length(length);
}

void ClientHandler::set_client_ipv4_addr(std::string ipv4_addr, uint32_t list_num)
{
	this->connected_client_list[list_num].set_client_ipv4_addr(ipv4_addr);
}


void ClientHandler::set_count_hb_list(int list_num)
{
	int& count_ = std::get<2>(this->heart_beat_list[list_num]);
	count_++;
}

void ClientHandler::incliment_heart_beat_count(uint32_t list_num)
{
	this->connected_client_list[list_num].incliment_heart_beat_count();
}

void ClientHandler::clear_heart_beat_count(uint32_t list_num)
{
	this->connected_client_list[list_num].clear_heart_beat_count();
}



void ClientHandler::set_client_id_to_hb_list(const std::string& id, int list_num) 
{
	std::string& change_id = std::get<3>(this->heart_beat_list[list_num]);
	change_id = id;
}



