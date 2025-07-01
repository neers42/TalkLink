#include "Chat/ClientHandler.h"
ClientHandler::ClientHandler() :connect_client_num{ 0 }
{

}

ClientHandler::~ClientHandler()
{

}
std::string ClientHandler::generate_client_id()
{
#ifdef _WIN32
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
#else
	uuid_t uuid;
	char uuid_str[37]; // 36文字 + '\0'

	uuid_generate(uuid);         // ランダムなUUIDを生成
	uuid_unparse(uuid, uuid_str); // 文字列に変換

	return std::string(uuid_str);
#endif // _WIN32

	
}

std::vector<std::shared_ptr<Client>>::iterator ClientHandler::find_client(const std::shared_ptr<Client>& client)
{
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	for (auto it = this->connected_clients.begin(); it != this->connected_clients.end(); ++it) {
		if ((*it) == client) { // 修正: ポインタの中身を比較
			return it; // クライアントが見つかった場合、そのイテレータを返す
		}
	}
	return this->connected_clients.end(); // クライアントが見つからなかった場合、endイテレータを返す
}



bool ClientHandler::add_client(std::shared_ptr<Client> client)  
{  
    std::unique_lock<std::shared_mutex> lock(shared_mtx);  
    try  
    {  
        if (this->connected_clients.size() >= MAX_CONNECT_NUM) {  
            THROW_APP_EXCEPTION("The maximum number of client connections has been reached and the connection will be abandoned");  
        }  
		this->connected_clients.push_back(std::move(client));
		return true;  
    }  
    catch (const exception::ExceptionBase& ex)  
    {  
        LOG_ERROR(ex.what_for_str());  
        return false;  
    }  
}


void ClientHandler::delete_client(const uint32_t delete_num)
{
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	if (!this->connected_clients.empty() && delete_num >= 0 && delete_num < this->get_connect_clients_num() ){
		auto it = this->connected_clients.begin() + delete_num;
		this->connected_clients.erase(it);
	}
	else {
		LOG_ERROR("Invalid delete_num: {:d}", delete_num);
	}
}

void ClientHandler::delete_client(std::vector<std::shared_ptr<Client>>::iterator it)
{
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	if (this->connected_clients.empty() == true) {
		LOG_ERROR("Client list is empty.");
		return;
	}
	else {
		this->connected_clients.erase(it);
	}
}

void ClientHandler::delete_client(const std::shared_ptr<Client> client)
{
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	if (this->connected_clients.empty() == true) {
		LOG_ERROR("Client list is empty.");
		return;
	}
	else {
		auto it = std::find(this->connected_clients.begin(), this->connected_clients.end(), client);
		if (it != this->connected_clients.end()) {
			spdlog::info("Deleted Client(ID : {}).", client->get_id());
			this->connected_clients.erase(it);
		}
		else {
			LOG_ERROR("Client not found in connected_clients.");
		}
	}
}



bool ClientHandler::get_client_auth_status(const int index)
{
	LOG_TRACE;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	return this->connected_clients[index]->get_auth_status();
}


void ClientHandler::change_client_auth_status(std::shared_ptr<Client> client)
{

	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	client->change_auth_status();

	//auto it = this->find_client(client);
	//if (it != connected_clients.end()) {
	//	(*it)->change_auth_status();
	//}
	//else {
	//	LOG_ERROR("Client not found. Cannot change auth status.");
	//}
}

std::shared_ptr < Client > ClientHandler::get_client_by_index(const uint32_t index)
{
	LOG_TRACE;
	//std::unique_lock<std::mutex> lock(this->mtx);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	if (index < connected_clients.size()) {
		return connected_clients[index];
	}
	return nullptr;
}

std::shared_ptr<Client>& ClientHandler::get_client_ref_by_index(const uint32_t index)
{
	LOG_TRACE;
	std::shared_ptr<Client> dummy_ptr = nullptr;
	//std::unique_lock<std::mutex> lock(this->mtx);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	if (index < connected_clients.size()) {
		return connected_clients[index];
	}
	return dummy_ptr;
}

std::shared_ptr<Client> ClientHandler::get_client_by_socket(const socket_t socket)
{
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	for (const auto& client : connected_clients) {
		if (client != nullptr && client->get_socket() == socket) {
			return client;
		}
	}
	return nullptr; // 見つからなかった場合
}


std::vector<std::shared_ptr<Client>> ClientHandler::get_connected_clients_snapshot()
{
	LOG_TRACE;
	{
		std::shared_lock<std::shared_mutex> lock(shared_mtx);
		return this->connected_clients; // 共有ロックを使用してコピーを取得
	}
	
}

WOLFSSL* ClientHandler::get_client_ssl(const int index)
{
	LOG_TRACE;
	WOLFSSL* copy_ssl = nullptr;
	{
		std::shared_lock<std::shared_mutex> lock(shared_mtx);
		return this->connected_clients[index]->get_ssl();

	}
	
}

void ClientHandler::set_client_id(std::string id, uint32_t list_num)
{
	LOG_TRACE;
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	this->connected_clients[list_num]->set_id(id);
}

void ClientHandler::set_client_id(const std::shared_ptr<Client>& client, const std::string& id)
{
	LOG_TRACE;
		
	std::unique_lock<std::shared_mutex> lock(shared_mtx);

	for (auto& c : connected_clients) {
		if (c == client) {
			c->set_id(id); // クライアントのIDを更新
			return;
		}
	}

	LOG_ERROR("Failed to set Client ID. Client not found in the list.");
	
}


void ClientHandler::set_client_name(std::string name, uint32_t list_num)
{
	LOG_TRACE;
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	this->connected_clients[list_num]->set_name(name);
}

