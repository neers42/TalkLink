#include "ChatServerFacade.h"

std::time_t ServerTime::current_time;
std::tm ServerTime::time_info;

ChatServerFacade::ChatServerFacade() : _server_ip{ "" }
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
	_socket_handler = std::make_unique<SocketHandler>("conf/socket_setting.ini");
	this->set_server_ip(this->_socket_handler->obtain_local_ip_addr());
	_firebase_auth_handler = std::make_unique<FirebaseAuthHandler>();
	_mysql_client = std::make_unique<MySQLClient>();
	_redis_client = std::make_unique<RedisClient>();
	_protocol_handler = std::make_unique<ProtocolHandler>();
	_server_time = std::make_unique<ServerTime>();
	_client_handler = std::make_unique<ClientHandler>();
	_thread_pool_handler = std::make_unique<ThreadPoolHandler>(10);
	_chat_room_handler = std::make_unique<ChatroomHandler>();
	
	if (_socket_handler == nullptr ||
		_mysql_client == nullptr ||
		_redis_client == nullptr ||
		_protocol_handler == nullptr ||
		_server_time == nullptr ||
		_client_handler == nullptr ||
		_thread_pool_handler == nullptr ||
		_chat_room_handler == nullptr) {
		LOG_ERROR("Failed to initialize ChatServerFacade components. Please check the configuration and try again.");
		exit(1);
	}
}

ChatServerFacade::~ChatServerFacade()
{
	
}

//サーバプログラムの実行
void ChatServerFacade::run()
{
	//リッスンソケットの生成
	if (_socket_handler->open_tcp_listen_socket() == false) {
		LOG_ERROR("The TCP listening socket for message could not be opened properly, so the program is forcibly terminated. Please set it again.");
		exit(1);
	}

	if (_socket_handler->generate_udp_multicast_tx_socket() == false) {
		LOG_ERROR("The program terminates because it failed to create a socket for multicast transmission. Please reset the settings.");
		exit(1);
	}
	
	this->_mysql_client->connect();
	this->_mysql_client->create_database();
	this->_mysql_client->execute_query("USE mychat");
	this->_mysql_client->create_table();
	this->_redis_client->connect();

	spdlog::info("Start monitoring requests from clients.");
	this->_thread_pool_handler->enqueue_task([this]() {heartbeat_check_process(); });

	//ディスクリプション監視スタート
	while (1) {
		auto abs_time = std::chrono::system_clock::now() + std::chrono::milliseconds(1000);
		this->_socket_handler->initialize_select_param();
		this->prepare_client_fd_set();
		std::vector<SOCKET> changed_client_sockets = this->_client_handler->get_all_connected_sockets();
		_socket_handler->monitor_socket_descriptor(changed_client_sockets, 
			[this](SOCKET changed_socket) {handle_changed_socket_process(changed_socket); });
		std::this_thread::sleep_until(abs_time);

	}
	
}

void ChatServerFacade::prepare_client_fd_set()
{
	auto clients_snapshot = this->_client_handler->get_connected_clients_snapshot();
	for (auto& client : clients_snapshot) {
		//有効なソケットにFDSETを実行する
		if (client != nullptr && client->get_socket() != INVALID_SOCKET) {
			FD_SET(client->get_socket(), &_socket_handler->get_fd_read());
		}
	}
	this->_socket_handler->copy_fd_read_to_fds(&(_socket_handler->get_fds()), &(_socket_handler->get_fd_read()));
}


void ChatServerFacade::show_server_running_info()
{
	std::cout << " * TLS Version : " << this->_protocol_handler->get_tls_version(this->_protocol_handler->get_wolfssl_object()) << std::endl;
	std::cout << " * Cipher suite : " << this->_protocol_handler->get_cipher_suite(this->_protocol_handler->get_wolfssl_object()) << std::endl;
	std::cout << StringFormat(" * Server Listening %s : %d", this->_socket_handler->get_server_ip_address(), this->_socket_handler->get_port());

}


