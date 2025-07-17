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
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	for (auto it = this->connected_clients.begin(); it != this->connected_clients.end(); ++it) {
		if ((*it) == client) { // 修正: ポインタの中身を比較
			return it; // クライアントが見つかった場合、そのイテレータを返す
		}
	}
	return this->connected_clients.end(); // クライアントが見つからなかった場合、endイテレータを返す
}

socket_t ClientHandler::get_client_socket_by_cid(const std::string &cid)
{
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);

    for (const auto client : connected_clients) {
        if (client && client->get_id() == cid) {
            return client->get_socket(); 
        }
    }
    // 該当なしの場合
    spdlog::error("No socket found for client ID: {}", cid);
    return INVALID_SOCKET_FD;
}
std::string ClientHandler::get_client_id(const socket_t socket)
{
    std::shared_ptr<Client> client = nullptr;
	std::string id;
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

    if (client == nullptr) {
        LOG_ERROR("Client not found for socket {}", socket);
        return "";
    }
        id = client->get_id();  // Client::get_name() はロックなしの単純 getter

    return id;
}

std::string ClientHandler::get_client_name(const socket_t socket)
{
	std::shared_ptr<Client> client = nullptr;
	std::string name;
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return "";
	}

	name = client->get_name(); // Client::get_name() はロックなしの単純 getter

	return name;
}

std::string ClientHandler::get_client_mail_address(const socket_t socket)
{
	std::shared_ptr<Client> client = nullptr;
	std::string mail_addr;

	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return "";
	}

	return client->get_mail_address();
}
sockaddr_in ClientHandler::get_client_sockaddr_in(const socket_t socket)
{
	std::shared_ptr<Client> client = nullptr;
	sockaddr_in sock_in;
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return sockaddr_in{}; // デフォルト初期化された空の構造体を返す
	}

	sock_in = client->get_sockaddr_in(); // Client クラスの getter を呼び出す

	return sock_in;
}

std::string ClientHandler::get_client_ipv4_address(const socket_t socket)
{
	std::shared_ptr<Client> client = nullptr;
	std::string ipv4;

	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return "";
	}

	ipv4 = client->get_ipv4_address();

	return ipv4;
}

std::string ClientHandler::get_client_password(const socket_t socket)
{
	std::shared_ptr<Client> client = nullptr;
	std::string password;
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);
	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return "";
	}

	password = client->get_password();

	return password;
}

firebase::auth::Auth *ClientHandler::get_client_firebase_auth(const socket_t socket)
{
	std::shared_ptr<Client> client = nullptr;
	firebase::auth::Auth *auth = nullptr;

	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return nullptr;
	}

	auth = client->get_firebase_auth();

	return auth;
}

int ClientHandler::get_client_heart_beat_count(const socket_t socket)
{
	std::shared_ptr<Client> client = nullptr;
	int h_count = 0;

	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return -1;
	}

	h_count = client->get_heart_beat_count();

	return h_count;
}

bool ClientHandler::get_client_auth_status(const socket_t socket)
{
	std::shared_ptr<Client> client = nullptr;
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return false;
	}
	return client->get_auth_status();
}

WOLFSSL *ClientHandler::get_client_ssl(const socket_t socket)
{
	std::shared_ptr<Client> client = nullptr;

	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return nullptr;
	}
	return client->get_ssl();
}

Client::ConnectState ClientHandler::get_client_connect_state(const socket_t socket)
{
	std::shared_ptr<Client> client = nullptr;
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return Client::ConnectState::DISCONNECT; // または適切なデフォルト
	}

	// ここは atomic や thread-safe getter ならロック不要
	return client->get_connect_state();
}



void ClientHandler::change_client_auth_status(std::shared_ptr<Client> client)
{
	LOG_TRACE;
	client->change_auth_status();
}

std::shared_ptr < Client > ClientHandler::get_client_by_index(const uint32_t index)
{
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	if (index < connected_clients.size()) {
		return connected_clients[index];
	}
	return nullptr;
}

void ClientHandler::change_client_auth_status(socket_t socket)
{
	std::shared_ptr<Client> client = nullptr;
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return;
	}
	client->change_auth_status();
}
std::shared_ptr<Client> &ClientHandler::get_client_ref_by_index(const uint32_t index)
{
	std::shared_ptr<Client> dummy_ptr = nullptr;
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	if (index < connected_clients.size()) {
		return connected_clients[index];
	}
	return dummy_ptr;
}

std::shared_ptr<Client> ClientHandler::get_client_by_socket(const socket_t socket)
{
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
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	return this->connected_clients; // 共有ロックを使用してコピーを取得
}


void ClientHandler::set_client_id(std::string id, uint32_t list_num)
{
	LOG_TRACE;
	spdlog::debug("before locking unique_lock(shared). func : {}", __func__);
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	spdlog::debug("after locking unique_lock(shared). func : {}", __func__);
	this->connected_clients[list_num]->set_id(id);
}

