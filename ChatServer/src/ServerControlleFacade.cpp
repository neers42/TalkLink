#include "ServerControlleFacade.h"

std::time_t ServerTime::current_time;
std::tm ServerTime::time_info;

ServerControlleFacade::ServerControlleFacade() : _server_ip{ "" }
{
#ifdef DEBUG_MODE
	LogHandler::get_instance().set_log_level(static_cast<int>(LogHandler::LogLevel::DEBUG));
#else
	LogHandler::get_instance().set_log_level(static_cast<int>(LogHandler::LogLevel::INFO));

#endif // DEBUG_MODE

	_socket_handler = std::make_unique<SocketHandler>();
	this->set_server_ip(this->_socket_handler->obtain_local_ip_addr());
	_firebase_auth_handler = std::make_unique<FirebaseAuthHandler>();
	_db_accesor = std::make_unique<DataBaseAccesor>();
	_protocol_handler = std::make_unique<ProtocolHandler>();
	_mt000receiver = std::make_unique<MT000Receiver>();
	_mt001receiver = std::make_unique<MT001Receiver>();
	_mt002receiver = std::make_unique<MT002Receiver>();
	_sender = std::make_unique<Sender>();
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
		LogHandler::get_instance().logging_critical("The TCP listening socket for message could not be opened properly, so the program is forcibly terminated. Please set it again.");
		exit(1);
	}

	if (!_socket_handler->generate_udp_multicast_tx_socket()) {
		LogHandler::get_instance().logging_critical("The program terminates because it failed to create a socket for multicast transmission. Please reset the settings.");
		exit(1);
	}
	
	this->_db_accesor->connect_db();
	LogHandler::get_instance().logging_info("Start monitoring requests from clients.");
	//this->_thread_pool_handler->enqueue_task([this]() {heart_beat_send_process(); });
	this->_thread_pool_handler->enqueue_task([this]() {_thread_pool_handler->monitor_worker(); });
	this->_thread_pool_handler->enqueue_task([this]() {heart_beat_check_process(); });

	//ディスクリプション監視スタート
	while (1) {
		auto abs_time = std::chrono::system_clock::now() + std::chrono::milliseconds(500);
		this->_socket_handler->setting_select_info_to_listen_socket();
		this->setting_select_info();
		int detection_num = _socket_handler->monitor_socket_descriptor(this->_client_handler->connected_client_list);

		switch (detection_num)
		{
		case SELECT_TIME_OUT:
			break;

		case SOCKET_ERROR:
			break;

			//アクセプト要求があった場合
		case REQUEST_ACCEPT_CLIENT_DETECTION:
		{
			this->_thread_pool_handler->enqueue_task([this]() {accept_client_socket_process(); });
		};
			break;
		
		//接続中のクライアントリストから通知があった場合
		default:
			this->_thread_pool_handler->enqueue_task([this, detection_num]() {message_handle_process(detection_num); });
			break;
		}
		
		std::this_thread::sleep_until(abs_time);

	}
	
}

