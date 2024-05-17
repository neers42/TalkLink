#include "ServerControlleFacade.h"

std::time_t ServerTime::current_time;
std::tm ServerTime::time_info;

ServerControlleFacade::ServerControlleFacade()
{

}

ServerControlleFacade::~ServerControlleFacade()
{
	delete m_firebase_auth_handler;
	delete m_socket_handler;
	delete m_mt001receiver;
	delete m_mt002receiver;
	delete m_sender;
	delete m_server_time;
	delete m_thread_pool_handler;
	delete m_chat_room_handler;
	
}

//サーバプログラムの実行
void ServerControlleFacade::run()
{
	//スレッドの実行は以下のように実施する
	this->m_socket_handler->obtain_local_ip_addr();

	//リッスンソケットの生成
	if (!m_socket_handler->open_tcp_listen_socket_for_message()) {
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("The TCP listening socket for message could not be opened properly, so the program is forcibly terminated. Please set it again."));
		exit(1);
	}

	if (!m_socket_handler->open_tcp_listen_socket_for_heart_beat()) {
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("The TCP listening socket for heart beat could not be opened properly, so the program is forcibly terminated. Please set it again."));
		exit(1);
	}
	
	this->show_server_running_info();
	LogHandler::logging_info(LogHandler::LogType::INFO, std::cout, "Start monitoring requests from clients.");
	this->m_thread_pool_handler->enqueue_task([this]() {heart_beat_process(); });


	//this->m_socket_handler->setting_select_info_to_listen_socket();
	//ディスクリプション監視スタート
	while (1) {
		this->m_socket_handler->setting_select_info_to_listen_socket_for_msg();
		this->setting_select_info_to_client_msg();
		int detection_num = m_socket_handler->monitor_descriptor_for_message(m_client_handler);
		switch (detection_num)
		{
		case SELECT_TIME_OUT:
			break;

		case SOCKET_ERROR:
			break;

			//アクセプト要求があった場合
		case REQUEST_ACCEPT_CLIENT_DETECTION:
		{
			this->m_thread_pool_handler->enqueue_task([this]() {accept_client_socket_process(); });
		};
			break;
		
		//接続中のクライアントリストから通知があった場合
		default:
			this->m_thread_pool_handler->enqueue_task([this, detection_num]() {message_receive_process(detection_num); });
			break;
		}

	}
	
}

void ServerControlleFacade::setting_select_info_to_client_msg()
{
	for (int i = 0; i < MAX_CONNECT_NUM; i++) {
		//有効なソケットにFDSETを実行する
		if (this->m_client_handler.connected_client_list[i].get_client_socket() != INVALID_SOCKET) {
			m_socket_handler->bind_select_info_to_socket(this->m_client_handler.connected_client_list[i].get_client_socket(), &m_socket_handler->get_fd_read_msg());
			}
	}

	this->m_socket_handler->copy_fd_read_to_fds(&(m_socket_handler->get_fds_msg()), &(m_socket_handler->get_fd_read_msg()));
	//this->m_socket_handler->copy_fd_read_to_fds();
}

void ServerControlleFacade::setting_select_info_to_client_hb()
{
	for (int i = 0; i < this->m_client_handler.heart_beat_list.size(); i++) {
		if (std::get<0>(this->m_client_handler.heart_beat_list[i]) != INVALID_SOCKET) {
			m_socket_handler->bind_select_info_to_socket(std::get<0>(this->m_client_handler.heart_beat_list[i]), &m_socket_handler->get_fd_read_hb());
		}
	}
	this->m_socket_handler->copy_fd_read_to_fds(&(m_socket_handler->get_fds_hb()), &(m_socket_handler->get_fd_read_hb()));
}

void ServerControlleFacade::show_server_running_info()
{
	LogHandler::logging_info(LogHandler::LogType::INFO, std::cout, StringFormat("Server IPv4 address : %s", this->m_socket_handler->get_server_ip_address()));
}