void ClientHandler::set_client_name(const std::shared_ptr<Client>& client, const std::string& name)
{	
	LOG_TRACE;

	std::unique_lock<std::shared_mutex> lock(shared_mtx);

	for (auto& c : connected_clients) {
		if (c == client) {
			c->set_name(name); // クライアントのIDを更新
			return;
		}
	}
		
}

void ClientHandler::set_client_mail_addr(std::string mail_addr, uint32_t list_num)
{
	LOG_TRACE;
	
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	this->connected_clients[list_num]->set_mail_address(mail_addr);
}

void ClientHandler::set_client_mail_addr(const std::shared_ptr<Client>& client, const std::string& mail_addr)
{
	LOG_TRACE;
		
	std::unique_lock<std::shared_mutex> lock(shared_mtx);

	for (auto& c : connected_clients) {
		if (c == client) {
			c->set_mail_address(mail_addr); // クライアントのIDを更新
			return;
		}
	}
	
}

void ClientHandler::set_client_password(const std::shared_ptr<Client>& client, const std::string& password)
{

	LOG_TRACE;

	std::unique_lock<std::shared_mutex> lock(shared_mtx);

	for (auto& c : connected_clients) {
		if (c == client) {
			c->set_password(password); // クライアントのIDを更新
			return;
		}
	}
	
}


void ClientHandler::set_clietn_socket(const socket_t socket, uint32_t list_num)
{
	LOG_TRACE;
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	this->connected_clients[list_num]->set_socket(socket);
}

void ClientHandler::set_client_sockaddr_in(sockaddr_in sock_in, uint32_t list_num)
{
	LOG_TRACE;
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	this->connected_clients[list_num]->set_sockaddr_in(sock_in);
}

void ClientHandler::set_client_ipv4_addr(std::string ipv4_addr, uint32_t list_num)
{
	LOG_TRACE;
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	this->connected_clients[list_num]->set_ipv4_address(ipv4_addr);
}

int ClientHandler::search_cid_from_clients(const std::string& search_client_id)
{
	LOG_TRACE;
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	for (int search_result = 0; search_result < this->connected_clients.size(); search_result++) {

		if (this->connected_clients[search_result]->get_id() == search_client_id) {
			return search_result;
		}
	}
	
	return NOT_EXIST_CLIENT_ID;
}

int ClientHandler::search_socket_from_clients(const socket_t search_socket)
{
	LOG_TRACE;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	for (int search_result = 0; search_result < this->connected_clients.size(); search_result++) {
		if (this->connected_clients[search_result] != nullptr && this->connected_clients[search_result]->get_socket() == search_socket) {
			return search_result;
		}
	}
	return -1;
}

WOLFSSL* ClientHandler::search_client_ssl_from_clients(const std::string& cid)
{
	LOG_TRACE;
	//std::unique_lock<std::mutex> lock(this->mtx);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	for (int search_result = 0; search_result < this->connected_clients.size(); search_result++) {
		if (this->connected_clients[search_result] != nullptr && this->connected_clients[search_result]->get_id() == cid) {
			return this->connected_clients[search_result]->get_ssl();
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
	spdlog::debug("Client  ID : {:s}", client->get_id());
	spdlog::debug("Client  Name : {:s}", client->get_name());
	spdlog::debug("Client  Mail : {:s}", client->get_mail_address());
	spdlog::debug("Client  Password : {:s}", client->get_password());
}

std::vector<WOLFSSL*> ClientHandler::get_client_ssl_list(const std::vector<std::shared_ptr<Client>> clients)
{
	LOG_TRACE;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	std::vector<WOLFSSL*> client_ssl_list;
	for (const auto& client : clients) {
		if (client != nullptr && client->get_ssl() != nullptr) {
			client_ssl_list.push_back(client->get_ssl());
		}
	}
	return client_ssl_list;
}

std::shared_ptr<Client> ClientHandler::get_client_by_cid(const std::string& cid)
{
	LOG_TRACE;	
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	for (auto& client : connected_clients) {
		if (client != nullptr && client->get_id() == cid) {
			return client;
		}
	}
	LOG_ERROR("Client with ID {:s} not found.", cid);
	return nullptr; // クライアントが見つからない場合はnullptrを返す
}




size_t ClientHandler::get_connect_clients_num() const
{
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	return this->connected_clients.size();
}

std::vector<std::shared_ptr<Client>>& ClientHandler::get_connected_client_list()
{
	LOG_TRACE;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	return this->connected_clients;
}

const std::vector<std::shared_ptr<Client>>& ClientHandler::get_connected_client_list() const
{
	LOG_TRACE;
	//std::unique_lock<std::mutex> lock(this->mtx);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	return this->connected_clients;
}

const std::vector<socket_t> ClientHandler::get_all_connected_sockets() const  
{  
	LOG_TRACE;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
    std::vector<socket_t> socket_list;  
    for (const auto& client : connected_clients) {  
        if (client != nullptr && client->get_socket() != INVALID_SOCKET_FD) {
            socket_list.push_back(client->get_socket());
        }  
    }  

    return socket_list;  
}

const Client& ClientHandler::get_init_client()
{
	return init_client;
}

const Client& ClientHandler::get_init_client() const
{
	LOG_TRACE;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	return this->init_client;
}




void ClientHandler::incliment_heart_beat_count(std::shared_ptr<Client>& client)
{
	LOG_TRACE;
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	//std::unique_lock<std::mutex> lock(this->mtx);
	client->incliment_heart_beat_count();
}

void ClientHandler::clear_heart_beat_count(uint32_t list_num)
{
	LOG_TRACE;
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
	auto snap_shot = this->get_connected_clients_snapshot();
	for (auto& client : snap_shot) {
		if (client != nullptr) {
			client->clear_heart_beat_count();
		}
	}
}






