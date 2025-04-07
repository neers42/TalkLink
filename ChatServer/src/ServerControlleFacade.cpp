#include "ServerControlleFacade.h"

std::time_t ServerTime::current_time;
std::tm ServerTime::time_info;

ServerControlleFacade::ServerControlleFacade() : _server_ip{ "" }
{
	//ログレベルの設定
#ifdef DEBUG_MODE
	spdlog::set_level(spdlog::level::debug);
#else
	//ログのレベル設定
	spdlog::set_level(spdlog::level::info);

#endif // DEBUG_MODE

	////ログのフォーマット設定
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread id : %t] [%^%l%$] %v");
	spdlog::info("Starting MyChat server version 1.0.0!!");
	_socket_handler = std::make_unique<SocketHandler>();
	this->set_server_ip(this->_socket_handler->obtain_local_ip_addr());
	_firebase_auth_handler = std::make_unique<FirebaseAuthHandler>();
	_mysql_client = std::make_unique<MySQLClient>();
	_redis_client = std::make_unique<RedisClient>();
	_protocol_handler = std::make_unique<ProtocolHandler>();
	_server_time = std::make_unique<ServerTime>();
	_client_handler = std::make_unique<ClientHandler>();
	_thread_pool_handler = std::make_unique<ThreadPoolHandler>(10);
	_chat_room_handler = std::make_unique<ChatroomHandler>();
	
}

ServerControlleFacade::~ServerControlleFacade()
{
	
}

//サーバプログラムの実行
void ServerControlleFacade::run()
{
	//リッスンソケットの生成
	if (!_socket_handler->open_tcp_listen_socket()) {
		LOG_ERROR("The TCP listening socket for message could not be opened properly, so the program is forcibly terminated. Please set it again.");
		exit(1);
	}

	if (!_socket_handler->generate_udp_multicast_tx_socket()) {
		LOG_ERROR("The program terminates because it failed to create a socket for multicast transmission. Please reset the settings.");
		exit(1);
	}
	
	this->_mysql_client->connect();
	this->_mysql_client->create_database();
	this->_mysql_client->execute_query("USE mychat");
	this->_mysql_client->create_table();
	this->_redis_client->connect();

	spdlog::info("Start monitoring requests from clients.");
	this->_thread_pool_handler->enqueue_task([this]() {_thread_pool_handler->monitor_worker(); });
	this->_thread_pool_handler->enqueue_task([this]() {heart_beat_check_process(); });

	//ディスクリプション監視スタート
	while (1) {
		auto abs_time = std::chrono::system_clock::now() + std::chrono::milliseconds(500);
		this->_socket_handler->setting_select_info_to_listen_socket();
		this->setting_select_info();
		std::vector<SOCKET> changed_client_sockets = this->_client_handler->get_all_connected_sockets();
		_socket_handler->monitor_socket_descriptor(changed_client_sockets, 
			[this](SOCKET changed_socket) {handle_changed_socket_process(changed_socket); });

		std::this_thread::sleep_until(abs_time);

	}
	
}

void ServerControlleFacade::setting_select_info()
{
	{
		std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
		for (auto& client : this->_client_handler->connected_clients) {
			//有効なソケットにFDSETを実行する
			if (client.socket != INVALID_SOCKET) {
				FD_SET(client.socket, &_socket_handler->get_fd_read());
			}
		}

			}

	this->_socket_handler->copy_fd_read_to_fds(&(_socket_handler->get_fds()), &(_socket_handler->get_fd_read()));


}


void ServerControlleFacade::show_server_running_info()
{
	std::cout << " * TLS Version : " << this->_protocol_handler->get_tls_version(this->_protocol_handler->get_wolfssl_object()) << std::endl;
	std::cout << " * Cipher suite : " << this->_protocol_handler->get_cipher_suite(this->_protocol_handler->get_wolfssl_object()) << std::endl;
	std::cout << StringFormat(" * Server Listening %s : %d", this->_socket_handler->get_server_ip_address(), this->_socket_handler->get_port());

}