bool ChatServerFacade::accept_client_socket_process()
{
	std::thread::id this_thread_id = std::this_thread::get_id();
	SOCKET new_client_socket = INVALID_SOCKET;
	struct sockaddr_in new_client_sockaddr_in = { 0 };
	socklen_t new_client_socklen_t = 0;
	std::string new_client_ipv4_addr;

	//TCPソケットのアクセプト処理
	std::tie(new_client_socket, new_client_sockaddr_in) = _socket_handler->accept_client_socket(_client_handler->get_init_client().get_socket_address_in_length());
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
		wolfSSL_free(new_client_ssl);
		return ACCEPT_FAILED;
	}
	else {
		spdlog::info("client ssl is accepted(socket:{:d})", new_client_socket);
		this->_protocol_handler->get_peer_cetificate(new_client_ssl);
	}

	//新規クライアントのインスタンスを生成
	auto new_client = std::make_unique<Client>(new_client_socket, new_client_sockaddr_in, new_client_socklen_t, new_client_ipv4_addr, new_client_ssl);
	spdlog::info("client ip : {:s} , success, server accept client(socket:{:d}).", new_client->get_ipv4_address(), new_client->get_socket());
	if (_client_handler->add_client(std::move(new_client)) == true) {
		return ACCEPT_SUCCEEDED;
	}
	else {
		return ACCEPT_FAILED;
	}
	
}

const std::string& ChatServerFacade::get_server_ip()
{
	return this->_server_ip;
}

void ChatServerFacade::set_server_ip(const std::string& ip)
{
	this->_server_ip = ip;
}

