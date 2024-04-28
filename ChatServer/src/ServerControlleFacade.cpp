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
	
	
}

//サーバプログラムの実行
void ServerControlleFacade::run()
{
	//スレッドの実行は以下のように実施する
    this->m_thread_pool_handler->enqueue_task([this]() {m_thread_pool_handler->exampleTask(); });
	this->m_socket_handler->obtain_local_ip_addr();

	//リッスンソケットの生成
	if (m_socket_handler->open_tcp_listen_socket() == true) {
		;
	}
	else {
		std::cerr << StringFormat("%s\033[31m[ERROR]\033[mThe TCP listening socket could not be opened properly, so the program is forcibly terminated. Please set it again.", ServerTime::get_current_time());
		return;
	}
	
	this->show_server_running_info();
	
	std::cout << ServerTime::get_current_time() << "[INFO]Start monitoring requests from clients." << std::endl;

	//ディスクリプション監視スタート
	while (1) {
		//int monitor_result = m_socket_handler->monitor_socket();
		this->m_socket_handler->setting_select_info_to_listen_socket();
		this->client_select_setting();
		int monitor_result = m_socket_handler->monitor_socket(m_client_handler);
		switch (monitor_result)
		{
		case SELECT_TIME_OUT:
			break;

		case SOCKET_ERROR:
			break;

			//アクセプト要求があった場合
		case REQUEST_ACCEPT_CLIENT_DETECTION:
		{
			this->accept_client_socket_process();
		};
			break;
		
		//接続中のクライアントリストから通知があった場合
		default:
			int receive_result = 0;
			std::string receive_data, mt;
			std::tie(receive_result, receive_data) = Receiver::receive_data(m_client_handler, monitor_result);

			//receive_result == 0の場合(クライアントのソケットが切断された場合)
			if (receive_result == DISCONNECT_SOCKET) {

				std::cout << ServerTime::get_current_time() << "[INFO]Client list[" << monitor_result << "](IP address: " << m_client_handler.connected_client_list[monitor_result].get_client_ipv4_addr() << ") has been disconnected, so initializing client list." << std::endl;
				m_socket_handler->disconnect_socket(m_client_handler.connected_client_list[monitor_result].get_client_socket(), m_client_handler.connected_client_list[monitor_result].get_client_ipv4_addr(), monitor_result);
				m_client_handler.delete_client(monitor_result);
				break;
			}
			//receive_result == SOCKET_ERRORの場合(クライアントのソケットがSOCKET_ERRORの場合)
			else if (receive_result == SOCKET_ERROR) {
				std::cout << ServerTime::get_current_time() << "[INFO]Client list[" << monitor_result << "](IP address: " << m_client_handler.connected_client_list[monitor_result].get_client_ipv4_addr() << ") is SOCKET_ERROR now, so initializing client list." << std::endl;
				m_socket_handler->disconnect_socket(m_client_handler.connected_client_list[monitor_result].get_client_socket(), m_client_handler.connected_client_list[monitor_result].get_client_ipv4_addr(), monitor_result);
				m_client_handler.delete_client(monitor_result);

			}
			//それ以外の場合はMTの判定を実行し、MTごとの処理を実行する。
			else {
				mt = Receiver::extraction_mt(receive_data);
				
				//MTごとに処理を変更する
				//MT001の場合
				if (std::equal(mt.begin(), mt.end(), "001")) {
					this->mt001_process_task(receive_data, monitor_result);
				}
				//MT002の場合
				else if (std::equal(mt.begin(), mt.end(), "002")) {
					this->mt002_process_task(receive_data, receive_result);
				}
				break;
			}
		}

	}
	
}

void ServerControlleFacade::client_select_setting()
{
	for (int i = 0; i < MAX_CONNECT_NUM; i++) {
		//有効なソケットにFDSETを実行する
		if (this->m_client_handler.connected_client_list[i].get_client_socket() != INVALID_SOCKET) {
			m_socket_handler->setting_select_info_to_client_socket(this->m_client_handler.connected_client_list[i].get_client_socket(), &m_socket_handler->get_fd_read());
			}
	}

	this->m_socket_handler->copy_fd_read_to_fds();
}

void ServerControlleFacade::show_server_running_info()
{
	std::cout << StringFormat("%s[SEVER_INFO]Server IPv4 address : %s", ServerTime::get_current_time(), this->m_socket_handler->get_server_ip_address()) << std::endl;
}

