#include "Chat/Client.h"
Client::Client() 
	:id{ "" }
	, name{ "" }
	, mail_address{ "" }
	, socket{ INVALID_SOCKET_FD }
	, ipv4_address{ "" }
	, password{ "" }
	, heart_beat_count{ 0 }
	, auth_status{ false }
	, ssl{ nullptr }
{
	this->sockaddr_in.sin_family = AF_INET;
	this->sockaddr_in.sin_port = 0;
#ifdef _WIN32
	this->sockaddr_in.sin_addr.S_un.S_addr = INADDR_ANY;
#else
	this->sockaddr_in.sin_addr.s_addr = INADDR_ANY;
#endif
}

Client::Client(
	const std::string id, 
	const std::string name, 
	const std::string mail_addr, 
	const socket_t socket, 
	const struct sockaddr_in sockaddr_in, 
	const std::string client_ipv4_addr, 
	WOLFSSL* ssl
)
	:id{ id },
	name{ name },
	mail_address{ mail_addr },
	socket{ socket },
	sockaddr_in{ sockaddr_in },
	firebase_auth{ nullptr },
	ipv4_address{ client_ipv4_addr },
	heart_beat_count{ 0 },
	auth_status { false },
	ssl{ ssl },
	c_state{Client::ConnectState::ACTIVE}
{

}

Client::Client(
	const socket_t socket, 
	const struct sockaddr_in sockaddr_in, 
	const socklen_t length, 
	std::string& ipv4_addr, 
	WOLFSSL* ssl
)
	:id{ "" },
	name{ "" },
	mail_address{ "" },
	socket{ socket },
	sockaddr_in{ sockaddr_in },
	firebase_auth{ nullptr },
	ipv4_address{ ipv4_addr },
	heart_beat_count{ 0 },
	auth_status { false },
	ssl{ ssl },
	c_state{Client::ConnectState::ACTIVE}
{
}

Client::~Client()
{

}

std::string Client::get_id() const
{
	return this->id;
}

std::string Client::get_name() const
{
	return this->name;
}

std::string Client::get_mail_address() const
{
	return this->mail_address;
}

socket_t Client::get_socket() const
{
	return this->socket;
}

sockaddr_in Client::get_sockaddr_in() const
{
	return this->sockaddr_in;
}

socklen_t Client::get_socket_address_in_length() const
{
	
	return this->socket_address_in_length;
}

std::string Client::get_ipv4_address() const
{
	return this->ipv4_address;
}

std::string Client::get_password() const
{
	return this->password;
}

firebase::auth::Auth* Client::get_firebase_auth() const
{
	return this->firebase_auth;
}

int Client::get_heart_beat_count() const
{
	return this->heart_beat_count.load();
}

bool Client::get_auth_status() const
{
	return this->auth_status.load();
}

WOLFSSL* Client::get_ssl()
{
	return this->ssl;
}

Client::ConnectState Client::get_connect_state() const
{
	return c_state.load();
}


void Client::set_id(const std::string& id)
{
	//std::unique_lock<std::mutex> lock(this->mtx);
	this->id = id;
}

void Client::set_name(const std::string& name)
{
	//std::unique_lock<std::mutex> lock(this->mtx);
	this->name = name;
}

void Client::set_mail_address(const std::string& mail_address)
{
	//std::unique_lock<std::mutex> lock(this->mtx);
	this->mail_address = mail_address;
}

void Client::set_socket(const socket_t socket)
{
	//std::unique_lock<std::mutex> lock(this->mtx);
	this->socket = socket;
}

void Client::set_sockaddr_in(const struct sockaddr_in sockaddr_in)
{
	//std::unique_lock<std::mutex> lock(this->mtx);
	this->sockaddr_in = sockaddr_in;
}

void Client::set_ipv4_address(const std::string& ipv4_addr)
{
	this->ipv4_address = ipv4_addr;
}

void Client::set_password(const std::string& password)
{
	this->password = password;
}

void Client::set_firebase_auth(firebase::auth::Auth* firebase_auth)
{
	this->firebase_auth = firebase_auth;
}

void Client::set_heart_beat_count(const int count)
{
	this->heart_beat_count.store(count);
}

void Client::set_auth_status(const bool status)
{
	this->auth_status.store(status);
}

void Client::set_ssl(WOLFSSL* ssl)
{
	this->ssl = ssl;
}

void Client::set_conncet_state(ConnectState state)
{
	c_state.store(state);
}

void Client::incliment_heart_beat_count()
{
	this->heart_beat_count.fetch_add(1);
}

void Client::clear_heart_beat_count()
{
	this->heart_beat_count.store(0);
}

void Client::change_auth_status()
{
	bool expected = auth_status.load();
    auth_status.store(!expected);

}

bool Client::operator==(const Client& other_client) const
{
	return this->id == other_client.id;
}

Client& Client::operator=(const Client& other_client)
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

bool Client::compare_and_set_state(Client::ConnectState& expected, Client::ConnectState desired) 
{
    return c_state.compare_exchange_strong(expected, desired);
}