void ClientHandler::set_client_id(const std::shared_ptr<Client>& client, const std::string& id)
{
	LOG_TRACE;
	spdlog::debug("before locking unique_lock(shared). func : {}", __func__);
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	spdlog::debug("after locking unique_lock(shared). func : {}", __func__);

	for (auto& c : connected_clients) {
		if (c == client) {
			c->set_id(id); // クライアントのIDを更新
			return;
		}
	}

	LOG_ERROR("Failed to set Client ID. Client not found in the list.");
	
}

void ClientHandler::set_client_id(const socket_t socket, const std::string &id)
{
	std::shared_ptr<Client> client = nullptr;
	LOG_TRACE;
	spdlog::debug("before locking unique lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking unique lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return;
	}
	{
		spdlog::debug("before locking client lock. func : {}", __func__);
		std::lock_guard<std::mutex> client_lock(client->mtx);
		spdlog::debug("aftre locking client lock. func : {}", __func__);
		client->set_id(id);
	}
	
}
void ClientHandler::set_client_name(std::string name, uint32_t list_num)
{
	LOG_TRACE;
	spdlog::debug("before locking unique_lock(shared). func : {}", __func__);
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	spdlog::debug("after locking unique_lock(shared). func : {}", __func__);
	this->connected_clients[list_num]->set_name(name);
}

void ClientHandler::set_client_name(const std::shared_ptr<Client>& client, const std::string& name)
{	
	LOG_TRACE;
	spdlog::debug("before locking unique_lock(shared). func : {}", __func__);
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	spdlog::debug("after locking unique_lock(shared). func : {}", __func__);

	for (auto& c : connected_clients) {
		if (c == client) {
			c->set_name(name); // クライアントのIDを更新
			return;
		}
	}
		
}

void ClientHandler::set_client_name(const socket_t socket, const std::string &name)
{
	std::shared_ptr<Client> client = nullptr;
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return;
	}
	{
		spdlog::debug("before locking client lock. func : {}", __func__);
		std::lock_guard<std::mutex> client_lock(client->mtx);
		spdlog::debug("aftre locking client lock. func : {}", __func__);
		client->set_name(name);
	}
	
}

void ClientHandler::set_client_mail_addr(std::string mail_addr, uint32_t list_num)
{
	LOG_TRACE;
	spdlog::debug("before locking unique_lock(shared). func : {}", __func__);
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	spdlog::debug("after locking unique_lock(shared). func : {}", __func__);
	this->connected_clients[list_num]->set_mail_address(mail_addr);
}

void ClientHandler::set_client_mail_addr(const std::shared_ptr<Client>& client, const std::string& mail_addr)
{
	LOG_TRACE;
	spdlog::debug("before locking unique_lock(shared). func : {}", __func__);
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	spdlog::debug("after locking unique_lock(shared). func : {}", __func__);

	for (auto& c : connected_clients) {
		if (c == client) {
			c->set_mail_address(mail_addr); // クライアントのIDを更新
			return;
		}
	}
	
}

void ClientHandler::set_client_mail_addr(const socket_t socket, const std::string &mail_addr)
{
	std::shared_ptr<Client> client = nullptr;
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return;
	}
	{
		spdlog::debug("before locking client lock. func : {}", __func__);
		std::lock_guard<std::mutex> client_lock(client->mtx);
		spdlog::debug("aftre locking client lock. func : {}", __func__);
		client->set_mail_address(mail_addr);
	}
	
}

void ClientHandler::set_client_password(const std::shared_ptr<Client>& client, const std::string& password)
{

	LOG_TRACE;
	spdlog::debug("before locking unique_lock(shared). func : {}", __func__);
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	spdlog::debug("after locking unique_lock(shared). func : {}", __func__);

	for (auto& c : connected_clients) {
		if (c == client) {
			c->set_password(password); // クライアントのIDを更新
			return;
		}
	}
	
}

void ClientHandler::set_client_password(const socket_t socket, const std::string &password)
{
	std::shared_ptr<Client> client = nullptr;

	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return;
	}
	{
		spdlog::debug("before locking client lock. func : {}", __func__);
		std::lock_guard<std::mutex> client_lock(client->mtx);
		spdlog::debug("aftre locking client lock. func : {}", __func__);
		client->set_password(password);
	}
	
}


void ClientHandler::set_client_sockaddr_in(sockaddr_in sock_in, uint32_t list_num)
{
	LOG_TRACE;
	spdlog::debug("before locking unique_lock(shared). func : {}", __func__);
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	spdlog::debug("after locking unique_lock(shared). func : {}", __func__);
	this->connected_clients[list_num]->set_sockaddr_in(sock_in);
}

