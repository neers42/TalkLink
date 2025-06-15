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

std::vector<std::shared_ptr<ClientHandler::Client>>::iterator ClientHandler::find_client(const std::shared_ptr<Client>& client)
{
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	for (auto it = this->connected_clients.begin(); it != this->connected_clients.end(); ++it) {
		if ((*it) == client) { // 修正: ポインタの中身を比較
			return it; // クライアントが見つかった場合、そのイテレータを返す
		}
	}
}

ClientHandler::ClientHandler() :connect_client_num{0}
{
	
}

ClientHandler::~ClientHandler()
{

}

bool ClientHandler::add_client(std::shared_ptr<Client> client)  
{  
	LOG_TRACE; // Ensure LOG_TRACE is a valid macro or function.  
    if (shared_mtx.try_lock()) {  
        spdlog::debug("ssl_mtx locked successfully");  
        // ... 処理 ...  
        shared_mtx.unlock();  
    }  
    else {  
        LOG_ERROR("ssl_mtx is already locked by another thread");  
    }  
    std::unique_lock<std::shared_mutex> lock(shared_mtx);  
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

    this->connected_clients.push_back(std::move(client));  
    this->connect_client_num++;  
    return true;  
}


void ClientHandler::delete_client(const uint32_t delete_num)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	if (!this->connected_clients.empty() && delete_num >= 0 && delete_num < this->connected_clients.size()) {
		auto it = this->connected_clients.begin() + delete_num;
		this->connected_clients.erase(it);
	}
	else {
		LOG_ERROR("Invalid delete_num: {:d}", delete_num);
	}
	this->connect_client_num--;
}

void ClientHandler::delete_client(std::vector<std::shared_ptr<Client>>::iterator it)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
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
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	//std::unique_lock<std::mutex> lock(this->mtx);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	for (int n_invalid_socket = 0; n_invalid_socket < MAX_CONNECT_NUM; n_invalid_socket++) {

		if (this->connected_clients[n_invalid_socket]->socket == INVALID_SOCKET)
			return n_invalid_socket;
	}
	return -1;
}

void ClientHandler::delete_client_hbsock_from_hb_list(const std::uint32_t delete_num)
{
	LOG_TRACE;
	this->heart_beat_list.erase(this->heart_beat_list.begin() + delete_num);
}

bool ClientHandler::get_client_auth_status(const int client_num)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	return this->connected_clients[client_num]->auth_status;
}


void ClientHandler::change_client_auth_status(std::shared_ptr<Client>& client)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	client->change_auth_status();
}

std::shared_ptr < ClientHandler:: Client > ClientHandler::get_client_by_index(const uint32_t index)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	//std::unique_lock<std::mutex> lock(this->mtx);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	if (index < connected_clients.size()) {
		return connected_clients[index];
	}
	return nullptr;
}

std::vector<std::shared_ptr<ClientHandler::Client>> ClientHandler::get_connected_clients_snapshot()
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::vector<std::shared_ptr<ClientHandler::Client>> copy_connected_lists;
	{
		std::shared_lock<std::shared_mutex> lock(shared_mtx);
		copy_connected_lists = connected_clients;
	}
	
	return copy_connected_lists; // コピーを返す
}

WOLFSSL* ClientHandler::get_client_ssl(const int index)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	WOLFSSL* copy_ssl = nullptr;
	{
		std::shared_lock<std::shared_mutex> lock(shared_mtx);
		copy_ssl = this->connected_clients[index]->ssl;

	}
	
	return copy_ssl;
}

void ClientHandler::set_client_id(std::string id, uint32_t list_num)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	this->connected_clients[list_num]->id = id;
}

void ClientHandler::set_client_id(const std::shared_ptr<ClientHandler::Client>& client, const std::string& id)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	
	//std::unique_lock<std::mutex> lock(this->mtx);
	auto it = find_client(client);
	{
		std::unique_lock<std::shared_mutex> lock(shared_mtx);
		if (it != connected_clients.end()) {
			(*it)->id = id; // クライアントのIDを更新
		}
		else {
			LOG_ERROR("Failed set Client ID.Client not found in the list.");
		}
	}
	
}


void ClientHandler::set_client_name(std::string name, uint32_t list_num)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	this->connected_clients[list_num]->name = name;
}

void ClientHandler::set_client_name(const std::shared_ptr<Client>& client, const std::string& name)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	
	//std::unique_lock<std::mutex> lock(this->mtx);
	auto it = find_client(client);
	{
		std::unique_lock<std::shared_mutex> lock(shared_mtx);
		if (it != connected_clients.end()) {
			(*it)->name = name; // クライアントの名前を更新
		}
		else {
			LOG_ERROR("Failed set Client name.Client not found in the list.");
		}
	}
	
}

void ClientHandler::set_client_mail_addr(std::string mail_addr, uint32_t list_num)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}

	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	this->connected_clients[list_num]->mail_address = mail_addr;
}

void ClientHandler::set_client_mail_addr(const std::shared_ptr<Client>& client, const std::string& mail_addr)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	
	//std::unique_lock<std::mutex> lock(this->mtx);
	auto it = find_client(client);
	{
		std::unique_lock<std::shared_mutex> lock(shared_mtx);
		if (it != connected_clients.end()) {
			(*it)->mail_address = mail_addr; // クライアントのメールアドレスを更新
		}
		else {
			LOG_ERROR("Failed set Client mail address.Client not found in the list.");
		}
	}
	
}

