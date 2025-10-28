#include "Chat/ClientHandler.hpp"

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


socket_t ClientHandler::get_client_socket_by_cid(const std::string &cid)
{
	LOG_TRACE;
	std::shared_lock<std::shared_mutex> lock(shared_mtx); // 読み取り用ロック
	for (const auto &[sock, ctx] : clients_map)
	{
		if (ctx.client && ctx.client->get_id() == cid)
		{
			return sock; // map のキーが socket なので直接返せる
		}
	}

	// 該当なしの場合
	spdlog::error("No socket found for client ID: {}", cid);
	return INVALID_SOCKET_FD;
}


std::string ClientHandler::get_client_id(const socket_t socket)
{
	LOG_TRACE;
    std::shared_ptr<Client> client = nullptr;
	std::string id;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
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
	LOG_TRACE;
	std::shared_ptr<Client> client = nullptr;
	std::string name;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
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
	LOG_TRACE;
	std::shared_ptr<Client> client = nullptr;
	std::string mail_addr;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
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
	LOG_TRACE;
	std::shared_ptr<Client> client = nullptr;
	sockaddr_in sock_in;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
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
	LOG_TRACE;
	std::shared_ptr<Client> client = nullptr;
	std::string ipv4;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
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
	LOG_TRACE;
	std::shared_ptr<Client> client = nullptr;
	std::string password;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	client = this->get_client_by_socket(socket);
	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return "";
	}

	password = client->get_password();

	return password;
}


int ClientHandler::get_client_heart_beat_count(const socket_t socket)
{
	LOG_TRACE;
	std::shared_ptr<Client> client = nullptr;
	int h_count = 0;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
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
	LOG_TRACE;
	std::shared_ptr<Client> client = nullptr;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return false;
	}
	return client->get_auth_status();
}

SSL *ClientHandler::get_client_ssl(const socket_t socket)
{
	LOG_TRACE;
	std::shared_ptr<Client> client = nullptr;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
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
	LOG_TRACE;
	std::shared_ptr<Client> client = nullptr;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
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
	if (!client)
	{
		LOG_ERROR("Client pointer is null.");
		return;
	}
	client->change_auth_status();
}


void ClientHandler::change_client_auth_status(socket_t socket)
{
	LOG_TRACE;
    std::shared_lock<std::shared_mutex> lock(shared_mtx);  // 読み取り用ロック

    auto it = clients_map.find(socket);
    if (it == clients_map.end() || !it->second.client) {
        LOG_ERROR("Client not found for socket {}", socket);
        return;
    }

    it->second.client->change_auth_status();
}

std::shared_ptr<Client> ClientHandler::get_client_by_socket(const socket_t socket)
{
	LOG_TRACE;
	std::shared_lock<std::shared_mutex> lock(this->shared_mtx);
	auto it = clients_map.find(socket);
    if (it != clients_map.end()) {
        return it->second.client; // 見つかったら Client を返す
    }

    return nullptr; // 見つからなかった場合
}


std::unordered_map<socket_t, ClientContext> ClientHandler::get_connected_clients_snapshot()
{
    LOG_TRACE;
    std::shared_lock<std::shared_mutex> lock(shared_mtx);
    return clients_map; // 共有ロックを使用してコピーを取得
}


void ClientHandler::set_client_id(const socket_t socket, const std::string &id)
{
	LOG_TRACE;
    // 読み取り用ロック
    std::shared_lock<std::shared_mutex> lock(shared_mtx);

    // map から ClientContext を取得
    auto it = clients_map.find(socket);
    if (it == clients_map.end() || !it->second.client) {
        LOG_ERROR("Client not found for socket {}", socket);
        return;
    }

    auto& client = it->second.client;

    // client 内部の mutex をロックして ID をセット
    std::lock_guard<std::mutex> client_lock(client->mtx);
    client->set_id(id);
}

void ClientHandler::set_client_name(const socket_t socket, const std::string &name)
{
    LOG_TRACE;

    // 読み取り用ロック
    std::shared_lock<std::shared_mutex> lock(shared_mtx);

    // map から ClientContext を取得
    auto it = clients_map.find(socket);
    if (it == clients_map.end() || !it->second.client) {
        LOG_ERROR("Client not found for socket {}", socket);
        return;
    }

    auto& client = it->second.client;

    // client 内部の mutex をロックして名前をセット
    std::lock_guard<std::mutex> client_lock(client->mtx);
    client->set_name(name);
	
}


