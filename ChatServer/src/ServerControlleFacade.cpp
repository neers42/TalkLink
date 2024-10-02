#include "ServerControlleFacade.h"

std::time_t ServerTime::current_time;
std::tm ServerTime::time_info;

ServerControlleFacade::ServerControlleFacade()
{
	m_firebase_auth_handler = std::make_unique<FirebaseAuthHandler>();
	m_socket_handler = std::make_unique<SocketHandler>();
	m_mt000receiver = std::make_unique<MT000Receiver>();
	m_mt001receiver = std::make_unique<MT001Receiver>();
	m_mt002receiver = std::make_unique<MT002Receiver>();
	m_sender = std::make_unique<Sender>();
	m_server_time = std::make_unique<ServerTime>();
	m_client_handler = std::make_unique<ClientHandler>();
	m_thread_pool_handler = std::make_unique<ThreadPoolHandler>(10);
	m_chat_room_handler = std::make_unique<ChatroomHandler>();

}

ServerControlleFacade::~ServerControlleFacade()
{
	
}

//サーバプログラムの実行
void ServerControlleFacade::run()
{
	//スレッドの実行は以下のように実施する
	this->m_socket_handler->obtain_local_ip_addr();

	//リッスンソケットの生成
	if (!m_socket_handler->open_tcp_listen_socket_for_message()) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("The TCP listening socket for message could not be opened properly, so the program is forcibly terminated. Please set it again."));
		exit(1);
	}

	if (!m_socket_handler->generate_udp_multicast_tx_socket()) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("The program terminates because it failed to create a socket for multicast transmission. Please reset the settings."));
		exit(1);
	}

	this->show_server_running_info();
	LogHandler::logging(LogHandler::LogType::INFO, std::cout, "Start monitoring requests from clients.");
	this->m_thread_pool_handler->enqueue_task([this]() {heart_beat_send_process(); });
	this->m_thread_pool_handler->enqueue_task([this]() {m_thread_pool_handler->monitor_worker(); });
	this->m_thread_pool_handler->enqueue_task([this]() {heart_beat_check_process(); });

	//ディスクリプション監視スタート
	while (1) {
		auto abs_time = std::chrono::system_clock::now() + std::chrono::milliseconds(500);
		this->m_socket_handler->setting_select_info_to_listen_socket_for_msg();
		this->setting_select_info_to_client_msg();
		int detection_num = m_socket_handler->monitor_descriptor_for_message(this->m_client_handler->connected_client_list);

		switch (detection_num)
		{
		case SELECT_TIME_OUT:
			break;

		case SOCKET_ERROR:
			break;

			//アクセプト要求があった場合
		case REQUEST_ACCEPT_CLIENT_DETECTION:
		{
			this->m_thread_pool_handler->enqueue_task([this]() {accept_client_socket_for_msg_process(); });
		};
			break;
		
		//接続中のクライアントリストから通知があった場合
		default:
			this->m_thread_pool_handler->enqueue_task([this, detection_num]() {message_receive_process(detection_num); });
			break;
		}
		
		std::this_thread::sleep_until(abs_time);

	}
	
}

void ServerControlleFacade::setting_select_info_to_client_msg()
{
	{
		std::unique_lock<std::mutex> lock(m_connect_client_list_mutex);
		for (int i = 0; i < this->m_client_handler->connected_client_list.size(); i++) {
			//有効なソケットにFDSETを実行する
			if (this->m_client_handler->connected_client_list[i].get_client_socket() != INVALID_SOCKET) {
				m_socket_handler->bind_select_info_to_socket(this->m_client_handler->connected_client_list[i].get_client_socket(), &m_socket_handler->get_fd_read_msg());
			}
		}
	}

	this->m_socket_handler->copy_fd_read_to_fds(&(m_socket_handler->get_fds_msg()), &(m_socket_handler->get_fd_read_msg()));


}

void ServerControlleFacade::setting_select_info_to_client_hb()
{
	for (int i = 0; i < this->m_client_handler->heart_beat_list.size(); i++) {
		if (std::get<0>(this->m_client_handler->heart_beat_list[i]) != INVALID_SOCKET) {
			m_socket_handler->bind_select_info_to_socket(std::get<0>(this->m_client_handler->heart_beat_list[i]), &m_socket_handler->get_fd_read_hb());
		}
	}
	this->m_socket_handler->copy_fd_read_to_fds(&(m_socket_handler->get_fds_hb()), &(m_socket_handler->get_fd_read_hb()));
}