void ClientHandler::set_client_sockaddr_in(const sockaddr_in sock_in, const socket_t socket)
{
	std::shared_ptr<Client> client = nullptr;
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return;
	}
	{
		spdlog::debug("before locking client lock. func : {}", __func__);
		std::lock_guard<std::mutex> client_lock(client->mtx);
		spdlog::debug("aftre locking client lock. func : {}", __func__);
		client->set_sockaddr_in(sock_in);
	}
	
}


void ClientHandler::set_client_ipv4_addr(std::string ipv4_addr, uint32_t list_num)
{
	LOG_TRACE;
	spdlog::debug("before locking unique_lock(shared). func : {}", __func__);
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	spdlog::debug("after locking unique_lock(shared). func : {}", __func__);
	//std::unique_lock<std::mutex> lock(this->mtx);
	this->connected_clients[list_num]->set_ipv4_address(ipv4_addr);
}

void ClientHandler::set_client_ipv4_addr(const std::string ipv4_addr, const socket_t socket)
{
	std::shared_ptr<Client> client = nullptr;
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return;
	}

	{
		spdlog::debug("before locking client lock. func : {}", __func__);
		std::lock_guard<std::mutex> client_lock(client->mtx);
		spdlog::debug("aftre locking client lock. func : {}", __func__);
		client->set_ipv4_address(ipv4_addr);
	}
	
}



std::shared_ptr<Client> ClientHandler::get_client_by_cid(const std::string& cid)
{
	LOG_TRACE;	
	if (cid.empty()) {
		LOG_ERROR("Client ID is empty.");
		return nullptr;
	}
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	if (connected_clients.empty()) {
		return nullptr;
	}

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
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	return this->connected_clients.size();
}

std::vector<std::shared_ptr<Client>>& ClientHandler::get_connected_client_list()
{
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	return this->connected_clients;
}

const std::vector<std::shared_ptr<Client>>& ClientHandler::get_connected_client_list() const
{
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	return this->connected_clients;
}

const std::vector<socket_t> ClientHandler::get_all_connected_sockets() const  
{
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
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
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	return this->init_client;
}




void ClientHandler::incliment_heart_beat_count(std::shared_ptr<Client>& client)
{
	LOG_TRACE;
	spdlog::debug("before locking unique_lock(shared). func : {}", __func__);
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	spdlog::debug("after locking unique_lock(shared). func : {}", __func__);
	client->incliment_heart_beat_count();
}

void ClientHandler::incliment_heart_beat_count(const socket_t socket)
{
	std::shared_ptr<Client> client = nullptr;
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return;
	}
	client->incliment_heart_beat_count();
}

void ClientHandler::clear_heart_beat_count(uint32_t list_num)
{
	LOG_TRACE;
	spdlog::debug("before locking unique_lock(shared). func : {}", __func__);
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	spdlog::debug("after locking unique_lock(shared). func : {}", __func__);
	if(list_num < 0 || list_num >= this->connected_clients.size()) {
		LOG_ERROR("Invalid list_num: {:d}", list_num);
		return;
	}
	this->connected_clients[list_num]->clear_heart_beat_count();
}

void ClientHandler::clear_heart_beat_count(const socket_t socket)
{
	std::shared_ptr<Client> client = nullptr;
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return;
	}
	client->clear_heart_beat_count();
}

void ClientHandler::all_clear_heart_beat_count()
{
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	for (auto& client : connected_clients) {
		if (client != nullptr) {
			client->clear_heart_beat_count();
		}
	}
}

void ClientHandler::set_conncet_status(const socket_t socket, Client::ConnectState status)
{
	std::shared_ptr<Client> client = nullptr;
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return;
	}
	client->set_conncet_state(status);
}

void ClientHandler::set_authentication_info(const socket_t socket, const std::string &id, const std::string &name, const std::string &mail_addr)
{
	LOG_TRACE;
	std::shared_ptr<Client> client = nullptr;

	// クライアントリストから該当ソケットの Client を探す
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return;
	}
	{
		spdlog::debug("before locking client lock. func : {}", __func__);
		std::lock_guard<std::mutex> client_lock(client->mtx);
		spdlog::debug("aftre locking client lock. func : {}", __func__);
		client->set_id(id);
		client->set_name(name);
		client->set_mail_address(mail_addr);
	}
}
bool ClientHandler::add_client(std::shared_ptr<Client> client)
{  
    LOG_TRACE;
	spdlog::debug("before locking unique_lock(shared). func : {}", __func__);
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	spdlog::debug("after locking unique_lock(shared). func : {}", __func__);
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
	LOG_TRACE;
	spdlog::debug("before locking unique_lock(shared). func : {}", __func__);
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	spdlog::debug("after locking unique_lock(shared). func : {}", __func__);
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
	LOG_TRACE;
	spdlog::debug("before locking unique_lock(shared). func : {}", __func__);
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	spdlog::debug("after locking unique_lock(shared). func : {}", __func__);
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
	LOG_TRACE;
	spdlog::debug("before locking unique_lock(shared). func : {}", __func__);
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	spdlog::debug("after locking unique_lock(shared). func : {}", __func__);
	if (this->connected_clients.empty()) {
		LOG_ERROR("Client list is empty.");
		return;
	}
	auto it = std::find_if(this->connected_clients.begin(), this->connected_clients.end(),
		[&](const std::shared_ptr<Client>& c) {
			return c && c->get_id() == client->get_id();
		});
	if (it != this->connected_clients.end()) {
		spdlog::info("Deleted Client(ID : {}).", client->get_id());
		this->connected_clients.erase(it);
	}
	else {
		LOG_ERROR("Client(ID : {}) not found in connected_clients.", client->get_id());
	}
}