void ClientHandler::set_client_mail_addr(const socket_t socket, const std::string &mail_addr)
{
    LOG_TRACE;

    // 読み取り用ロック
    std::shared_lock<std::shared_mutex> lock(shared_mtx);

    // map から ClientContext を取得
    auto it = clients_map.find(socket);
    if (it == clients_map.end() || !it->second.client) {
        LOG_ERROR("Client not found for socket {}", socket);
        return;
    }

    auto& client = it->second.client;

    // client 内部の mutex をロックしてメールアドレスをセット
    std::lock_guard<std::mutex> client_lock(client->mtx);
    client->set_mail_address(mail_addr);

}


void ClientHandler::set_client_sockaddr_in(const sockaddr_in sock_in, const socket_t socket)
{
    LOG_TRACE;

    // 読み取り用ロック
    std::shared_lock<std::shared_mutex> lock(shared_mtx);

    // map から ClientContext を取得
    auto it = clients_map.find(socket);
    if (it == clients_map.end() || !it->second.client) {
        LOG_ERROR("Client not found for socket {}", socket);
        return;
    }
    auto& client = it->second.client;

    // client 内部の mutex をロックして sockaddr_in をセット
    std::lock_guard<std::mutex> client_lock(client->mtx);
    client->set_sockaddr_in(sock_in);


}



void ClientHandler::set_client_ipv4_addr(const std::string ipv4_addr, const socket_t socket)
{
    LOG_TRACE;

    // 読み取り用ロック
    std::shared_lock<std::shared_mutex> lock(shared_mtx);

    // map から ClientContext を取得
    auto it = clients_map.find(socket);
    if (it == clients_map.end() || !it->second.client) {
        LOG_ERROR("Client not found for socket {}", socket);
        return;
    }

    auto& client = it->second.client;

    // client 内部の mutex をロックして IPv4 アドレスをセット
    std::lock_guard<std::mutex> client_lock(client->mtx);
    client->set_ipv4_address(ipv4_addr);
}



std::shared_ptr<Client> ClientHandler::get_client_by_cid(const std::string& cid)
{
	LOG_TRACE;

	if (cid.empty())
	{
		LOG_ERROR("Client ID is empty.");
		return nullptr;
	}

	std::shared_lock<std::shared_mutex> lock(shared_mtx);

	// clients_map をループして CID を検索
	for (const auto &[sock, ctx] : clients_map)
	{
		if (ctx.client && ctx.client->get_id() == cid)
		{
			return ctx.client; // 見つかった場合は shared_ptr<Client> を返す
		}
	}

	LOG_ERROR("Client with ID {:s} not found.", cid);
	return nullptr; // 見つからなければ nullptr
}




size_t ClientHandler::get_connect_clients_num() const
{
	LOG_TRACE;
	// 読み取り用ロック
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
    return clients_map.size();
}


const Client& ClientHandler::get_init_client()
{
	LOG_TRACE;
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
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	client->incliment_heart_beat_count();
}

void ClientHandler::incliment_heart_beat_count(const socket_t socket)
{
	std::shared_ptr<Client> client = nullptr;
	LOG_TRACE;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return;
	}
	client->incliment_heart_beat_count();
}


void ClientHandler::clear_heart_beat_count(const socket_t socket)
{
	LOG_TRACE;

	// 読み取り用ロック
	std::shared_lock<std::shared_mutex> lock(shared_mtx);

    // map から ClientContext を取得
    auto it = clients_map.find(socket);
    if (it == clients_map.end() || !it->second.client) {
        LOG_ERROR("Client not found for socket {}", socket);
        return;
    }

    auto& client = it->second.client;
    client->clear_heart_beat_count();
}

void ClientHandler::all_clear_heart_beat_count()
{
	LOG_TRACE;

	// 読み取り用ロック
	std::shared_lock<std::shared_mutex> lock(shared_mtx);

    if (clients_map.empty()) {
        return;
    }

    for (auto& [sock, ctx] : clients_map) {
        auto& client = ctx.client;
        if (client) {
            client->clear_heart_beat_count();
        }
    }
}

void ClientHandler::set_conncet_status(const socket_t socket, Client::ConnectState status)
{
	LOG_TRACE;
	std::shared_ptr<Client> client = nullptr;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
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
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return;
	}
	{
		std::lock_guard<std::mutex> client_lock(client->mtx);
		client->set_id(id);
		client->set_name(name);
		client->set_mail_address(mail_addr);
	}
}

