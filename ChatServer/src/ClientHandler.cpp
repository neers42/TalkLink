#include "ClientHandler.h"

ClientHandler::Client::Client():client_id{""}, client_name{""}, client_gmail{""}, client_socket{INVALID_SOCKET}, client_ipv4_addr{""}
{
	this -> client_sockaddr_in.sin_family = AF_INET;
	this->client_sockaddr_in.sin_port = 0;
	this->client_sockaddr_in.sin_addr.S_un.S_addr = INADDR_ANY;
}

ClientHandler::Client::~Client()
{

}

ClientHandler::Client::Client(std::string id, std::string name, std::string gmail, SOCKET socket, sockaddr_in sockaddr_in, std::string ipv4_addr) :
	client_id{id}, client_name{name}, client_gmail{gmail}, client_socket{socket}, client_sockaddr_in{sockaddr_in}, client_ipv4_addr{ipv4_addr}
{

}

ClientHandler::Client::Client(const SOCKET socket, const sockaddr_in sockaddr_in, const socklen_t length):
	client_id{""},client_name{""}, client_gmail{""},
	client_socket{socket}, client_sockaddr_in{sockaddr_in}, socket_addr_in_length{length}, client_ipv4_addr{""}
{

}

std::string ClientHandler::Client::generate_client_id()
{

	return std::string();
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

void ClientHandler::add_client(const Client& client, std::uint32_t add_num)
{
	this->connected_client_list[add_num] = client;
}

void ClientHandler::delete_client(Client client)
{
}