void ServerControlleFacade::show_server_running_info()
{
	LogHandler::logging(LogHandler::LogType::INFO, std::cout, StringFormat("Server IPv4 address : %s", this->m_socket_handler->get_server_ip_address()));
}


bool ServerControlleFacade::accept_client_socket_for_msg_process()
{
	std::thread::id this_thread_id = std::this_thread::get_id();
	LogHandler::logging(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Function %s() started processing.(tid : \033[32m%d\033[m)", __FUNCTION__, this_thread_id));
	SOCKET new_client_socket = INVALID_SOCKET;
	struct sockaddr_in new_client_sockaddr_in = { 0 };
	socklen_t new_client_socklen_t = 0;
	//uint32_t check_invalid_result = this->m_client_handler.check_next_invalid_socket();

	std::tie(new_client_socket, new_client_sockaddr_in) = m_socket_handler->accept_client_socket_msg(this->m_client_handler->init_client.get_sockaddr_in_length());
	
	if (new_client_socket == INVALID_SOCKET) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("The accepted socket for message could not be added to the list because it was an INVALID SOCKET"));
		return ACCEPT_FAILED;
	}
	ClientHandler::Client new_client = ClientHandler::Client(new_client_socket, new_client_sockaddr_in, new_client_socklen_t);
	new_client.set_client_ipv4_addr(m_socket_handler->extraction_client_ip(new_client_sockaddr_in, sizeof(new_client.get_client_ipv4_addr())));

	{
		std::unique_lock<std::mutex> lock(m_connect_client_list_mutex);
		if (m_client_handler->add_client(new_client) == false) {
			return ACCEPT_FAILED;
		}
	}
	LogHandler::logging(LogHandler::LogType::INFO, std::cout, StringFormat("Registered on the client list[%d] => IP : %s.",this->m_client_handler->connect_client_num - 1, m_client_handler->connected_client_list[this->m_client_handler->connect_client_num -1 ].get_client_ipv4_addr()));
	LogHandler::logging(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Function %s() process complete!.(tid : \033[32m%d\033[m)", __FUNCTION__, this_thread_id));
	return ACCEPT_SUCCEEDED;
}


void ServerControlleFacade::message_receive_process(int detection_num)
{
	
	std::thread::id this_thread_id = std::this_thread::get_id();
	int receive_result = 0;
	std::string receive_data, mt, data_time_stamp;
	{
		std::unique_lock<std::mutex> lock(m_connect_client_list_mutex);
		if (detection_num <= this->m_client_handler->connect_client_num - 1 && m_client_handler->connected_client_list.size() != 0) {
			LogHandler::logging(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Function %s() started processing.(tid : \033[32m%d\033[m)", __FUNCTION__, this_thread_id));
			std::tie(receive_result, receive_data, data_time_stamp) = Receiver::receive_data(m_client_handler->connected_client_list[detection_num].get_client_socket(), detection_num);
		}
		else {
			return;
		}
	}
	
	//receive_result == SOCKET_ERRORの場合(クライアントのソケットがSOCKET_ERRORの場合)
	if (receive_result == SOCKET_ERROR) {
		LogHandler::logging(LogHandler::LogType::INFO, std::cout, StringFormat("Client list[%d](IP address : %s) is SOCKET_ERROR now, so initializing client list.", detection_num, m_client_handler->connected_client_list[detection_num].get_client_ipv4_addr()));
		
		{
			std::unique_lock <std::mutex> lock(this->m_connect_client_list_mutex);
			m_socket_handler->disconnect_socket(m_client_handler->connected_client_list[detection_num].get_client_socket());
			m_client_handler->delete_client_from_con_list(detection_num);
		}
	}
	//receive_result == 0の場合(クライアントのソケットが切断された場合)
	else if (receive_result == DISCONNECT_SOCKET) {
		LogHandler::logging(LogHandler::LogType::INFO, std::cout, StringFormat("Client list[%d](IP address : %s) has been disconnected, so initializing client list.", detection_num, m_client_handler->connected_client_list[detection_num].get_client_ipv4_addr()));
		{
			std::unique_lock <std::mutex> lock(this->m_connect_client_list_mutex);
			m_socket_handler->disconnect_socket(m_client_handler->connected_client_list[detection_num].get_client_socket());
			m_client_handler->delete_client_from_con_list(detection_num);
		}
	}
	//それ以外の場合はMTの判定を実行し、MTごとの処理を実行する。
	else {
		mt = Receiver::extraction_mt(receive_data);
		//mt000 = ハートビートを受信した際の処理
		if (std::equal(mt.begin(), mt.end(), "000")) {
			this->mt000_process(receive_data, detection_num);
		}
		//MT001 = Auth認証メッセージの場合
		else if (std::equal(mt.begin(), mt.end(), "001")) {
			m_mt001receiver->show_receive_info(m_client_handler->connected_client_list[detection_num].get_client_ipv4_addr(), detection_num, receive_result);
			this->mt001_process(receive_data, detection_num);
		}
		//MT002 = 通常メッセージの場合
		else if (std::equal(mt.begin(), mt.end(), "002")) {
			m_mt002receiver->show_receive_info(m_client_handler->connected_client_list[detection_num].get_client_ipv4_addr(), detection_num, receive_result);
			this->mt002_process(receive_data, receive_result);
		}
		//MT003 = の場合
		else if (std::equal(mt.begin(), mt.end(), "003")) {
			
		}
		//MT004 = の場合
		else if (std::equal(mt.begin(), mt.end(), "004")) {

		}

		else {
			LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("The receive process was interrupted due to a bad message"));
		}
	}
	LogHandler::logging(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Function %s() process complete!.(tid : \033[32m%d\033[m)", __FUNCTION__, this_thread_id));
}

void ServerControlleFacade::mt000_process(std::string& receive_data, int detection_num)
{
	std::string src_client_id = "";
	try
	{
		src_client_id = this->m_mt000receiver->data_extraction_(receive_data);
		if(src_client_id.size() != 28){
			THROW_APP_EXCEPTION("The reception process was interrupted because the contents of MT000 were invalid.");
		}

		auto mt000_instance = this->m_mt000receiver->create_mt000_instance(src_client_id);
		{
			std::unique_lock<std::mutex> lock(m_connect_client_list_mutex);
			if (mt000_instance->get_s_client_id() == this->m_client_handler->connected_client_list[detection_num].get_client_id()) {
				this->m_client_handler->incliment_heart_beat_count(detection_num);
			}
		}
		
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
	}
}

void ServerControlleFacade::mt001_process(std::string& receive_data, int& detection_num)
{
	uint8_t sign_type = 0, auth_type = 0, name_byte_size = 0, mail_addr_byte_size = 0, password_byte_size = 0;
	std::string name = "", mail_addr = "", password = "";
	std::string user_id = "";
	
	try
	{
		std::tie(sign_type, auth_type, name_byte_size, name, mail_addr_byte_size, mail_addr, password_byte_size, password)
			= this->m_mt001receiver->data_extraction_(receive_data);

		if (sign_type == 3 || auth_type == 6 || name_byte_size == 0 || mail_addr_byte_size == 0 || password_byte_size == 0) {
			THROW_APP_EXCEPTION("The reception process was interrupted because the contents of MT001 were invalid.");
		}
		//mt001メッセージのインスタンスを作成
		auto mt001_instance = this->m_mt001receiver->create_mt001data_instance(sign_type, auth_type, name_byte_size, name, mail_addr_byte_size, mail_addr, password_byte_size, password);
		switch (mt001_instance->get_sign_type())
		{
		case 0:			//sign up
			//メッセージがsign_upの場合の処理を追記
		{
			std::cout << StringFormat("sign in user email : %s password : %s", mt001_instance->get_client_mail_addr(), mt001_instance->get_client_password()) << std::endl;
			std::unique_lock<std::mutex> lock(this->m_connect_client_list_mutex);
			user_id = this->sign_up_process(mt001_instance->get_client_mail_addr(), mt001_instance->get_client_password());
			if (user_id != "" && detection_num <= this->m_client_handler->connect_client_num - 1 && m_client_handler->connected_client_list.size() != 0) {
				//SIGN UPが成功した場合に、クライアント情報を登録する
				m_client_handler->connected_client_list[detection_num].set_client_id(user_id);
				m_client_handler->connected_client_list[detection_num].set_client_name(mt001_instance->get_client_name());
				m_client_handler->connected_client_list[detection_num].set_client_mail_addr(mt001_instance->get_client_mail_addr());
				m_client_handler->connected_client_list[detection_num].set_client_password(mt001_instance->get_client_password());

				//データベースにユーザ情報を登録する。
				//TODO　データべース処理を追記する。

				//Auth statusを変更する
				this->m_client_handler->change_client_auth_status(detection_num);
			}
			this->m_client_handler->show_client_info(detection_num);
		}
		break;

		case 1:			//sign in
		{
			std::unique_lock<std::mutex> lock(this->m_connect_client_list_mutex);
		
			user_id = this->sign_in_process(mt001_instance->get_client_mail_addr(), mt001_instance->get_client_password());

			if (user_id != "" && detection_num <= this->m_client_handler->connect_client_num - 1 && m_client_handler->connected_client_list.size() != 0) {
				//データベースからユーザ情報を読み取る。
				std::string client_name, client_mail_addr, client_password;
				m_client_handler->connected_client_list[detection_num].set_client_id(user_id);
				m_client_handler->connected_client_list[detection_num].set_client_name(client_name);
				m_client_handler->connected_client_list[detection_num].set_client_mail_addr(client_mail_addr);
				m_client_handler->connected_client_list[detection_num].set_client_password(client_password);

				//Auth statusを変更する
				this->m_client_handler->change_client_auth_status(detection_num);
			}
		}
		break;

		case 2:			//sign out
		{
			std::unique_lock<std::mutex> lock(this->m_connect_client_list_mutex);
			m_firebase_auth_handler->sign_out(m_client_handler->connected_client_list[detection_num].get_client_id(), m_client_handler->connected_client_list[detection_num].get_client_mail_addr());
			break;
		}
		default:
			THROW_APP_EXCEPTION("MT001 authentication data region is in violation.");
			break;
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
	}
}

//void ServerControlleFacade::mt001_process(std::string& receive_data, int& detection_num)
//{
//
//	std::string user_id = "";
//	auto mt001_instance = this->m_mt001receiver->create_mt001data_instance();
//	try
//	{
//
//		if (m_mt001receiver->data_extraction(receive_data)) {
//			switch (m_mt001receiver->get_mt001_data().get_sign_type())
//			{
//			case 0:			//sign up
//				//メッセージがsign_upの場合の処理を追記
//			{
//				std::unique_lock<std::mutex> lock(this->m_connect_client_list_mutex);
//				user_id = this->sign_up_process(m_mt001receiver->get_mt001_data().get_client_mail_addr(), m_mt001receiver->get_mt001_data().get_client_password());
//				if (user_id != "" && detection_num <= this->m_client_handler->connect_client_num - 1 && m_client_handler->connected_client_list.size() != 0) {
//					//SIGN UPが成功した場合に、クライアント情報を登録する
//					m_client_handler->connected_client_list[detection_num].set_client_id(user_id);
//					m_client_handler->connected_client_list[detection_num].set_client_name(m_mt001receiver->get_mt001_data().get_client_name());
//					m_client_handler->connected_client_list[detection_num].set_client_mail_addr(m_mt001receiver->get_mt001_data().get_client_mail_addr());
//					m_client_handler->connected_client_list[detection_num].set_client_password(m_mt001receiver->get_mt001_data().get_client_password());
//
//					//データベースにユーザ情報を登録する。
//					//TODO　データべース処理を追記する。
//
//					//Auth statusを変更する
//					this->m_client_handler->change_client_auth_status(detection_num);
//				}
//				this->m_client_handler->show_client_info(detection_num);
//			}
//			break;
//
//			case 1:			//sign in
//			{
//				std::unique_lock<std::mutex> lock(this->m_connect_client_list_mutex);
//				user_id = this->sign_in_process(m_mt001receiver->get_mt001_data().get_client_mail_addr(), m_mt001receiver->get_mt001_data().get_client_password());
//
//				if (user_id != "" && detection_num <= this->m_client_handler->connect_client_num - 1 && m_client_handler->connected_client_list.size() != 0) {
//					//データベースからユーザ情報を読み取る。
//					std::string client_name, client_mail_addr, client_password;
//					m_client_handler->connected_client_list[detection_num].set_client_id(user_id);
//					m_client_handler->connected_client_list[detection_num].set_client_name(client_name);
//					m_client_handler->connected_client_list[detection_num].set_client_mail_addr(client_mail_addr);
//					m_client_handler->connected_client_list[detection_num].set_client_password(client_password);
//
//					//Auth statusを変更する
//					this->m_client_handler->change_client_auth_status(detection_num);
//				}
//			}
//			break;
//
//			case 2:			//sign out
//			{
//				std::unique_lock<std::mutex> lock(this->m_connect_client_list_mutex);
//				m_firebase_auth_handler->sign_out(m_client_handler->connected_client_list[detection_num].get_client_id(), m_client_handler->connected_client_list[detection_num].get_client_mail_addr());
//				break;
//			}
//			default:
//				THROW_APP_EXCEPTION("MT001 authentication data region is in violation.");
//				break;
//			}
//		}
//		else
//		{
//			THROW_APP_EXCEPTION("MT001 data extraction failed.");
//		}
//	}
//	catch (const exception::ExceptionBase& ex)
//	{
//		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
//		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("The MT001 message data receiving process was suspended."));
//	}
//
//}


void ServerControlleFacade::mt002_process(std::string& receive_data, int& receive_result)
{
	std::vector<SOCKET> send_sockets;
	std::vector<std::string> dst_clients;
	int chatroom_list_num = -1;
	std::string d_client_id = "", s_client_id = "", d_chatroom_id = "", main_content = "";
	int trans_type = -1, data_type = -1;

	try
	{
		std::tie(trans_type, d_client_id, s_client_id, d_chatroom_id, data_type, main_content)
			= this->m_mt002receiver->data_extraction_(receive_data);

		if (trans_type == 3 || d_client_id == "" || s_client_id == "" || d_chatroom_id == "" || data_type == 4 || main_content == "") {
			THROW_APP_EXCEPTION("The reception process was interrupted because the contents of MT002 were invalid.");
		}
		auto mt002_intance = this->m_mt002receiver->create_mt002data_instance(trans_type, d_client_id, s_client_id, d_chatroom_id, data_type, main_content);
		std::unique_lock<std::mutex> lock(this->m_connect_client_list_mutex);
		int send_socket_num = m_client_handler->search_client_id_from_con_list(mt002_intance->get_dst_client_id());
		switch (mt002_intance->get_trans_type())
		{
			//TCPユニキャスト
		case 0:
			m_sender->tcp_unicast_data(m_client_handler->connected_client_list[send_socket_num].get_client_socket(),
				receive_data, receive_result,mt002_intance->get_dst_client_id(), mt002_intance->get_src_client_id());
			break;

			//TCPでマルチキャスト
		case 1:
			//TODO　以下を実装する
			//mt002のパケットの中身がTCPマルチキャストの場合(グループ内でのチャットの時)、グループチャットから
			chatroom_list_num = this->m_chat_room_handler->search_chatroom_id_from_list(mt002_intance->get_dst_chatroom_id());
			for (auto&& client : m_chat_room_handler->get_client_list(chatroom_list_num)) {
				send_sockets.push_back(client.get_client_socket());
			}
			for (auto&& client : m_chat_room_handler->get_client_list(chatroom_list_num)) {
				dst_clients.push_back(client.get_client_id());
			}
			m_sender->tcp_multicast_data(send_sockets, receive_data, receive_result);
			break;

			//UDPブロードキャスト
		case 2:
			//TODO　UDPのブロードキャストを実装する
			break;

		default:
			break;
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
	}
}


/*
void ServerControlleFacade::mt002_process(std::string& receive_data, int& receive_result)
{
	std::string trans_type = Receiver::extraction_trans_type(receive_data);
	std::vector<SOCKET> send_sockets;
	std::vector<std::string> dst_clients;
	int chatroom_list_num = -1;

	try
	{
		if (m_mt002receiver->data_extraction(receive_data)) {
			{
				std::unique_lock<std::mutex> lock(this->m_connect_client_list_mutex);
				int send_socket_num = m_client_handler->search_client_id_from_con_list(this->m_mt002receiver->get_dst_id());
				switch (m_mt002receiver->get_trans_type())
				{
					//TCPユニキャスト
				case 0:
					m_sender->tcp_unicast_data(m_client_handler->connected_client_list[send_socket_num].get_client_socket(),
						receive_data, receive_result, m_mt002receiver->get_dst_id(), m_mt002receiver->get_src_id());
					break;

					//TCPでマルチキャスト
				case 1:
					//TODO　以下を実装する
					//mt002のパケットの中身がTCPマルチキャストの場合(グループ内でのチャットの時)、グループチャットから
					chatroom_list_num = this->m_chat_room_handler->search_chatroom_id_from_list(m_mt002receiver->get_dst_chatroom_id());
					for (auto&& client : m_chat_room_handler->get_client_list(chatroom_list_num)) {
						send_sockets.push_back(client.get_client_socket());
					}
					for (auto&& client : m_chat_room_handler->get_client_list(chatroom_list_num)) {
						dst_clients.push_back(client.get_client_id());
					}
					m_sender->tcp_multicast_data(send_sockets, receive_data, receive_result);
					break;

					//UDPブロードキャスト
				case 2:
					//TODO　UDPのブロードキャストを実装する
					break;

				default:
					THROW_APP_EXCEPTION("The region of the MT002 communication method data is violated.");
					break;
				}
			}
		}
		else {
			THROW_APP_EXCEPTION("MT002 data extraction failed.");
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("The MT002 message data receiving process was suspended."));
	}
}
*/

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
			std::unique_lock<std::mutex> lock(m_connect_client_list_mutex);
			for (int i = 0; i < this->m_client_handler->connected_client_list.size(); i++) {
				send_socket = this->m_client_handler->connected_client_list[i].get_client_socket();
				if (send_socket != INVALID_SOCKET) {
					send_sockets.push_back(send_socket);
				}
			}
			
		}
		
		//②socketリストにマルチキャストをする
		if (send_sockets.size() > 0) {	//接続中のクライアントがいる場合ハートビートメッセージを送る
			fail_send_sockets = this->m_sender->send_heart_beat_message(send_sockets);
			LogHandler::logging(LogHandler::LogType::INFO, std::cout, StringFormat("The server sent a heartbeat message to the client."));
		}
		
		//5秒後に再送処理をする
		std::this_thread::sleep_until(abs_time_5);
		if (fail_send_sockets.size() > 0) {
			LogHandler::logging(LogHandler::LogType::INFO, std::cout, StringFormat("Heartbeat message was resent from server to client"));
			this->m_sender->send_heart_beat_message(fail_send_sockets);
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
	while (true) {
		
		auto time_now = std::chrono::system_clock::now();
		int search_client_num = -1;
		std::vector<ClientHandler::Client> check_con_list;
		{
			std::unique_lock<std::mutex> lock(m_connect_client_list_mutex);
			check_con_list = this->m_client_handler->connected_client_list;
		}
		auto abs_time_30 = time_now + std::chrono::seconds(30);
		std::this_thread::sleep_until(abs_time_30);
		for (int i = 0; i < check_con_list.size(); i++) {
			std::unique_lock<std::mutex> lock(m_connect_client_list_mutex);
			for (int j = 0; j < this->m_client_handler->connected_client_list.size(); j++) {
				if (check_con_list[i].get_client_socket() != INVALID_SOCKET && check_con_list[i] == this->m_client_handler->connected_client_list[j]) {
					if (this->m_client_handler->connected_client_list[j].get_client_heart_beat_count() == 0) {
						if (this->m_client_handler->connected_client_list[j].get_auth_status() == true) {
							LogHandler::logging(LogHandler::LogType::INFO, std::cout, StringFormat("The client (ID: %s) does not respond to the heartbeat message for 30 seconds, so it disconnects.",
							this->m_client_handler->connected_client_list[j].get_client_id()));
							m_socket_handler->disconnect_socket(m_client_handler->connected_client_list[j].get_client_socket());
							m_client_handler->delete_client_from_con_list(j);
						}
						else {
							LogHandler::logging(LogHandler::LogType::INFO, std::cout, StringFormat("The client (SOCKET : %d) does not respond to the heartbeat message for 30 seconds, so it disconnects.",
							this->m_client_handler->connected_client_list[j].get_client_socket()));
							m_socket_handler->disconnect_socket(m_client_handler->connected_client_list[j].get_client_socket());
							m_client_handler->delete_client_from_con_list(j);
						}
						
						break;

					}
					else {
							this->m_client_handler->clear_heart_beat_count(j);

					}

				}
			}
		}
		
	}
}

void ServerControlleFacade::setting_select_process()
{
	while (true) {
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			this->m_socket_handler->setting_select_info_to_listen_socket_for_msg();
			this->setting_select_info_to_client_msg();
			
		}
	}
}



std::string ServerControlleFacade::sign_up_process(const std::string& mail_addr, const std::string& password)
{
	std::string user_id = this->m_firebase_auth_handler->create_user_with_email_and_password(mail_addr, password
		, &FirebaseAuthHandler::on_auth_callback);
	return user_id;
}

std::string ServerControlleFacade::sign_in_process(const std::string& mail_addr, const std::string& password)
{
	std::string user_id = this->m_firebase_auth_handler->sign_in_user_with_email_and_password(mail_addr, password, &FirebaseAuthHandler::on_auth_callback);
	return user_id;
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