void ClientHandler::set_client_password(const std::shared_ptr<Client>& client, const std::string& password)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	
	//std::unique_lock<std::mutex> lock(this->mtx);
	auto it = find_client(client);
	{
		std::unique_lock<std::shared_mutex> lock(shared_mtx);
		if (it != connected_clients.end()) {
			(*it)->password = password; // クライアントのパスワードを更新
		}
		else {
			LOG_ERROR("Failed set Client password.Client not found in the list.");
		}
	}
	
}


void ClientHandler::set_clietn_socket(SOCKET socket, uint32_t list_num)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	this->connected_clients[list_num]->socket = socket;
}

void ClientHandler::set_client_sockaddr_in(sockaddr_in sock_in, uint32_t list_num)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	this->connected_clients[list_num]->sockaddr_in = sock_in;
}

void ClientHandler::set_socket_addr_in_length(socklen_t length, uint32_t list_num)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	this->connected_clients[list_num]->socket_address_in_length = length;
}

void ClientHandler::set_client_ipv4_addr(std::string ipv4_addr, uint32_t list_num)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	this->connected_clients[list_num]->ipv4_address = ipv4_addr;
}

int ClientHandler::search_cid_from_clients(const std::string& search_client_id)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	for (int search_result = 0; search_result < this->connected_clients.size(); search_result++) {

		if (this->connected_clients[search_result]->id == search_client_id) {
			return search_result;
		}
	}
	
	return NOT_EXIST_CLIENT_ID;
}

int ClientHandler::search_socket_from_clients(const SOCKET& search_socket)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	for (int search_result = 0; search_result < this->connected_clients.size(); search_result++) {
		if (this->connected_clients[search_result] != nullptr && this->connected_clients[search_result]->socket == search_socket) {
			return search_result;
		}
	}
	return -1;
}

WOLFSSL* ClientHandler::search_client_ssl_from_clients(const std::string& cid)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	//std::unique_lock<std::mutex> lock(this->mtx);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	for (int search_result = 0; search_result < this->connected_clients.size(); search_result++) {
		if (this->connected_clients[search_result] != nullptr && this->connected_clients[search_result]->id == cid) {
			return this->connected_clients[search_result]->ssl;
		}
	}
	return nullptr;
}

void ClientHandler::show_client_info(const std::shared_ptr<Client>& client)
{
	LOG_TRACE;
	if (client == nullptr) {
		spdlog::error("Client is null. Cannot show client info.");
		return;
	}
	spdlog::debug("Client  ID : {:s}", client->id);
	spdlog::debug("Client  Name : {:s}", client->name);
	spdlog::debug("Client  Mail : {:s}", client->mail_address);
	spdlog::debug("Client  Password : {:s}", client->password);
}

std::vector<WOLFSSL*> ClientHandler::get_client_ssl_list(const std::vector<std::shared_ptr<Client>> clients)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	std::vector<WOLFSSL*> client_ssl_list;
	for (const auto& client : clients) {
		if (client != nullptr && client->ssl != nullptr) {
			client_ssl_list.push_back(client->ssl);
		}
	}
	return client_ssl_list;
}

std::shared_ptr<ClientHandler:: Client>& ClientHandler::get_client_by_cid(const std::string& cid)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	return this->connected_clients.at(this->search_cid_from_clients(cid));
}



std::vector<std::shared_ptr<ClientHandler::Client>>& ClientHandler::get_connected_client_list()
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	//std::unique_lock<std::mutex> lock(this->mtx);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	return this->connected_clients;
}

const std::vector<std::shared_ptr<ClientHandler::Client>>& ClientHandler::get_connected_client_list() const
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	//std::unique_lock<std::mutex> lock(this->mtx);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	return this->connected_clients;
}

const std::vector<SOCKET> ClientHandler::get_all_connected_sockets() const  
{  
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
    std::vector<SOCKET> socket_list;  
    //std::unique_lock<std::mutex> lock(this->mtx); 
    for (const auto& client : connected_clients) {  
        if (client != nullptr && client->socket != INVALID_SOCKET) {  
            socket_list.push_back(client->socket);  
        }  
    }  

    return socket_list;  
}

ClientHandler::Client& ClientHandler::get_init_client()
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	return this->init_client;
}

const ClientHandler::Client& ClientHandler::get_init_client() const
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	return this->init_client;
}




void ClientHandler::incliment_heart_beat_count(std::shared_ptr<Client>& client)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	client->incliment_heart_beat_count();
}

void ClientHandler::clear_heart_beat_count(uint32_t list_num)
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	if(list_num < 0 || list_num >= this->connected_clients.size()) {
		LOG_ERROR("Invalid list_num: {:d}", list_num);
		return;
	}
	this->connected_clients[list_num]->clear_heart_beat_count();
}

void ClientHandler::all_clear_heart_beat_count()
{
	LOG_TRACE;
	if (shared_mtx.try_lock()) {
		spdlog::debug("ssl_mtx locked successfully");
		// ... 処理 ...
		shared_mtx.unlock();

	}
	else {
		LOG_ERROR("ssl_mtx is already locked by another thread");
	}
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	for (auto& client : this->connected_clients) {
		if (client != nullptr) {
			client->clear_heart_beat_count();
		}
	}
}






