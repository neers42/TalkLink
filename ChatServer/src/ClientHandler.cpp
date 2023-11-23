#include "ClientHandler.h"

ClientHandler::Client::Client():m_client_id{""}, m_client_name{""}, m_client_gmail{""}, m_client_socket{INVALID_SOCKET}, m_client_ipv4_addr{""}
{
	this -> m_client_sockaddr_in.sin_family = AF_INET;
	this->m_client_sockaddr_in.sin_port = 0;
	this->m_client_sockaddr_in.sin_addr.S_un.S_addr = INADDR_ANY;
}

ClientHandler::Client::~Client()
{

}

ClientHandler::Client::Client(std::string id, std::string name, std::string gmail, SOCKET socket, sockaddr_in sockaddr_in, std::string ipv4_addr) :
	m_client_id{id}, m_client_name{name}, m_client_gmail{gmail}, m_client_socket{socket}, m_client_sockaddr_in{sockaddr_in}, m_client_ipv4_addr{ipv4_addr}
{

}

ClientHandler::Client::Client(const SOCKET socket, const sockaddr_in sockaddr_in, const socklen_t length):
	m_client_id{""},m_client_name{""}, m_client_gmail{""},
	m_client_socket{socket}, m_client_sockaddr_in{sockaddr_in}, m_socket_addr_in_length{length}, m_client_ipv4_addr{""}
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
#ifdef DEBUG
	std::cout << StringFormat("%s[DEBUG]The UUID has been successfully generated. UUIDF%s", ServerTime::get_current_time(), converted_uuid_str) << std::endl;
#endif // DEBUG
	
	RpcStringFreeA(&uuid_str);

	return converted_uuid_str;
}

ClientHandler::ClientHandler()
{
	//Client *init_client = new Client();
	//std::shared_ptr<Client> init_client = std::make_shared<Client>();
	
	this->connected_client_list.reserve(MAX_CONNECT_NUM);
	for (int i = 0; i < MAX_CONNECT_NUM; i++) {
		connected_client_list.push_back(init_client);
	}
	

}

ClientHandler::~ClientHandler()
{

}

bool ClientHandler::add_client(const Client& client, std::uint32_t add_num)
{
	this->connected_client_list[add_num] = client;
	std::string new_client_id = generate_client_id();
	if (new_client_id == "") {
		return false;
	}
	this->connected_client_list[add_num].set_client_id(new_client_id);
	return true;
}

bool ClientHandler::delete_client(const Client& client)
{

	return true;
}