void ServerControlleFacade::accept_client_socket_process()
{
	SOCKET new_client_socket = INVALID_SOCKET;
	struct sockaddr_in new_client_sockaddr_in = { 0 };
	socklen_t new_client_socklen_t = 0;
	uint32_t check_invalid_result = this->m_client_handler.check_next_invalid_socket();

	std::tie(new_client_socket, new_client_sockaddr_in) = m_socket_handler->accept_client_socket(check_invalid_result, this->m_client_handler.connected_client_list[check_invalid_result].get_sockaddr_in_length());

	ClientHandler::Client new_client = ClientHandler::Client(new_client_socket, new_client_sockaddr_in, new_client_socklen_t);
	new_client.set_client_ipv4_addr(m_socket_handler->extraction_client_ip(new_client_sockaddr_in, sizeof(new_client.get_client_ipv4_addr())));
	m_client_handler.add_client(new_client, check_invalid_result);
	std::cout << ServerTime::get_current_time() << "[INFO]Registered on the client list[" << check_invalid_result
		<< "] => IP:" << m_client_handler.get_connected_client_list()[check_invalid_result].get_client_ipv4_addr() << "." << std::endl;
}

void ServerControlleFacade::mt001_process_task(std::string& receive_data, int& monitor_result)
{
	std::string user_id = "";
	if (m_mt001receiver->data_extraction(receive_data)) {
		switch (m_mt001receiver->get_mt001_data().get_sign_type())
		{
		case 0:			//sign up
			//メッセージがsign_upの場合の処理を追記
#ifdef DEBUG
			user_id = this->m_firebase_auth_handler->create_user_with_email_and_password(this->m_mt001receiver->get_mt001_data().get_client_mail_addr(), this->m_mt001receiver->get_mt001_data().get_client_password());
			if (user_id != "") {
				//SIGN UPが成功した場合に、クライアント情報を登録する
				m_client_handler.connected_client_list[monitor_result].set_client_id(user_id);
				m_client_handler.connected_client_list[monitor_result].set_client_name(m_mt001receiver->get_mt001_data().get_client_name());
				m_client_handler.connected_client_list[monitor_result].set_client_mail_addr(m_mt001receiver->get_mt001_data().get_client_mail_addr());
				m_client_handler.connected_client_list[monitor_result].set_client_password(m_mt001receiver->get_mt001_data().get_client_password());
				
				//データベースにユーザ情報を登録する。
			
			}
			this->m_client_handler.show_client_info(monitor_result);

#endif // DEBUG

			break;

		case 1:			//sign in
			user_id = this->m_firebase_auth_handler->sign_in_user_with_email_and_password(this->m_mt001receiver->get_mt001_data().get_client_mail_addr()
				, this->m_mt001receiver->get_mt001_data().get_client_password());

			if (user_id != "") {
				//データベースからユーザ情報を読み取る。
				std::string client_name, client_mail_addr, client_password;
				m_client_handler.connected_client_list[monitor_result].set_client_id(user_id);
				m_client_handler.connected_client_list[monitor_result].set_client_name(client_name);
				m_client_handler.connected_client_list[monitor_result].set_client_mail_addr(client_mail_addr);
				m_client_handler.connected_client_list[monitor_result].set_client_password(client_password);
			}
			
			break;

		case 2:			//sign out
			
			break;

		default:
			break;
		}
	}
	else
	{
		std::cerr << StringFormat("%s\033[31m[ERORR]\033[mMT001 received processing was not completed normally.");
		//break;
	}

}

void ServerControlleFacade::mt002_process_task(std::string& receive_data, int& receive_result)
{
	std::string trans_type = Receiver::extraction_trans_type(receive_data);
	if (m_mt002receiver->data_extraction(receive_data)) {
		int send_socket_num = m_client_handler.search_connect_client_socket_num_from_id(this->m_mt002receiver->get_dst_id());
		switch (m_mt002receiver->get_trans_type())
		{
			//TCPユニキャスト
		case 0:
			m_sender->tcp_unicast_data(m_client_handler.connected_client_list[send_socket_num].get_client_socket(),
				receive_data, receive_result);
			break;

			//TCPでマルチキャスト
		case 1:

			break;

			//UDPブロードキャスト
		case 2:
			break;

		default:
			std::cerr << StringFormat("%s\033[31m[ERORR]\033[mThe communication type is different.") << std::endl;
			break;
		}
	}
	else {
		std::cerr << StringFormat("%s[INFO]The message receiving process was suspended.", ServerTime::get_current_time()) << std::endl;
		//break;
	}
}


std::string ServerControlleFacade::sign_in_process(std::string& mail_addr, std::string& password)
{
	return std::string();
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