void ServerControlleFacade::setting_select_info()
{
	{
		std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
		for (int i = 0; i < this->_client_handler->connected_client_list.size(); i++) {
			//有効なソケットにFDSETを実行する
			if (this->_client_handler->connected_client_list[i].get_client_socket() != INVALID_SOCKET) {
				_socket_handler->bind_select_info_to_socket(this->_client_handler->connected_client_list[i].get_client_socket(), &_socket_handler->get_fd_read());
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
	std::tie(new_client_socket, new_client_sockaddr_in) = _socket_handler->accept_client_socket(this->_client_handler->init_client.get_sockaddr_in_length());
	if (new_client_socket == INVALID_SOCKET) {
		LogHandler::get_instance().logging_error(StringFormat("The accepted socket for message could not be added to the list because it was an INVALID SOCKET"));
		return ACCEPT_FAILED;
	}
	else {
		new_client_ipv4_addr = this->_socket_handler->extraction_client_ip(new_client_sockaddr_in, sizeof(new_client_ipv4_addr));
	}

	//SSLアクセプト処理
	WOLFSSL* new_client_ssl = this->_protocol_handler->wolfssl_accept(new_client_socket);
	if (new_client_ssl == nullptr) {
		LogHandler::get_instance().logging_error(StringFormat("failed, client ssl is not accepted(client socket : %d),", new_client_socket));
		wolfSSL_free(new_client_ssl);
		return ACCEPT_FAILED;
	}
	else {
		LogHandler::get_instance().logging_info(StringFormat("success, client ssl is accepted(client socket : %d),", new_client_socket));
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
	LogHandler::get_instance().logging_info(StringFormat("client ip : %s , success, server accept client(socket : %d)."
		, _client_handler->connected_client_list[this->_client_handler->connect_client_num - 1].get_client_ipv4_addr()
		, _client_handler->connected_client_list[this->_client_handler->connect_client_num - 1].get_client_socket()));
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


void ServerControlleFacade::message_handle_process(int detection_num)
{
	std::thread::id this_thread_id = std::this_thread::get_id();
	//int receive_bytes = 0;
	std::string receive_data, mt, data_time_stamp;
	json receive_json;
	bool disconnect_flag = false;


#ifdef TLS_PROTOCOL
	{
		std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
		auto& client = _client_handler->connected_client_list[detection_num];
		if (detection_num <= this->_client_handler->connect_client_num - 1 && _client_handler->connected_client_list.size() != 0) {
			std::tie(receive_json, disconnect_flag) = _protocol_handler->receive_message_tls(client.get_ssl());


		}
		else {
			return;
		}


		if (disconnect_flag == true) {
			LogHandler::get_instance().logging_info(StringFormat("client(ip:%s, client id:%s) is disconnected, server initialize client info."
				, client.get_client_ipv4_addr(), client.get_client_id()));
				this->_protocol_handler->shutdown_wolfssl(client.get_ssl());
				_socket_handler->disconnect_socket(client.get_client_socket());
				_client_handler->delete_client(detection_num);
			return;
		}
	}


	if (receive_json.empty() == true) {
		return;
	}
	

	mt = receive_json["message_type"];
	//BaseMessage::print_received_message_info(receive_json, this->_client_handler->connected_client_list[detection_num].get_client_ipv4_addr());
	//mt000 = ハートビートを受信した際の処理
	if (mt == "000") {
		this->mt000_process(receive_data, detection_num);

	}
	//MT001 = Auth認証メッセージの場合
	else if (mt == "001") {
		auto mt001 = this->_protocol_handler->serialize_message<MessageType001>(receive_json);
		if (mt001 == nullptr) {
			return;
		}
		this->_protocol_handler->print_received_message_info(mt001, "", "", "", _client_handler->connected_client_list[detection_num].get_client_ipv4_addr(), _client_handler->connected_client_list[detection_num].get_client_socket());
		this->mt001_process(mt001, detection_num);

	}
	//MT002 = 通常メッセージの場合
	else if (mt == "002") {
		auto mt002 = this->_protocol_handler->serialize_message<MessageType002>(receive_json);
		if (mt002 == nullptr) {
			return;
		}
		this->_protocol_handler->print_received_message_info(mt002, mt002->get_message_id(), mt002->get_src_client_id(), mt002->get_dst_client_id(),
			_client_handler->connected_client_list[detection_num].get_client_ipv4_addr(), _client_handler->connected_client_list[detection_num].get_client_socket());
		this->mt002_process(mt002, detection_num);
	}

	else {
		LogHandler::get_instance().logging_error("The receive process was interrupted due to a bad message.");
	}


#endif // TLS_PROTOCOL



#ifdef TCP_PROTOCOL
	{
		std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
		auto& client = _client_handler->connected_client_list[detection_num];
		if (detection_num <= this->_client_handler->connect_client_num - 1 && _client_handler->connected_client_list.size() != 0) {
			std::tie(receive_data, disconnect_flag) = _socket_handler->receive_message(client.get_client_socket());
		}
		else {
			return;
		}
		if (disconnect_flag == true) {
			LogHandler::get_instance().logging_info(StringFormat("client(ip:%s, client id:%s) is disconnected, server initialize client info."
				, client.get_client_ipv4_addr(), client.get_client_id()));
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
		LogHandler::get_instance().logging_error("The receive process was interrupted due to a bad message.");
	} 
#endif // TCP_PROTOCOL


}

void ServerControlleFacade::mt000_process(std::string& receive_data, int detection_num)
{
	std::string src_client_id = "";
	std::string* client_id_ptr = new std::string();
	try
	{
		src_client_id = this->_mt000receiver->data_extraction_(receive_data);
		if(src_client_id.size() != 28){
			THROW_APP_EXCEPTION("The reception process was interrupted because the contents of MT000 were invalid.");
		}

		auto mt000_instance = this->_mt000receiver->create_mt000_instance(src_client_id);
		{
			std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
			if (mt000_instance->get_src_client_id() == this->_client_handler->connected_client_list[detection_num].get_client_id()) {
				this->_client_handler->incliment_heart_beat_count(detection_num);
			}
		}
		
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::get_instance().logging_error(ex.what_for_str());
	}
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
			auto& client = this->_client_handler->connected_client_list[detection_num];
			client_id = this->sign_up_process(mt001->get_client_mail_addr(), mt001->get_client_password(), client.get_client_ipv4_addr());
			if (client_id != "" && detection_num <= this->_client_handler->connect_client_num - 1 && _client_handler->connected_client_list.size() != 0) {
				LogHandler::get_instance().logging_info(StringFormat("Client(ip:%s, socket num:%d) successfully signed in, Client id %s", client.get_client_ipv4_addr(), client.get_client_socket(), client_id));
				//SIGN UPが成功した場合に、クライアント情報を登録する
				client.set_client_id(client_id);
				client.set_client_name(mt001->get_client_name());
				client.set_client_mail_addr(mt001->get_client_mail_addr());
				client.set_client_password(mt001->get_client_password());

				//データベースにユーザ情報を登録する。
				//TODO　データべース処理を追記する。

				//Auth statusを変更する
				this->_client_handler->change_client_auth_status(detection_num);
			}
			this->_client_handler->show_client_info(detection_num);
		}
		break;

		case 1:			//sign in
		{
			std::unique_lock<std::mutex> lock(this->_connect_client_list_mutex);
			auto& client = this->_client_handler->connected_client_list[detection_num];
			client_id = this->sign_in_process(mt001->get_client_mail_addr(), mt001->get_client_password(), client.get_client_ipv4_addr(), client_id_ptr);
			
			if (client_id != "" && detection_num <= this->_client_handler->connect_client_num - 1 && _client_handler->connected_client_list.size() != 0) {

				LogHandler::get_instance().logging_info(StringFormat("Client(ip:%s, socket num:%d) successfully signed in, Client id %s",client.get_client_ipv4_addr(), client.get_client_socket(), client_id));
				//データベースからユーザ情報を読み取る。
				std::string client_name, client_mail_addr, client_password;
				client.set_client_id(client_id);
				client.set_client_name(client_name);
				client.set_client_mail_addr(client_mail_addr);
				client.set_client_password(client_password);

				//Auth statusを変更する
				this->_client_handler->change_client_auth_status(detection_num);

				//TODO
				//MT101を送信する処理を記載する
				//
			}
		}
		break;

		case 2:			//sign out
		{
			std::unique_lock<std::mutex> lock(this->_connect_client_list_mutex);
			auto& client = this->_client_handler->connected_client_list[detection_num];
			_firebase_auth_handler->sign_out(client.get_client_id(), client.get_client_mail_addr());
			break;
		}
		default:
			THROW_APP_EXCEPTION("MT001 authentication data region is in violation.");
			break;
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::get_instance().logging_error(ex.what_for_str());
	}
	delete client_id_ptr;

}




void ServerControlleFacade::mt002_process(const std::unique_ptr<MessageType002>& mt002, int& receive_result)
{
	std::vector<SOCKET> send_sockets;
	std::vector<std::string> dst_clients;
	int chatroom_list_num = -1;
	int send_socket_num = 0;

	try
	{
		if (mt002 == nullptr || mt002->is_valid() == false) {
			// メッセージ処理
			THROW_APP_EXCEPTION("The reception process was interrupted because the contents of MT002 were invalid.");
		}
		/*	std::tie(trans_type, d_client_id, s_client_id, d_chatroom_id, data_type, main_content)
				= this->_mt002receiver->data_extraction_(receive_json);

			if (trans_type == 3 || d_client_id.size() != 28 || s_client_id.size() != 28 || d_chatroom_id.size() != 36 || data_type == 4 || main_content.size() == 0) {
				THROW_APP_EXCEPTION("The reception process was interrupted because the contents of MT002 were invalid.");
			}*/
			//	auto mt002_intance = this->_mt002receiver->create_mt002data_instance(trans_type, d_client_id, s_client_id, d_chatroom_id, data_type, main_content);
		{
			std::unique_lock<std::mutex> lock(this->_connect_client_list_mutex);
			send_socket_num = _client_handler->search_client_id_from_con_list(mt002->get_dst_client_id());
			LogHandler::get_instance().logging_info(StringFormat("send_socket_num : %d", send_socket_num));


			////===================ここ実装していない============================================
			//connect_listの中にいない場合は、データベースの中に入れる。
			//①データベースに登録されているidから検索
			//②データベースに登録されているidがある場合は、メッセージを保存し、ない場合はメッセージを破棄する
			if (send_socket_num == -1) {
				THROW_APP_EXCEPTION("The reception process was interrupted because the destination client ID was not found.");
			}

			//=========================================================================================


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
		LogHandler::get_instance().logging_error(ex.what_for_str());
	}


}



void ServerControlleFacade::heart_beat_send_process()
{
	std::vector<SOCKET> send_sockets, fail_send_sockets;
	SOCKET send_socket = INVALID_SOCKET;
	while (true) {
		auto time_now = std::chrono::system_clock::now();
		auto abs_time_10 = time_now + std::chrono::seconds(10);
		auto abs_time_5 = time_now + std::chrono::seconds(5);

		//送信処理を以下に記載する
		//①接続されているsocketを集計する

		{
			std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
			for (int i = 0; i < this->_client_handler->connected_client_list.size(); i++) {
				send_socket = this->_client_handler->connected_client_list[i].get_client_socket();
				if (send_socket != INVALID_SOCKET) {
					send_sockets.push_back(send_socket);
				}
			}
			
		}
		
		//②socketリストにマルチキャストをする
		if (send_sockets.size() > 0) {	//接続中のクライアントがいる場合ハートビートメッセージを送る
			fail_send_sockets = this->_sender->send_heart_beat_message(send_sockets);
			LogHandler::get_instance().logging_info("The server sent a heartbeat message to the client.");
		}
		
		//5秒後に再送処理をする
		std::this_thread::sleep_until(abs_time_5);
		if (fail_send_sockets.size() > 0) {
			LogHandler::get_instance().logging_info("Heartbeat message was resent from server to client");
			this->_sender->send_heart_beat_message(fail_send_sockets);
		}

		send_sockets.clear();
		fail_send_sockets.clear();
		assert(send_sockets.empty());
		assert(fail_send_sockets.empty());
		std::this_thread::sleep_until(abs_time_10);
	}
}


void ServerControlleFacade::heart_beat_check_process()
{

	//while (true) {
	//	std::this_thread::sleep_for(std::chrono::seconds(30));  // 30秒間隔

	//	// ① 現在の接続クライアントリストをコピー
	//	std::vector<ClientHandler::Client> current_clients;
	//	{
	//		std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
	//		current_clients = this->_client_handler->connected_client_list;
	//	}

	//	// ② ハートビート未応答のクライアントをチェック
	//	for (const auto& client : current_clients) {
	//		if (client.get_client_heart_beat_count() == 0) {
	//			this->heart_beat_response_process(client);
	//		}
	//	}

	//	// ③ ハートビートカウントをクリア
	//	{
	//		std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
	//		this->_client_handler->all_clear_heart_beat_count();
	//	}
	//}

	while (true) {
		
		auto time_now = std::chrono::system_clock::now();
		int search_client_num = -1;

		//現在接続しているクライアントリストを抽出
		std::vector<ClientHandler::Client> current_connect_clients;
		{
			std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
			current_connect_clients = this->_client_handler->connected_client_list;
		}

		//30秒wait
		auto abs_time_30 = time_now + std::chrono::seconds(30);
		std::this_thread::sleep_until(abs_time_30);
		
		//ハートビートの回数集計
		for (int then_count = 0; then_count < current_connect_clients.size(); then_count++) {
			std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
			for (int now_count = 0; now_count < this->_client_handler->connected_client_list.size(); now_count++) {
				//接続しているクライアントのハートビート回数が0のときの処理
				if (current_connect_clients[then_count].get_client_socket() != INVALID_SOCKET && current_connect_clients[then_count] == this->_client_handler->connected_client_list[now_count]) {
					this->heart_beat_no_response_process(now_count);
				}
				
			}
		}

		//ハートビート回数をクリアする
		{
			std::unique_lock<std::mutex> lock(_connect_client_list_mutex);
			this->_client_handler->all_clear_heart_beat_count();
		}
				
	}
}

void ServerControlleFacade::heart_beat_no_response_process(const int hb_zero_client_num)
{
	//ハートビートの回数がゼロの場合クライアントを切断する
	if (this->_client_handler->connected_client_list[hb_zero_client_num].get_client_heart_beat_count() == 0) {
		//クライアントが既に認証済みの場合
		if(this->_client_handler->connected_client_list[hb_zero_client_num].get_auth_status() == true) {
			LogHandler::get_instance().logging_info(StringFormat("Client (Socket : %d ID: %s), disconnect.(Not response)",
				this->_client_handler->connected_client_list[hb_zero_client_num].get_client_socket(), this->_client_handler->connected_client_list[hb_zero_client_num].get_client_id()));
		}
		else {
			LogHandler::get_instance().logging_info(StringFormat("Client (Socket : %d ID: None), disconnect.(Not response)",
				this->_client_handler->connected_client_list[hb_zero_client_num].get_client_socket()));
		}
		//クライアント切断処理
		_socket_handler->disconnect_socket(_client_handler->connected_client_list[hb_zero_client_num].get_client_socket());
		_client_handler->delete_client(hb_zero_client_num);
	}	
}

void ServerControlleFacade::heart_beat_response_process(const ClientHandler::Client& client)
{
	std::unique_lock<std::mutex> lock(_connect_client_list_mutex);

	// クライアントが still connected か確認
	auto& clients = _client_handler->connected_client_list;
	auto it = std::find(clients.begin(), clients.end(), client);

	if (it == clients.end() || it->get_client_heart_beat_count() > 0) {
		return;  // 既に削除されている or ハートビートがリセットされた場合
	}

	// ログ出力
	if (it->get_auth_status()) {
		LogHandler::get_instance().logging_info(StringFormat(
			"Client (Socket : %d ID: %s), disconnect.(Not response)",
			it->get_client_socket(), it->get_client_id()));
	}
	else {
		LogHandler::get_instance().logging_info(StringFormat(
			"Client (Socket : %d ID: None), disconnect.(Not response)",
			it->get_client_socket()));
	}

	// クライアント切断処理
	_socket_handler->disconnect_socket(it->get_client_socket());

	// クライアント削除（イテレータを使って安全に削除）
	clients.erase(it);
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
	LogHandler::get_instance().logging_debug(StringFormat("user_id : %s", *id));
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