bool ServerControlleFacade::accept_client_socket_process()
{
	std::thread::id this_thread_id = std::this_thread::get_id();
	SOCKET new_client_socket = INVALID_SOCKET;
	struct sockaddr_in new_client_sockaddr_in = { 0 };
	socklen_t new_client_socklen_t = 0;
	std::string new_client_ipv4_addr;

	//TCPソケットのアクセプト処理
	std::tie(new_client_socket, new_client_sockaddr_in) = _socket_handler->accept_client_socket(this->_client_handler->init_client.socket_address_in_length);
	if (new_client_socket == INVALID_SOCKET) {
		LOG_ERROR("The accepted socket for message could not be added to the list because it was an INVALID SOCKET");
		return ACCEPT_FAILED;
	}
	else {
		new_client_ipv4_addr = this->_socket_handler->extraction_client_ip(new_client_sockaddr_in, sizeof(new_client_ipv4_addr));
	}

	//SSLアクセプト処理
	WOLFSSL* new_client_ssl = this->_protocol_handler->wolfssl_accept(new_client_socket);
	if (new_client_ssl == nullptr) {
		LOG_ERROR("client ssl is not accepted(socket:{:d})", new_client_socket);
		//spdlog::error("client ssl is not accepted(socket:{:d})", new_client_socket);
		wolfSSL_free(new_client_ssl);
		return ACCEPT_FAILED;
	}
	else {
		spdlog::info("client ssl is accepted(socket:{:d})", new_client_socket);
		this->_protocol_handler->get_peer_cetificate(new_client_ssl);
	}

	//新規クライアントのインスタンスを生成
	ClientHandler::Client new_client = ClientHandler::Client(new_client_socket, new_client_sockaddr_in, new_client_socklen_t, new_client_ipv4_addr, new_client_ssl);

	{
		std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
		if (_client_handler->add_client(new_client) == false) {
			return ACCEPT_FAILED;
		}
	}
	spdlog::info("client ip : {:s} , success, server accept client(socket:{:d}).", new_client.ipv4_address, new_client.socket);
	return ACCEPT_SUCCEEDED;
}

const std::string& ServerControlleFacade::get_server_ip()
{
	return this->_server_ip;
}

void ServerControlleFacade::set_server_ip(const std::string& ip)
{
	this->_server_ip = ip;
}

//ソケットの変化を検知し、処理を振り分ける
void ServerControlleFacade::handle_changed_socket_process(const SOCKET socket)
{
	if (socket == this->_socket_handler->get_listen_socket()) {
		this->_thread_pool_handler->enqueue_task([this]() {accept_client_socket_process(); });
	}
	else {
		// 既存クライアントのデータを処理
		this->_thread_pool_handler->enqueue_task([this, socket]() {message_handle_process(socket); });
	}
}