//ソケットの変化を検知し、処理を振り分ける
void ChatServerFacade::handle_changed_socket_process(const socket_t socket)
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
void ChatServerFacade::message_handle_process(const socket_t socket)
{
	std::thread::id this_thread_id = std::this_thread::get_id();
	std::string receive_data, mt, data_time_stamp;
	json receive_json;
	bool disconnect_flag = false;
	bool recv_flag = false;
	int detection_num = -1;
	std::string* error_msg = new std::string();
	bool* result = new bool();
	char buf[32];
	std::shared_ptr<Client> client;
	WOLFSSL* client_ssl = nullptr;

#ifdef TLS_PROTOCOL
	try
	{
		auto client = _client_handler->get_client_by_socket(socket);
		if (client == nullptr) {
			LOG_ERROR("Client not found for detection number: {:d}", detection_num);
			return;
		}
		if ((_socket_handler->is_socket_closed(client->get_socket()) == true)) {
			spdlog::warn("Client closed tcp connection.");
			this->handle_disconnect_process(client);
			return;
		}
		if (client->get_ssl() != nullptr) {
			std::tie(receive_json, disconnect_flag, recv_flag) = _protocol_handler->receive_message_tls(client);
		}
	

		//クライアントの切断を検知した場合
		if (disconnect_flag == true) {
			spdlog::info("client(ip:{:s}, client id:{:s}) is disconnected, server initialize client info.", client->get_ipv4_address(), client->get_id());
			this -> handle_disconnect_process(client);
			return;
		}


		if (receive_json.empty() == true) {
			return;
		}


		mt = receive_json["message_type"];
		//mt000 = ハートビートを受信した際の処理
		if (mt == "000") {
			auto mt000 = MessageType000::serialize_message(receive_json);
			this->mt000_process(mt000, client);

		}
		//MT001 = Auth認証メッセージの場合
		else if (mt == "001") {
			auto mt001 = MessageType001::serialize_message(receive_json);
			if (mt001 == nullptr) {
				return;
			}
			this->_protocol_handler->print_received_message_info(mt001, 0, "", "", client->get_ipv4_address(), client->get_socket());
			this->_protocol_handler->print_json_message(receive_json);
			this->mt001_process(mt001, client, error_msg, result);
			this->mt101_process(client, *error_msg, *result, recv_flag);

		}
		//MT002 = Group or Friend追加のメッセージの場合
		else if (mt == "002") {

		}
		else if (mt == "003") {
			auto mt003 = MessageType003::serialize_message(receive_json);
			if (mt003 == nullptr) {
				return;
			}
			this->_protocol_handler->print_received_message_info(mt003, mt003->get_message_id(), mt003->get_sender_id(), "", client->get_ipv4_address(), client->get_socket());
			this->_protocol_handler->print_json_message(receive_json);
			this->mt003_process(mt003, error_msg, result);

		}
		//MT004 = 通常メッセージの場合
		else if (mt == "004") {
			auto mt004 = MessageType004::serialize_message(receive_json);
			if (mt004 == nullptr) {
				return;
			}
			this->_protocol_handler->print_received_message_info(mt004, mt004->get_message_id(), mt004->get_sender_id(), mt004->get_receiver_id(), client->get_ipv4_address(), client->get_socket());
			this->_protocol_handler->print_json_message(receive_json);
			this->mt004_process(mt004);
			this->mt104_process(mt004, client);
		}

		else {
			LOG_ERROR("The receive process was interrupted due to a bad message.");
		}
	}
	catch (const std::exception& e)
	{
		LOG_ERROR(e.what());
	}





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

void ChatServerFacade::mt000_process(const std::unique_ptr<MessageType000>& mt000, std::shared_ptr<Client>& client)
{
	try
	{
		if (mt000 == nullptr || mt000->is_valid() == false) {
			throw std::invalid_argument("The reception process was interrupted because the contents of MT000 were invalid.");
		}
		if (mt000->get_client_id() == client->get_id()) {
			spdlog::info("client(ip:{:s}, client id:{:s}) send HeartBeat.", client->get_ipv4_address(), client->get_id());
			this->_client_handler->incliment_heart_beat_count(client);
		}
		else {
			spdlog::error("The reception process was interrupted because the contents of MT000 were invalid.");
			return;
			
		}
	}
	catch (const std::invalid_argument& e) {
		LOG_ERROR(e.what());
	}
		
}

void ChatServerFacade::mt001_process(const std::unique_ptr<MessageType001>& mt001, std::shared_ptr<Client>& client, std::string* error_msg_text, bool* login_result)
{
	std::string client_id = "";
	std::string* client_id_ptr = new std::string();
	std::string* error_msg_ptr = new std::string();

	try
	{
		if (mt001 == nullptr || mt001->is_valid() == false) {
			THROW_APP_EXCEPTION("The reception process was interrupted because the contents of MT001 were invalid.");
		}
		switch (mt001->get_sign_type_int())
		{
		case 0:			//sign up
			//メッセージがsign_upの場合の処理を追記
			std::tie(client_id, *error_msg_text) = this->sign_up_process(mt001->get_client_mail_addr(), mt001->get_client_password(), client->get_ipv4_address(), client_id_ptr, error_msg_ptr);
			spdlog::debug("client id : {} error_code : {}", client_id, *error_msg_text);
			{
				//std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
				if (client_id != "" && _client_handler->get_connect_clients_num() != 0) {
					spdlog::info("Client(ip:{:s}, socket num:{:d}) successfully signed up, Client id:{:s}", client->get_ipv4_address(), client->get_socket(), client_id);
					*login_result = true;
					//SIGN UPが成功した場合に、クライアント情報を登録する
					_client_handler->set_client_id(client, client_id);
					_client_handler->set_client_name(client, mt001->get_client_name());
					_client_handler->set_client_mail_addr(client, mt001->get_client_mail_addr());
					_client_handler->set_client_password(client, mt001->get_client_password());

					if (this->_mysql_client->insert_client_data(client_id, mt001->get_client_name(), mt001->get_client_mail_addr(), mt001->get_client_password(), "", ServerTime::get_current_time_to_seconds(), "") == true) {
						spdlog::info("Client(ip:{:s}, socket num:{:d}) information has been successfully registered in the database.", client->get_ipv4_address(), client->get_socket(), client_id);
					}
					else {
						LOG_ERROR("Client(ip:{:s}, socket num:{:d}) information has been failed registered in the database.", client->get_ipv4_address(), client->get_socket(), client_id);
					}

					//Auth statusを変更する
					this->_client_handler->change_client_auth_status(client);
					//ハートビートのカウント回数をインクリメントする
					this->_client_handler->incliment_heart_beat_count(client);
				}
				else
				{
					*login_result = false;
				}

			}
			
			this->_client_handler->show_client_info(client);

			break;

		case 1:			//sign in
			std::tie(client_id, *error_msg_text) = this->sign_in_process(mt001->get_client_mail_addr(), mt001->get_client_password(), client->get_ipv4_address(), client_id_ptr, error_msg_ptr);
			spdlog::debug("client id : {} error_code : {}", client_id, *error_msg_text);

			{
				//std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
				if (client_id != "" && _client_handler->get_connect_clients_num() != 0) {
					spdlog::info("Client(ip:{:s}, socket num:{:d}) successfully signed in, Client id:{:s}", client->get_ipv4_address(), client->get_socket(), client_id);
					*login_result = true;

					//データベースからユーザ情報を読み取る。			
					std::string client_name, client_mail_addr, client_password;
					std::tie(client_name, client_mail_addr, client_password) = this->_mysql_client->get_client_info_fromDB(client_id);
					_client_handler->set_client_id(client, client_id);
					_client_handler->set_client_name(client, client_name);
					_client_handler->set_client_mail_addr(client, client_mail_addr);
					_client_handler->set_client_password(client, client_password);

					spdlog::debug("client name:{:s}, mail address:{:s}, password:{:s}", client_name, client_mail_addr, client_password);

					//Auth statusを変更する
					this->_client_handler->change_client_auth_status(client);
					//ハートビートのカウント回数をインクリメントする
					this->_client_handler->incliment_heart_beat_count(client);
				}
				else {
					*login_result = false;
				}
			}
		
	
			break;

		case 2:			//sign out
			_firebase_auth_handler->sign_out(client->get_id(), client->get_mail_address());
			break;

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

void ChatServerFacade::mt003_process(const std::unique_ptr<MessageType003>& mt003, std::string* error_msg, bool* action_result)
{
	switch (mt003->get_room_action())
	{

	case MessageType003::RoomAction::CREATE_ROOM:
		//ルームの作成
	{
		auto new_room = this->_chat_room_handler->create_chatroom(mt003->get_room_name(), mt003->get_room_image_url(), mt003->get_add_member_list());
		if (new_room == nullptr) {
			LOG_ERROR("The chat room was not created.");
			*action_result = false;
			*error_msg = "The chat room was not created.";
			return;
		};

		//チャットルームのDB保存
		*action_result = this->_mysql_client->insert_chatroom(new_room->get_chatroom_id(), new_room->get_chatroom_name(), new_room->get_chatroom_icon(), ServerTime::get_current_time_to_seconds());
		if (*action_result == false) {
			LOG_ERROR("The chat room was not saved in the database.");
			*error_msg = "The chat room was not saved in the database.";
			return;
		}
		*action_result = this->_mysql_client->insert_chatroom_member(new_room->get_chatroom_id(), mt003->get_add_member_list(), ServerTime::get_current_time_to_seconds());
		if (*action_result == false) {
			LOG_ERROR("The chat room member was not saved in the database.");
			*error_msg = "The chat room member was not saved in the database.";
			return;
		}
		if (*action_result == true) {
			spdlog::info("The chat room(ID:{}) was successfully created and saved in the database.", new_room->get_chatroom_id());
			*error_msg = "No Error.";
		}
		else {
			*error_msg = "The chat room(ID:{}) was not created.", new_room->get_chatroom_id();
			return;
		}
		break;
		this->_chat_room_handler->add_chatroom(std::move(new_room));
		auto chatrooms = this->_chat_room_handler->get_chatrooms_snapshot();
		for (const auto& room : chatrooms) {
			spdlog::debug("room id : {:s}", room->get_chatroom_id());
		}
				
	}	

	case MessageType003::RoomAction::ADD_MEMBER:
		//ルームのメンバー追加
		*action_result = this->_chat_room_handler->add_member_to_chatroom(mt003->get_room_id(), mt003->get_add_member_list());

		//データベースにメンバーを追加
		*action_result = this->_mysql_client->insert_chatroom_member(mt003->get_room_id(), mt003->get_add_member_list(), ServerTime::get_current_time_to_seconds());
	
		if (*action_result == true) {
			spdlog::info("The chat room(ID:{}) was successfully added and saved in the database.", mt003->get_room_id());
			*error_msg = "No Error.";
		}
		else {
			*error_msg = "The chat room(ID:{}) was not created.", mt003->get_room_id();
			return;
		}

		break;

	case MessageType003::RoomAction::REMOVE_MEMBER:
		//ルームのメンバー削除
		*action_result = this->_chat_room_handler->delete_member_from_chatroom(mt003->get_room_id(), mt003->get_remove_member_list());

		//データベースからメンバーを削除
		*action_result = this->_mysql_client->delete_chatroom_member(mt003->get_room_id(), mt003->get_remove_member_list());

		if (*action_result == true) {
			spdlog::info("The chat room(ID:{}) was successfully removed and saved in the database.", mt003->get_room_id());
			*error_msg = "No Error.";
		}
		else {
			*error_msg = "The chat room(ID:{}) was not created.", mt003->get_room_id();
			return;
		}
		break;


	case MessageType003::RoomAction::ROOM_NAME_CHANGE:
		//ルーム名の変更
		*action_result = this->_chat_room_handler->change_room_name(mt003->get_room_id(), mt003->get_room_name());
		*action_result = this->_mysql_client->update_chatroom_name(mt003->get_room_id(), mt003->get_room_name());
		if (*action_result == true) {
			spdlog::info("The chat room(ID:{}) was successfully changed and saved in the database.", mt003->get_room_id());
			*error_msg = "No Error.";
		}
		else {
			*error_msg = "The chat room(ID:{}) was not created.", mt003->get_room_id();
			return;
		}
		break;

	case MessageType003::RoomAction::ROOM_ICON_CHANGE:
		//ルームアイコンの変更
		*action_result = this->_chat_room_handler->change_room_icon(mt003->get_room_id(), mt003->get_room_image_url());
		*action_result = this->_mysql_client->update_chatroom_icon(mt003->get_room_id(), mt003->get_room_image_url());
		if (*action_result == true) {
			spdlog::info("The chat room(ID:{}) was successfully changed and saved in the database.", mt003->get_room_id());
			*error_msg = "No Error.";
		}
		else {
			*error_msg = "The chat room(ID:{}) was not created.", mt003->get_room_id();
			return;
		}
		break;

	default:
		break;
	}
}




void ChatServerFacade::mt004_process(const std::unique_ptr<MessageType004>& mt004)
{
	std::vector<SOCKET> send_sockets;
	std::vector<std::string> receive_clients;
	int chatroom_list_num = -1;
	int send_socket_num = 0;
	bool client_db_exist_flag = false;

	try
	{
		if (mt004 == nullptr || mt004->is_valid() == false) {
			// メッセージ処理
			THROW_APP_EXCEPTION("The reception process was interrupted because the contents of MT002 were invalid.");
		}
		send_socket_num = _client_handler->search_cid_from_clients(mt004->get_receiver_id());
		spdlog::debug("send_socket_num:{:d}", send_socket_num);
		//データベースにクライアントIDが存在するか確認する
		if (send_socket_num == -1) {
			client_db_exist_flag = this->_mysql_client->is_client_id_exist(mt004->get_receiver_id());
			if (client_db_exist_flag == false) {
				LOG_ERROR("The client ID was not found in the database.");
				THROW_APP_EXCEPTION("The reception process was interrupted because the destination client ID was not found.");
			}
		}
		//メッセージをデータベースに保存する
		if (this->_mysql_client->is_room_id_hex_exist(mt004->get_room_id())) {
			if (this->_mysql_client->insert_chat_message(mt004->get_message_id(), mt004->get_room_id(), mt004->get_sender_id(), mt004->get_send_time(), mt004->get_content())) {
				spdlog::info("The message was successfully saved in the database.");
			}
			else {
				LOG_ERROR("The message was not saved in the database.");
			}
		}
		else {
			THROW_APP_EXCEPTION("The reception process was interrupted because the room ID was not found.");
		}
		

	}
	catch (const exception::ExceptionBase& ex)
	{
		LOG_ERROR(ex.what_for_str());
		return;
	}


}

void ChatServerFacade::mt101_process(
	std::shared_ptr<Client>& client, 
	const std::string& error_msg, 
	const bool login_result, 
	const bool recv_flag)
{
	if (recv_flag == false) {
		LOG_ERROR("The reception process was interrupted because the MT001 message was not received properly.");
		return;
	}
	//TODO　ここでcidに関するメッセージをデータベースから取得する。
	json unsend_messages_json, unsend_control_messages_json;
	//未送信コントロールメッセージを取得する
	unsend_control_messages_json = this->_mysql_client->get_unsend_control_message_fromDB(client->get_id());
	//未送信チャットメッセージを取得する
	unsend_messages_json = this->_mysql_client->get_unsend_message_fromDB(client->get_id(), 20);
	
	//MT101を送信する処理を記載する
	auto mt101 = std::make_unique<MessageType101>(client->get_id(), login_result);
	json mt101_json = MessageType101::deserialize_message(mt101, unsend_messages_json, error_msg);
	//bool send_result = this->_protocol_handler->send_message_tls(_client_handler->search_client_ssl_from_clients(client->get_id()), mt101_json);
	bool send_result = this->_protocol_handler->send_message_tls(client, mt101_json);
	if (send_result == true) {
		spdlog::info("The MT101 message was successfully sent to the client(ip:{}, id:{}).", client->get_ipv4_address(), client->get_id());
		
		if (unsend_control_messages_json.empty() != true) {
			for (const auto& unsend_control_message_json : unsend_control_messages_json) {
				//データベースから未送信コントロールメッセージを削除する
				const auto mid = unsend_control_message_json["mid"].get<uint64_t>();
				const auto cid = unsend_control_message_json["cid"].get<std::string>();
				const auto rid = unsend_control_message_json["room_id"].get<std::string>();
				this->_mysql_client->delete_unsend_control_message(mid, cid, rid);
			}
		}

		if (unsend_messages_json.empty() != true) {
			//unsend_messageを削除する
			for (const auto& unsend_message : unsend_messages_json) {
				//データベースから未送信メッセージを削除する
				const auto mid = unsend_message["mid"].get<uint64_t>();
				const auto cid = unsend_message["sender_cid"].get<std::string>();
				const auto rid = unsend_message["send_room_id"].get<std::string>();
				this->_mysql_client->delete_unsend_message(mid, cid, rid);
			}
		}
	}
	else {
		spdlog::error("The MT101 message was not sent to the client.");
		//TODO MT101の送信に失敗した場合の処理を記載する⇒再送処理する
	}
}

void ChatServerFacade::mt103_process(const std::unique_ptr<MessageType003>& mt003, const std::shared_ptr<Client>& client)
{
	std::vector<std::string> unsend_cids;
	unsend_cids = this->mt103A_process(mt003);

}

std::vector<std::string> ChatServerFacade::mt103A_process(const std::unique_ptr<MessageType003>& mt003)
{
	std::vector<std::string> unsend_cids;
	std::vector<MessageType103A::ClientInfo> client_infos;

	//MT103Aを生成する
	switch (mt003->get_room_action())
	{
	case MessageType003::RoomAction::CREATE_ROOM:
	case MessageType003::RoomAction::ADD_MEMBER:
	case MessageType003::RoomAction::REMOVE_MEMBER:
		client_infos = this->generate_client_infos(mt003->get_add_member_list());
		break;
	default:
		break;
	}
	auto mt103A = std::make_unique<MessageType103A>(
		mt003->get_sender_id(),
		mt003->get_room_id(),
		mt003->get_room_name(),
		mt003->get_room_image_url(),
		client_infos,
		mt003->get_room_action()
		);
	this->send_message_to_room<MessageType103A>(mt103A, &unsend_cids);

	return unsend_cids;
}

void ChatServerFacade::mt103B_process(
	const std::unique_ptr<MessageType003>& mt003, 
	const std::vector<std::string>& unsend_cids,
	const std::shared_ptr<Client>& client)
{
	std::string message_type = "103B";
	MessageType103B::MessageSendStatus send_status;
	std::string send_room_id = mt003->get_room_id();
	//送り先のclientの情報を取得する
	auto receivers_infos = this->_chat_room_handler->get_client_infos_by_chatroom_id(send_room_id);
	//ChatroomHandlerが保持しているクライアントリストからチャットルームを検出できない場合
	if (receivers_infos.size() == 0) {
		//データベースからチャットルームに参加しているcidのリストを抽出
		auto client_list = this->_mysql_client->get_client_list_from_chatroom(send_room_id);
		if (client_list.size() == 0 && unsend_cids.size() == 0) {
			//LOG_ERROR("The Chatroom ID was not found in the database.");
			send_status = MessageType103B::MessageSendStatus::SEND_FAILED;

		}
		else if (client_list.size() != 1 && client_list.size() - 1 == unsend_cids.size()) {
			//全てのクライアントにメッセージが送信できなかった場合
			send_status = MessageType103B::MessageSendStatus::SEND_FAILED;
		}
		else if (client_list.size() > 0 && unsend_cids.size() == 0) {
			//クライアントにメッセージが送信できた場合
			send_status = MessageType103B::MessageSendStatus::SEND_SUCCESS;
		}

		else {		//一部のクライアントにメッセージが送信できなかった場合
			send_status = MessageType103B::MessageSendStatus::SEND_PARTICAL_SUCCESS;
		}

	}
	else {
		if (receivers_infos.size() == 0 && unsend_cids.size() == 0) {
			//全てのクライアントにメッセージが送信できなかった場合
			send_status = MessageType103B::MessageSendStatus::SEND_FAILED;
		}
		else if (receivers_infos.size() - 1 == unsend_cids.size()) {
			//全てのクライアントにメッセージが送信できなかった場合
			send_status = MessageType103B::MessageSendStatus::SEND_FAILED;
		}
		else if (receivers_infos.size() > 0 && unsend_cids.size() == 0) {
			//クライアントにメッセージが送信できた場合
			send_status = MessageType103B::MessageSendStatus::SEND_SUCCESS;
		}
		else {		//一部のクライアントにメッセージが送信できなかった場合
			send_status = MessageType103B::MessageSendStatus::SEND_PARTICAL_SUCCESS;

		}
	}
	auto mt103B = std::make_unique<MessageType103B>(
		0,
		message_type,
		send_status,
		send_room_id,
		mt003->get_message_id(),
		unsend_cids
	);
	try
	{
		//mt104Bをデシリアライズする
		json mt103B_json = MessageType103B::deserialize_message(mt103B);
		if (this->_protocol_handler->send_message_tls(client, mt103B_json)) {
			spdlog::info("The MT103B message was successfully sent to the client(ip:{}, id:{}).", client->get_ipv4_address(), client->get_id());
		}
		else {
			LOG_ERROR("The MT103B message was not sent to the client(ip:{}, id:{}).", client->get_ipv4_address(), client->get_id());
		}
	}
	catch (const std::out_of_range& e)
	{
		// エラーハンドリング
		LOG_ERROR("Sender ID not found. exception content: {}", e.what());
	}
}

std::vector<MessageType103A::ClientInfo> ChatServerFacade::generate_client_infos(const std::vector<std::string>& cids)
{
	std::vector<MessageType103A::ClientInfo> client_infos;
	if (cids.size() == 0) {
		return std::vector<MessageType103A::ClientInfo>();
	}
	for (const auto& cid : cids) {
		std::string client_name, client_mail_addr, client_icon_url;
		std::make_tuple(client_name, client_mail_addr, client_icon_url) = this->_mysql_client->get_client_info_fromDB(cid);
		MessageType103A::ClientInfo add_client_info(cid, client_name, client_mail_addr, client_icon_url);
		client_infos.push_back(add_client_info);
	}
	return client_infos;
}

void ChatServerFacade::mt104_process(const std::unique_ptr<MessageType004>& mt004, const std::shared_ptr<Client>& client)
{
	std::vector<std::string> unsend_cids;
	unsend_cids = this->mt104A_process(mt004);
	this->mt104B_process(mt004, unsend_cids, client);
}



void ChatServerFacade::heartbeat_check_process()
{
	while (true) {
	
		//現在接続しているクライアントリストを抽出
		std::vector<std::shared_ptr<Client>> current_connect_clients;
		current_connect_clients = this->_client_handler->get_connected_clients_snapshot();

		//30秒wait
		std::this_thread::sleep_for(std::chrono::seconds(30));  // 30秒間隔

		for (const auto& client : current_connect_clients) {
			if (client != nullptr && client->get_heart_beat_count() == 0) {
				this->heartbeat_nonresponse_process(client);
			}
		}
		
		//ハートビート回数をクリアする
			this->_client_handler->all_clear_heart_beat_count();
	}
}


void ChatServerFacade::heartbeat_nonresponse_process(const std::shared_ptr<Client>& client)
{

	// クライアントが still connected か確認
	auto clients = _client_handler->get_connected_clients_snapshot();
	//auto it = std::find(clients.begin(), clients.end(), client);
	auto it = std::find_if(clients.begin(), clients.end(),
		[&client](const std::shared_ptr<Client>& c) {
			return c.get() == client.get();  // 所有している生ポインタで比較
		});

	if (it == clients.end() || (*it)->get_heart_beat_count() > 0) {
		return;  // 既に削除されている or ハートビートがリセットされた場合
	}
	spdlog::warn("The Client(ID:{}) did not return heartbeat.", (*it)->get_id());
	this->handle_disconnect_process((*it));

}


void ChatServerFacade::setting_select_process()
{
	while (true) {
		{
			this->_socket_handler->initialize_select_param();
			this->prepare_client_fd_set();
			
		}
	}
}

void ChatServerFacade::handle_disconnect_process(const std::shared_ptr<Client> client)
{
	this->_mysql_client->update_last_login(client->get_id(), this->_server_time->get_current_time_to_milliseconds());
	if(client == nullptr) {
		LOG_ERROR("The client is null.");
		return;
	}
	if (client->get_ssl() != nullptr) {
		_protocol_handler->shutdown_wolfssl(client);
	}
	_socket_handler->close_socket(client->get_socket());
	this->_client_handler->delete_client(client);

}


std::tuple<std::string, std::string> ChatServerFacade::sign_up_process(const std::string& mail_addr, const std::string& password, const std::string& ip, std::string* id, std::string* error_msg)
{
	this->_firebase_auth_handler->create_user_with_email_and_password(mail_addr, password, ip, id, error_msg);
	return std::tuple<std::string, std::string>(*id, *error_msg);
}

std::tuple<std::string, std::string> ChatServerFacade::sign_in_process(
	const std::string& mail_addr,
	const std::string& password,
	const std::string& ip,
	std::string* id,
	std::string* error_msg
)
{
	this->_firebase_auth_handler->sign_in_user_with_email_and_password(mail_addr, password, ip, id, error_msg);
	return std::make_tuple(*id, *error_msg);
}

std::string ChatServerFacade::sign_in_process(int& auth_type, const char* token, const char* secret)
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


std::vector<std::string> ChatServerFacade::mt104A_process(const std::unique_ptr<MessageType004>& mt004)
{
	//mt104Aを生成
	std::string message_type = "104A";
	std::vector<std::string> unsend_cids;

	//MT104Aを生成する
	auto mt104A = std::make_unique<MessageType104A>(
		0,
		message_type,
		mt004->get_room_id(),
		mt004->get_sender_id(),
		mt004->get_message_id(),
		mt004->get_send_time(),
		mt004->get_content()
	);
	this->send_message_to_room<MessageType104A>(mt104A, &unsend_cids);

	
	return unsend_cids;
}

void ChatServerFacade::mt104B_process(
	const std::unique_ptr<MessageType004>& mt004, 
	const std::vector<std::string>& unsend_cids,
	const std::shared_ptr<Client>& client
)
{
	//mt104Bを生成
	std::string message_type = "104B";
	MessageType104B::MessageSendStatus send_status;
	std::string send_room_id = mt004->get_room_id();
	//送り先のclientの情報を取得する
	auto receivers_infos = this->_chat_room_handler->get_client_infos_by_chatroom_id(send_room_id);
	//ChatroomHandlerが保持しているクライアントリストからチャットルームを検出できない場合
	if (receivers_infos.size() == 0) {
		//データベースからチャットルームに参加しているcidのリストを抽出
		auto client_list = this->_mysql_client->get_client_list_from_chatroom(send_room_id);
		if (client_list.size() == 0 && unsend_cids.size() == 0) {
			//LOG_ERROR("The Chatroom ID was not found in the database.");
			send_status = MessageType104B::MessageSendStatus::SEND_FAILED;

		}
		else if (client_list.size() != 1 && client_list.size() - 1 == unsend_cids.size()) {
			//全てのクライアントにメッセージが送信できなかった場合
			send_status = MessageType104B::MessageSendStatus::SEND_FAILED;
		}
		else if (client_list.size() > 0 && unsend_cids.size() == 0) {
			//クライアントにメッセージが送信できた場合
			send_status = MessageType104B::MessageSendStatus::SEND_SUCCESS;
		}
		
		else {		//一部のクライアントにメッセージが送信できなかった場合
			send_status = MessageType104B::MessageSendStatus::SEND_PARTIAL_SUCCESS;
		}

	}
	else {
		if (receivers_infos.size() == 0 && unsend_cids.size() == 0) {
			//全てのクライアントにメッセージが送信できなかった場合
			send_status = MessageType104B::MessageSendStatus::SEND_FAILED;
		}
		else if (receivers_infos.size() - 1 == unsend_cids.size()) {
			//全てのクライアントにメッセージが送信できなかった場合
			send_status = MessageType104B::MessageSendStatus::SEND_FAILED;
		}
		else if (receivers_infos.size() > 0 && unsend_cids.size() == 0) {
			//クライアントにメッセージが送信できた場合
			send_status = MessageType104B::MessageSendStatus::SEND_SUCCESS;
		}
		else {		//一部のクライアントにメッセージが送信できなかった場合
			send_status = MessageType104B::MessageSendStatus::SEND_PARTIAL_SUCCESS;

		}
	}
	auto mt104B = std::make_unique<MessageType104B>(
		0,
		message_type,
		send_status,
		send_room_id,
		mt004->get_message_id(),
		unsend_cids
	);
	try
	{
		//mt104Bをデシリアライズする
		json mt104B_json = MessageType104B::deserialize_message(mt104B);
		if (this->_protocol_handler->send_message_tls(client, mt104B_json)) {
			spdlog::info("The MT104B message was successfully sent to the client(ip:{}, id:{}).", client->get_ipv4_address(), client->get_id());
		}
		else {
			LOG_ERROR("The MT104B message was not sent to the client(ip:{}, id:{}).", client->get_ipv4_address(), client->get_id());
		}
	}
	catch (const std::out_of_range& e)
	{
		// エラーハンドリング
		LOG_ERROR("Sender ID not found. exception content: {}", e.what());
	}

}




void ChatServerFacade::resend_message_process(const json& message, const std::string& cid)
{
	uint64_t mid = message["message_id"];
		// 1秒スリープする
		std::this_thread::sleep_for(std::chrono::seconds(1));
	// クライアントのソケットを取得する
	auto client = this->_client_handler->get_client_by_cid(cid);
	for (int resend_count = 1; resend_count < MAX_RESEND_COUNT; resend_count++) {
		if (resend_count == 2) {
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}
		if (resend_count == 3) {
			std::this_thread::sleep_for(std::chrono::seconds(5));
		}
		if (client->get_socket() != INVALID_SOCKET_FD || client->get_ssl() != nullptr) {
			// メッセージを送信する
			if (this->_protocol_handler->send_message_tls(client, message)) {
				spdlog::info("Resend message(mid: {}) to client with cid: {}", mid, cid);
				return;
			}
		}
	}
	LOG_ERROR("Resend message(mid: {}) to client with cid: {} failed.", mid, cid);
}