void ServerControlleFacade::accept_client_socket_process()
{
	std::thread::id this_thread_id = std::this_thread::get_id();
	LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Function %s() started processing.(tid : \033[32m%d\033[m)", __FUNCTION__, this_thread_id));
	SOCKET new_client_socket = INVALID_SOCKET;
	struct sockaddr_in new_client_sockaddr_in = { 0 };
	socklen_t new_client_socklen_t = 0;
	uint32_t check_invalid_result = this->m_client_handler.check_next_invalid_socket();

	std::tie(new_client_socket, new_client_sockaddr_in) = m_socket_handler->accept_client_socket_msg(check_invalid_result, this->m_client_handler.connected_client_list[check_invalid_result].get_sockaddr_in_length());

	ClientHandler::Client new_client = ClientHandler::Client(new_client_socket, new_client_sockaddr_in, new_client_socklen_t);
	new_client.set_client_ipv4_addr(m_socket_handler->extraction_client_ip(new_client_sockaddr_in, sizeof(new_client.get_client_ipv4_addr())));
	m_client_handler.add_client(new_client, check_invalid_result);
	
	
	LogHandler::logging_info(LogHandler::LogType::INFO, std::cout, StringFormat("Registered on the client list[%d] => IP : %s.",check_invalid_result, m_client_handler.connected_client_list[check_invalid_result].get_client_ipv4_addr()));
	LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Function %s() process complete!.(tid : \033[32m%d\033[m)", __FUNCTION__, this_thread_id));
}

void ServerControlleFacade::message_receive_process(int detection_num)
{
	std::thread::id this_thread_id = std::this_thread::get_id();
	LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Function %s() started processing.(tid : \033[32m%d\033[m)", __FUNCTION__, this_thread_id));
	int receive_result = 0;
	std::string receive_data, mt;
	{
		std::unique_lock<std::mutex> lock(m_connect_client_list_mutex);
		std::tie(receive_result, receive_data) = Receiver::receive_data(m_client_handler.connected_client_list[detection_num].get_client_socket()
			, detection_num, m_client_handler.connected_client_list[detection_num].get_client_ipv4_addr());
	}
	
	//receive_result == SOCKET_ERRORの場合(クライアントのソケットがSOCKET_ERRORの場合)
	if (receive_result == SOCKET_ERROR) {
		LogHandler::logging_info(LogHandler::LogType::INFO, std::cout, StringFormat("Client list[%d](IP address : %s) is SOCKET_ERROR now, so initializing client list.", detection_num, m_client_handler.connected_client_list[detection_num].get_client_ipv4_addr()));
		
		{
			std::unique_lock <std::mutex> lock(this->m_connect_client_list_mutex);
			m_socket_handler->disconnect_socket(m_client_handler.connected_client_list[detection_num].get_client_socket(), m_client_handler.connected_client_list[detection_num].get_client_ipv4_addr(), detection_num);
			m_client_handler.delete_client(detection_num);
		}
	}
	//receive_result == 0の場合(クライアントのソケットが切断された場合)
	else if (receive_result == DISCONNECT_SOCKET) {
		LogHandler::logging_info(LogHandler::LogType::INFO, std::cout, StringFormat("Client list[%d](IP address : %s) has been disconnected, so initializing client list.", detection_num, m_client_handler.connected_client_list[detection_num].get_client_ipv4_addr()));
		{
			std::unique_lock <std::mutex> lock(this->m_connect_client_list_mutex);
			m_socket_handler->disconnect_socket(m_client_handler.connected_client_list[detection_num].get_client_socket(), m_client_handler.connected_client_list[detection_num].get_client_ipv4_addr(), detection_num);
			m_client_handler.delete_client(detection_num);
		}
	}
	//それ以外の場合はMTの判定を実行し、MTごとの処理を実行する。
	else {
		mt = Receiver::extraction_mt(receive_data);

		//MTごとに処理を変更する
		if (std::equal(mt.begin(), mt.end(), "000")) {
			this->heart_beat_process();
		}
		//MT001の場合
		if (std::equal(mt.begin(), mt.end(), "001")) {
			m_mt001receiver->show_receive_info(m_client_handler.connected_client_list[detection_num].get_client_ipv4_addr(), detection_num, receive_result);
			this->mt001_process(receive_data, detection_num);
		}
		//MT002の場合
		else if (std::equal(mt.begin(), mt.end(), "002")) {
			m_mt002receiver->show_receive_info(m_client_handler.connected_client_list[detection_num].get_client_ipv4_addr(), detection_num, receive_result);
			this->mt002_process(receive_data, receive_result);
		}
		else {
			LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("The receive process was interrupted due to a bad message"));
		}
	}
	LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Function %s() process complete!.(tid : \033[32m%d\033[m)", __FUNCTION__, this_thread_id));
}

void ServerControlleFacade::mt001_process(std::string& receive_data, int& monitor_result)
{
	
	std::string user_id = "";
	try
	{
		if (m_mt001receiver->data_extraction(receive_data)) {
			switch (m_mt001receiver->get_mt001_data().get_sign_type())
			{
			case 0:			//sign up
				//メッセージがsign_upの場合の処理を追記
			{
				std::unique_lock<std::mutex> lock(this->m_connect_client_list_mutex);
				user_id = this->sign_up_process(m_mt001receiver->get_mt001_data().get_client_mail_addr(), m_mt001receiver->get_mt001_data().get_client_password());
				if (user_id != "") {
					//SIGN UPが成功した場合に、クライアント情報を登録する
					m_client_handler.connected_client_list[monitor_result].set_client_id(user_id);
					m_client_handler.connected_client_list[monitor_result].set_client_name(m_mt001receiver->get_mt001_data().get_client_name());
					m_client_handler.connected_client_list[monitor_result].set_client_mail_addr(m_mt001receiver->get_mt001_data().get_client_mail_addr());
					m_client_handler.connected_client_list[monitor_result].set_client_password(m_mt001receiver->get_mt001_data().get_client_password());

					//データベースにユーザ情報を登録する。
					//TODO　データべース処理を追記する。
				}
				this->m_client_handler.show_client_info(monitor_result);
			}
			break;

			case 1:			//sign in
			{
				std::unique_lock<std::mutex> lock(this->m_connect_client_list_mutex);
				user_id = this->sign_in_process(m_mt001receiver->get_mt001_data().get_client_mail_addr(), m_mt001receiver->get_mt001_data().get_client_password());

				if (user_id != "") {
					//データベースからユーザ情報を読み取る。
					std::string client_name, client_mail_addr, client_password;
					m_client_handler.connected_client_list[monitor_result].set_client_id(user_id);
					m_client_handler.connected_client_list[monitor_result].set_client_name(client_name);
					m_client_handler.connected_client_list[monitor_result].set_client_mail_addr(client_mail_addr);
					m_client_handler.connected_client_list[monitor_result].set_client_password(client_password);
				}
			}
			break;

			case 2:			//sign out
			{
				std::unique_lock<std::mutex> lock(this->m_connect_client_list_mutex);
				m_firebase_auth_handler->sign_out(m_client_handler.connected_client_list[monitor_result].get_client_id(), m_client_handler.connected_client_list[monitor_result].get_client_mail_addr());
				break;
			}
			default:
				THROW_APP_EXCEPTION("MT001 authentication data region is in violation.");
				break;
			}
		}
		else
		{
			THROW_APP_EXCEPTION("MT001 data extraction failed.");
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("The MT001 message data receiving process was suspended."));
	}

}

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
				int send_socket_num = m_client_handler.search_connect_client_socket_num_from_id(this->m_mt002receiver->get_dst_id());
				switch (m_mt002receiver->get_trans_type())
				{
					//TCPユニキャスト
				case 0:
					m_sender->tcp_unicast_data(m_client_handler.connected_client_list[send_socket_num].get_client_socket(),
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

void ServerControlleFacade::heart_beat_process()
{
	//TODO
	//接続中のクライアントに10秒に1回heart beat messageを送る
	//heartbeat送信処理
	int i = 0;
	while (true) {
		auto abs_time = std::chrono::system_clock::now() + std::chrono::seconds(10);
		LogHandler::logging_info(LogHandler::LogType::INFO, std::cout, StringFormat("count : %d", i));
		i++;
		std::this_thread::sleep_until(abs_time);
	}

	//クライアント～のheartbeat受信処理
	//スレッドを分ける
	//30秒に

}

void ServerControlleFacade::heart_beat_send_process()
{
	while (true) {
		auto abs_time = std::chrono::system_clock::now() + std::chrono::seconds(10);
		//送信処理を以下に記載する
		//①接続されているsocketを集計する
		std::vector<SOCKET> con_sockets = this->m_client_handler.extract_conneted_sockets();
		//②socketリストにマルチキャストをする
		if (con_sockets.size() > 0) {	//接続中のクライアントがいる場合ハートビートメッセージを送る
			this->m_sender->send_heart_beat_message(con_sockets);
			
		}
		std::this_thread::sleep_until(abs_time);

	}
}

void ServerControlleFacade::heart_beat_receive_process()
{
	int detection_num_hb = -1;
	while (true) {

		auto abs_time = std::chrono::system_clock::now() + std::chrono::seconds(30);
		//受信処理を以下に記載する
		//①30秒カウント開始する
		//②クライアントから返信があったリストの該当箇所のステータスをconnectにする
		//③クライアントから返信がない場合はステータスをdisconnectにする

		std::this_thread::sleep_until(abs_time);

		this->m_socket_handler->setting_select_info_to_listen_socket_for_hb();
		this->setting_select_info_to_client_msg();
		this->setting_select_info_to_client_hb();
		int detection_num = m_socket_handler->monitor_descriptor_for_message(m_client_handler);
		switch (detection_num)
		{
		case SELECT_TIME_OUT:
			break;

		case SOCKET_ERROR:
			break;

			//アクセプト要求があった場合
		case REQUEST_ACCEPT_CLIENT_DETECTION:
		{
			this->m_thread_pool_handler->enqueue_task([this]() {accept_client_socket_process(); });
		};
		break;

		//接続中のクライアントリストから通知があった場合
		default:
			this->m_thread_pool_handler->enqueue_task([this, detection_num]() {message_receive_process(detection_num); });
			break;
		}
	}
}



std::string ServerControlleFacade::sign_up_process(const std::string& mail_addr, const std::string& password)
{
	std::string user_id = this->m_firebase_auth_handler->create_user_with_email_and_password(this->m_mt001receiver->get_mt001_data().get_client_mail_addr(), this->m_mt001receiver->get_mt001_data().get_client_password()
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