//メッセージの処理を行う
void ServerControlleFacade::message_handle_process(const SOCKET socket)
{
	std::thread::id this_thread_id = std::this_thread::get_id();
	std::string receive_data, mt, data_time_stamp;
	json receive_json;
	bool disconnect_flag = false;
	int detection_num = -1;


#ifdef TLS_PROTOCOL
	{
		std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
		detection_num = this->_client_handler->find_socket_from_client_list(socket);
		auto& client = _client_handler->connected_clients[detection_num];
		if (detection_num <= this->_client_handler->connect_client_num - 1 && _client_handler->connected_clients.size() != 0) {
			std::tie(receive_json, disconnect_flag) = _protocol_handler->receive_message_tls(client.ssl);
		}
		else {
			return;
		}

		//クライアントの切断を検知した場合
		if (disconnect_flag == true) {
			
			spdlog::info("client(ip:{:s}, client id:{:s}) is disconnected, server initialize client info.", client.ipv4_address, client.id);
			this->_mysql_client->update_last_login(client.id, this->_server_time->get_current_time());
			this->_protocol_handler->shutdown_wolfssl(client.ssl);
			_socket_handler->disconnect_socket(client.socket);
			_client_handler->delete_client(detection_num);

			return;
		}
	}


	if (receive_json.empty() == true) {
		return;
	}
	

	mt = receive_json["message_type"];
	//mt000 = ハートビートを受信した際の処理
	if (mt == "000") {
		this->mt000_process(receive_data, detection_num);

	}
	//MT001 = Auth認証メッセージの場合
	else if (mt == "001") {
		auto mt001 = MessageType001::serialize_message(receive_json);
		if (mt001 == nullptr) {
			return;
		}
		{
			std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
			auto& client = _client_handler->connected_clients[detection_num];
			this->_protocol_handler->print_received_message_info(mt001, 0, "", "", client.ipv4_address, client.socket);
		}
		this->mt001_process(mt001, detection_num);

	}
	//MT002 = Group or Friend追加のメッセージの場合
	else if (mt == "002") {

	}
	//MT003 = 通常メッセージの場合
	else if (mt == "003") {
		auto mt003 = MessageType003::serialize_message(receive_json);
		if (mt003 == nullptr) {
			return;
		}
		{
			std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
			auto& client = _client_handler->connected_clients[detection_num];
			this->_protocol_handler->print_received_message_info(mt003, mt003->get_message_id(), mt003->get_sender_id(), mt003->get_receiver_id(), client.ipv4_address, client.socket);
		}
		this->mt003_process(mt003, detection_num);
	}

	else {
		LOG_ERROR("The receive process was interrupted due to a bad message.");
	}
	this->_protocol_handler->print_json_message(receive_json);


#endif // TLS_PROTOCOL



#ifdef TCP_PROTOCOL
	{
		std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
		auto& client = _client_handler->connected_client_list[detection_num];
		if (detection_num <= this->_client_handler->connect_client_num - 1 && _client_handler->connected_client_list.size() != 0) {
			std::tie(receive_data, disconnect_flag) = _socket_handler->receive_message(client.socket);
		}
		else {
			return;
		}
		if (disconnect_flag == true) {
			spdlog::info("client(ip:{:s}, client id:{:s}) is disconnected, server initialize client info.", client.get_client_ipv4_addr(), client.get_client_id());
			_socket_handler->disconnect_socket(client.get_client_socket());
			_client_handler->delete_client(detection_num);
			return;
		}
	}
	if (receive_data.empty() == true) {
		return;
	}
	mt = _protocol_handler->data_extraction_mt(receive_data);
	//mt000 = ハートビートを受信した際の処理
	if (mt == "000") {
		this->mt000_process(receive_data, detection_num);
	}
	//MT001 = Auth認証メッセージの場合
	else if (mt == "001") {
		auto mt001 = this->_mt001receiver->create_mt001_instance(receive_data);
		this->mt001_process(mt001, detection_num);
	}
	//MT002 = 通常メッセージの場合
	else if (mt == "002") {
		auto mt002 = this->_mt002receiver->create_mt002_instance(receive_data);
		this->mt002_process(mt002, detection_num);
	}
	else {
		LOG_ERROR("The receive process was interrupted due to a bad message.");
	} 
#endif // TCP_PROTOCOL


}

void ServerControlleFacade::mt000_process(std::string& receive_data, int detection_num)
{
	//std::string src_client_id = "";
	//std::string* client_id_ptr = new std::string();
	//try
	//{
	//	src_client_id = this->_mt000receiver->data_extraction_(receive_data);
	//	if(src_client_id.size() != 28){
	//		THROW_APP_EXCEPTION("The reception process was interrupted because the contents of MT000 were invalid.");
	//	}

	//	auto mt000_instance = this->_mt000receiver->create_mt000_instance(src_client_id);
	//	{
	//		std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
	//		if (mt000_instance->get_src_client_id() == this->_client_handler->connected_clients[detection_num].get_client_id()) {
	//			this->_client_handler->incliment_heart_beat_count(detection_num);
	//		}
	//	}
	//	
	//}
	//catch (const exception::ExceptionBase& ex)
	//{
	//	spdlog::error(ex.what_for_str());
	//	//LogHandler::get_instance().logging_error(ex.what_for_str());
	//}
}