bool ClientHandler::add_client(const socket_t socket, SSL *ssl, std::shared_ptr<Client> client)
{
	LOG_TRACE;
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック
	try
	{
		if (clients_map.size() >= MAX_CONNECT_NUM)
		{
			throw std::runtime_error("The maximum number of client connections has been reached and the connection will be abandoned.");
		}

		// map に追加
		this->clients_map[socket] = ClientContext{client, socket, ssl};
		return true;
	}
	catch (const std::exception &e)
	{
		LOG_ERROR("Runtime Error: {}", e.what());
		return false;
	}
}


void ClientHandler::delete_client(const socket_t socket)
{
	LOG_TRACE;
	std::unique_lock<std::shared_mutex> lock(shared_mtx);  // 書き込み用ロック

	auto it = clients_map.find(socket);
	if (it != clients_map.end())
	{
		// 削除前にログ出力
		if (it->second.client)
		{
			spdlog::info("Deleted Client (ID: {}).", it->second.client->get_id());
		}
		else
		{
			spdlog::info("Deleted nullptr Client.");
		}

		clients_map.erase(it); // map から削除
	}
	else
	{
		LOG_ERROR("Client(socket: {}) not found in clients_map.", socket);
	}
}

std::shared_ptr<Client> ClientHandler::search_cid_from_clients(const std::string& search_client_id) const
{
	LOG_TRACE;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
	for (const auto& [sock, ctx] : clients_map) {
        if (ctx.client && ctx.client->get_id() == search_client_id) {
            return ctx.client; // 見つかった場合は shared_ptr<Client> を返す
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


std::vector<socket_t> ClientHandler::monitor_heartbeat()
{
	LOG_TRACE;
	std::vector<socket_t> unresponsive_sockets;

	std::shared_lock<std::shared_mutex> lock(shared_mtx);

	if (clients_map.empty())
	{
		return unresponsive_sockets;
	}

	for (auto &[sock, ctx] : clients_map)
	{
		auto &client = ctx.client;
		if (client)
		{
			if (client->get_heart_beat_count() == 0)
			{
				unresponsive_sockets.push_back(sock);
			}
		}
	}

	return unresponsive_sockets;
}

std::tuple<json, bool, bool> ClientHandler::client_message_receive(socket_t socket)
{
	LOG_TRACE;
	std::shared_ptr<Client> client = nullptr;
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
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
		std::lock_guard<std::mutex> client_lock(client->mtx);
		std::tie(recv_json, disconnect_flag, recv_flag) = _recv_func(client->get_ssl());
	}
	
	return {recv_json, disconnect_flag, recv_flag};
}

bool ClientHandler::client_message_send(socket_t socket, const json& send_data)
{
	LOG_TRACE;
	std::shared_ptr<Client> client = nullptr;

	// クライアントリストから該当ソケットの Client を探す
	std::shared_lock<std::shared_mutex> lock(shared_mtx);
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
		std::lock_guard<std::mutex> client_lock(client->mtx);
		send_result = _send_func(client->get_ssl(), send_data);
	}
	if (send_result == false)
	{
		LOG_ERROR("Failed to send message to client(id : {}, socket : {})", client->get_id(), socket);
	}else{
		spdlog::info("Message sent to client(id : {}, socket : {})", client->get_id(), socket);
	}
	return send_result;
}

bool ClientHandler::client_ssl_shutdown(socket_t socket)
{
	LOG_TRACE;
	std::shared_ptr<Client> client = nullptr;
	bool ssl_shutdown_result = false;

	// まず接続中クライアント一覧から対象を取得（読み取りロック）
	client = this->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return false;
		
	}

	std::unique_lock<std::shared_mutex> lock(shared_mtx);
	ssl_shutdown_result = this->_ssl_shutdown_func(client->get_ssl());
	return ssl_shutdown_result;
}

void ClientHandler::set_send_function(SendFunction func)
{
	LOG_TRACE;
	this->_send_func = std::move(func);
}

void ClientHandler::set_receive_function(ReceiveFunction func)
{
	LOG_TRACE;
	this->_recv_func = std::move(func);
}

void ClientHandler::set_ssl_shutdown_function(SSLShutdownFunction func)
{
	LOG_TRACE;
	this->_ssl_shutdown_func = std::move(func);
}
