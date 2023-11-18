#include "ServerControlleFacade.h"

std::time_t ServerTime::current_time;
std::tm ServerTime::time_info;

ServerControlleFacade::ServerControlleFacade()
{

}

ServerControlleFacade::~ServerControlleFacade()
{

	delete m_socket_handler;
	delete m_001receiver;
	delete m_sender;
	delete m_server_time;
	
	
}

//サーバプログラムの実行
void ServerControlleFacade::run()
{
	std::cout << ServerTime::get_current_time() <<"[INFO]Starting MyChat server version 1.0.0!!" << std::endl;

	
	//リッスンソケットの生成
	if (m_socket_handler->open_tcp_listen_socket() == true) {
		;
	}
	else {
		std::cerr << StringFormat("%s\033[31m[ERROR]\033[mThe TCP listening socket could not be opened properly, so the program is forcibly terminated. Please set it again.", ServerTime::get_current_time());
		return;
	}

	
	int receive_result = 0;
	std::string receive_data = "";
	std::string mt = "";
	std::string trans_type = "";
	std::cout << ServerTime::get_current_time() << "[INFO]Start monitoring requests from clients." << std::endl;

	//ディスクリプション監視スタート
	while (1) {
		//int monitor_result = m_socket_handler->monitor_socket();
		int monitor_result = m_socket_handler->monitor_socket(m_client_handler);
		switch (monitor_result)
		{
		case SELECT_TIME_OUT:
			//std::cout << "SELECT_TIME_OUT" << std::endl;
			break;

		case SOCKET_ERROR:
			break;

		//アクセプト要求があった場合
		case REQUEST_ACCEPT_CLIENT_DETECTION:
			//m_socket_handler->accept_client_socket();
			m_socket_handler->accept_client_socket(m_client_handler);
			break;
		
		//接続中のクライアントリストから通知があった場合
		default:
			//receive_result = m_receiver->data_receive(m_socket_handler->get_client_socket_info_list(), monitor_result);
			
			std::tie(receive_result, receive_data) = Receiver::receive_data(m_client_handler, monitor_result);
			

			//receive_result = m_001receiver->data_receive(m_client_handler ,monitor_result);
				
			if (receive_result == 0) {
				m_socket_handler->disconnect_socket(m_client_handler, monitor_result);
			}
			//ソケットエラーの場合
			else if (receive_result == SOCKET_ERROR) {
				m_socket_handler->disconnect_socket(m_client_handler, monitor_result);
			}
			//それ以外の場合はMTの判定を実行し、MTごとの処理を実行する。
			else {
				mt = Receiver::extraction_mt(receive_data);
				trans_type = Receiver::extraction_trans_type(receive_data);

				//TODO⇒メッセージをsendするときに
				if (std::equal(mt.begin(), mt.end(), "001")) {
					if (m_001receiver->data_extraction(receive_data)) {
						switch (m_001receiver->get_trans_type())
						{
						//TCPユニキャスト
						case 0:
							m_sender->tcp_unicast_data(m_client_handler, 
												receive_data, receive_result,
												m_socket_handler->search_connect_client_socket_num_from_ip(m_client_handler,
													this->m_001receiver->get_dst_ip_address()));

							break;

						//TCPでマルチキャスト
						case 1:
							break;

						//UDPブロードキャスト
						case 2:
							break;

						default:
							std::cerr << StringFormat("%s\033[31m[ERROR]\033[mThe communication type is different.") << std::endl;
							break;
						}
											}
					else {
						break;
					}
					
				}
				else if (std::equal(mt.begin(), mt.end(), "002")) {

				}
				break;
			}
		}

	}
	
}