void ServerControlleFacade::mt001_process(const std::unique_ptr<MessageType001>& mt001, int detection_num)
{
	std::string client_id = "";
	std::string* client_id_ptr = new std::string();

	try
	{
		if (mt001 == nullptr || mt001->is_valid() == false) {
			THROW_APP_EXCEPTION("The reception process was interrupted because the contents of MT001 were invalid.");
		}
		switch (mt001->get_sign_type_int())
		{
		case 0:			//sign up
			//メッセージがsign_upの場合の処理を追記
		{
			std::unique_lock<std::mutex> lock(this->_connect_client_list_mutex);
			auto& client = this->_client_handler->connected_clients[detection_num];
			client_id = this->sign_up_process(mt001->get_client_mail_addr(), mt001->get_client_password(), client.ipv4_address);
			if (client_id != "" && detection_num <= this->_client_handler->connect_client_num - 1 && _client_handler->connected_clients.size() != 0) {
				spdlog::info("Client(ip:{:s}, socket num:{:d}) successfully signed up, Client id:{:s}", client.ipv4_address, client.socket, client_id);
				//SIGN UPが成功した場合に、クライアント情報を登録する
				client.id = client_id;
				client.name = mt001->get_client_name();
				client.mail_address = mt001->get_client_mail_addr();
				client.password = mt001->get_client_password();
				
				//データベースにユーザ情報を登録する
				if (insert_client_info_to_db(client_id, mt001->get_client_name(), mt001->get_client_mail_addr(), mt001->get_client_password()) == true) {
					spdlog::info("Client(ip:{:s}, socket num:{:d}) information has been successfully registered in the database.", client.ipv4_address, client.socket, client_id);
				}
				else {
					LOG_ERROR("Client(ip:{:s}, socket num:{:d}) information has been failed registered in the database.", client.ipv4_address, client.socket, client_id);
				}
				
				//Auth statusを変更する
				this->_client_handler->change_client_auth_status(detection_num);
			}
			this->_client_handler->show_client_info(detection_num);
		}
		break;

		case 1:			//sign in
		{
			std::unique_lock<std::mutex> lock(this->_connect_client_list_mutex);
			auto& client = this->_client_handler->connected_clients[detection_num];
			client_id = this->sign_in_process(mt001->get_client_mail_addr(), mt001->get_client_password(), client.ipv4_address, client_id_ptr);
			
			if (client_id != "" && detection_num <= this->_client_handler->connect_client_num - 1 && _client_handler->connected_clients.size() != 0) {
				spdlog::info("Client(ip:{:s}, socket num:{:d}) successfully signed in, Client id:{:s}", client.ipv4_address, client.socket, client_id);
			
				
				//データベースからユーザ情報を読み取る。			
				std::string client_name, client_mail_addr, client_password;
				std::tie(client_name, client_mail_addr, client_password) = this->_mysql_client->select_client_info(client_id);
				client.id = client_id;
				client.name = client_name;
				client.mail_address = client_mail_addr;
				client.password = client_password;
				spdlog::debug("client name:{:s}, mail address:{:s}, password:{:s}", client_name, client_mail_addr, client_password);

				//TODO　ここでcidに関するメッセージをデータベースから取得する。
				json unread_messages_json;
				unread_messages_json = this->_mysql_client->get_unread_message_fromDB(client_id, 20);
				
				//Auth statusを変更する
				this->_client_handler->change_client_auth_status(detection_num);

				//TODO
				//MT101を送信する処理を記載する
				auto mt101 = std::make_unique<MessageType101>(client_id, true);
				json mt101_json = MessageType101::desirialize_message(mt101, unread_messages_json, "No Error.");
				spdlog::debug("mt101_json : {}", mt101_json.dump(4));
			}
		}
		break;

		case 2:			//sign out
		{
			std::unique_lock<std::mutex> lock(this->_connect_client_list_mutex);
			auto& client = this->_client_handler->connected_clients[detection_num];
			_firebase_auth_handler->sign_out(client.id, client.mail_address);
			break;
		}
		default:
			THROW_APP_EXCEPTION("MT001 authentication data region is in violation.");
			break;
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		LOG_ERROR(ex.what_for_str());
	}
	delete client_id_ptr;

}




void ServerControlleFacade::mt003_process(const std::unique_ptr<MessageType003>& mt003, int& receive_result)
{
	std::vector<SOCKET> send_sockets;
	std::vector<std::string> receive_clients;
	int chatroom_list_num = -1;
	int send_socket_num = 0;
	bool client_db_exist_flag = false;

	try
	{
		if (mt003 == nullptr || mt003->is_valid() == false) {
			// メッセージ処理
			THROW_APP_EXCEPTION("The reception process was interrupted because the contents of MT002 were invalid.");
		}

		{
			std::unique_lock<std::mutex> lock(this->_connect_client_list_mutex);
			send_socket_num = _client_handler->search_client_id_from_con_list(mt003->get_receiver_id());
			spdlog::debug("send_socket_num:{:d}", send_socket_num);

			//データベースにクライアントIDが存在するか確認する
			if (send_socket_num == -1) {
				client_db_exist_flag = this->_mysql_client->is_client_id_exist(mt003->get_receiver_id());
				if (client_db_exist_flag == false) {
					LOG_ERROR("The client ID was not found in the database.");
					THROW_APP_EXCEPTION("The reception process was interrupted because the destination client ID was not found.");
				}
			}
			//メッセージをデータベースに保存する
			if (this->_mysql_client->insert_chat_message(mt003->get_message_id(), mt003->get_room_id(), mt003->get_sender_id(), mt003->get_send_time(), mt003->get_content())) {
				spdlog::info("The message was successfully saved in the database.");
			}
			else {
				LOG_ERROR("The message was not saved in the database.");
			}



			//=========================================================================================
			//ここにメッセージの送信処理を記載する
			//=========================================================================================

			//switch (mt002->get_trans_type_int())
			//{
			//	//TCPユニキャスト
			//case 0:
			//	_sender->tcp_unicast_data(_client_handler->connected_client_list[send_socket_num].get_client_socket(),
			//		receive_json, receive_result, mt002->get_dst_client_id(), mt002->get_src_client_id());
			//	break;

			//	//TCPでマルチキャスト
			//case 1:
			//	//TODO　以下を実装する
			//	//mt002のパケットの中身がTCPマルチキャストの場合(グループ内でのチャットの時)、グループチャットから
			//	chatroom_list_num = this->_chat_room_handler->search_chatroom_id_from_list(mt002->get_dst_chatroom_id());
			//	for (auto&& client : _chat_room_handler->get_client_list(chatroom_list_num)) {
			//		send_sockets.push_back(client.get_client_socket());
			//	}
			//	for (auto&& client : _chat_room_handler->get_client_list(chatroom_list_num)) {
			//		dst_clients.push_back(client.get_client_id());
			//	}
			//	_sender->tcp_multicast_data(send_sockets, receive_json, receive_result);
			//	break;

			//	//UDPブロードキャスト
			//case 2:
			//	//TODO　UDPのブロードキャストを実装する
			//	break;

			//default:
			//	break;
			//}
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		LOG_ERROR(ex.what_for_str());
		return;
	}


}


void ServerControlleFacade::heart_beat_check_process()
{
	while (true) {
		
		//auto time_now = std::chrono::system_clock::now();

		//現在接続しているクライアントリストを抽出
		std::vector<ClientHandler::Client> current_connect_clients;
		{
			std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
			current_connect_clients = this->_client_handler->connected_clients;
		}

		//30秒wait
		/*auto abs_time_30 = time_now + std::chrono::seconds(30);
		std::this_thread::sleep_until(abs_time_30);*/

		std::this_thread::sleep_for(std::chrono::seconds(30));  // 30秒間隔

		for (const auto& client : current_connect_clients) {
			if (client.heart_beat_count == 0) {
				this->heart_beat_response_process(client);
			}
		}
		
		//ハートビート回数をクリアする
		{
			std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
			this->_client_handler->all_clear_heart_beat_count();
		}
				
	}
}


void ServerControlleFacade::heart_beat_response_process(const ClientHandler::Client& client)
{
	std::unique_lock<std::mutex> lock(_connect_client_list_mutex);

	// クライアントが still connected か確認
	auto& clients = _client_handler->connected_clients;
	auto it = std::find(clients.begin(), clients.end(), client);

	if (it == clients.end() || it->heart_beat_count > 0) {
		return;  // 既に削除されている or ハートビートがリセットされた場合
	}

	// ログ出力
	if (it->auth_status == true) {
		spdlog::info("Client (Socket : {:d} ID: {:s}), disconnect.(Not response)", it->socket, it->id);
	
	}
	else {
		spdlog::info("Client (Socket : {:d} ID: None), disconnect.(Not response)", it->socket);

	}

	// クライアント切断処理
	_socket_handler->disconnect_socket(it->socket);

	// クライアント削除（イテレータを使って安全に削除）
	this->_client_handler->delete_client(it);
}


void ServerControlleFacade::setting_select_process()
{
	while (true) {
		{
			std::unique_lock<std::mutex> lock(_mutex);
			this->_socket_handler->setting_select_info_to_listen_socket();
			this->setting_select_info();
			
		}
	}
}

std::string ServerControlleFacade::sign_up_process(const std::string& mail_addr, const std::string& password, const std::string& ip)
{
	std::string user_id = this->_firebase_auth_handler->create_user_with_email_and_password(mail_addr, password, ip
		, &FirebaseAuthHandler::on_auth_callback);
	return user_id;
}

std::string ServerControlleFacade::sign_in_process(const std::string& mail_addr, const std::string& password, const std::string& ip, std::string* id)
{
	std::string client_id = "";
	this->_firebase_auth_handler->sign_in_user_with_email_and_password(mail_addr, password, ip, id);
	client_id = *id;
	return client_id;
}

std::string ServerControlleFacade::sign_in_process(int& auth_type, const char* token, const char* secret)
{
	firebase::auth::Credential credential;
	switch (auth_type)
	{
	case 1:		//google auth
		credential = firebase::auth::GoogleAuthProvider::GetCredential(token, nullptr);
		break;
	case 2:		//facebook auth
		credential = firebase::auth::FacebookAuthProvider::GetCredential(token);
		break;
	case 3:		//twitter auth
		credential = firebase::auth::TwitterAuthProvider::GetCredential(token, secret);
		break;
	case 4:		//microsoft auth
		
		break;
	case 5:		//yahoo auth
		break;
	default:
		break;
	}
	return std::string();
}

bool ServerControlleFacade::insert_client_info_to_db(const std::string& id, const std::string& name, const std::string& mail_addr, const std::string& password_hash)
{
	// 現在時刻を取得してDATETIME形式に変換
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	std::tm tm;
	localtime_s(&tm, &now_c);  // 安全な関数 localtime_s を使用

	std::ostringstream datetime_ss;
	datetime_ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
	std::string datetime_create_at_str = datetime_ss.str();
	std::string datetime_last_login = "";

	//データベースにユーザ情報を登録する。
	if (this->_mysql_client->insert_client_data(id, name, mail_addr, password_hash, datetime_create_at_str, datetime_last_login) == true) {
		return true;
	}
	else {
		LOG_ERROR("Client information was not registered in the database.");
		return false;
	}
}