void ClientHandler::delete_client(const socket_t socket)
{
	LOG_TRACE;
	spdlog::debug("before locking unique_lock(shared). func : {}", __func__);
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	spdlog::debug("after locking unique_lock(shared). func : {}", __func__);
	auto it = std::find_if(this->connected_clients.begin(), this->connected_clients.end(),
		[&](const std::shared_ptr<Client>& c) {
			return c && c->get_socket() == socket;
		});
	if (it != this->connected_clients.end()) {
	    if (*it) {
            spdlog::info("Deleted Client (ID: {}).", (*it)->get_id());
        } else {
            spdlog::info("Deleted nullptr Client.");
        }
        this->connected_clients.erase(it);
		spdlog::info("delete saremashita!!!");
	}else {
		LOG_ERROR("Client(ID : {}) not found in connected_clients.", (*it)->get_id());
	}
}

int ClientHandler::search_cid_from_clients(const std::string& search_client_id)
{
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
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
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
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
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
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
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	std::vector<WOLFSSL*> client_ssl_list;
	for (const auto& client : clients) {
		if (client != nullptr && client->get_ssl() != nullptr) {
			client_ssl_list.push_back(client->get_ssl());
		}
	}
	return client_ssl_list;
}

std::tuple<json, bool, bool> ClientHandler::client_message_receive(socket_t socket)
{
	LOG_TRACE;
	std::shared_ptr<Client> client = nullptr;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return {json{}, true, false}; // 切断扱いなど、適切に返す
	}
	if (!_recv_func)
	{
		LOG_ERROR("Receive function not set.");
		return {json{}, true, false};
	}
	// 実際の受信処理（TLSなど）へ渡す
	json recv_json;
	bool disconnect_flag = false;
	bool recv_flag = false;
	{
		spdlog::debug("before locking client lock. func : {}", __func__);
		std::lock_guard<std::mutex> client_lock(client->mtx);
		spdlog::debug("aftre locking client lock. func : {}", __func__);
		std::tie(recv_json, disconnect_flag, recv_flag) = _recv_func(client);
	}
	
	return {recv_json, disconnect_flag, recv_flag};
}

bool ClientHandler::client_message_send(socket_t socket, const json& send_data)
{
	LOG_TRACE;
	std::shared_ptr<Client> client = nullptr;

	// クライアントリストから該当ソケットの Client を探す
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return false;
	}
	if (!_send_func)
	{
		LOG_ERROR("Send function not set.");
		return false;
	}
	// クライアントの送信を thread-safe に行う
	bool send_result;
	{
		spdlog::debug("before locking client lock. func : {}", __func__);
		std::lock_guard<std::mutex> client_lock(client->mtx);
		spdlog::debug("aftre locking client lock. func : {}", __func__);
		send_result = _send_func(client, send_data);
	}

	return send_result;
}

bool ClientHandler::client_ssl_shutdown(socket_t socket)
{
	std::shared_ptr<Client> client = nullptr;

	// まず接続中クライアント一覧から対象を取得（読み取りロック）
	LOG_TRACE;
	spdlog::debug("before locking shared lock. func : {}", __func__);
	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	spdlog::debug("after locking shared lock. func : {}", __func__);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return false;
	}

	if (client->get_ssl() != nullptr)
	{
		this->_ssl_shutdown_func(client);
		return true;
	}
	else
	{
		spdlog::warn("SSL is already null for socket {}", socket);
		return false;
	}
}

void ClientHandler::set_send_function(SendFunction func)
{
	this->_send_func = std::move(func);
}

void ClientHandler::set_receive_function(ReceiveFunction func)
{
	this->_recv_func = std::move(func);
}

void ClientHandler::set_ssl_shutdown_function(SSLShutdonwFunction func)
{
	this->_ssl_shutdown_func = std::move(func);
}
