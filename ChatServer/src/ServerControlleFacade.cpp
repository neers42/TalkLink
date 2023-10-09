#include "ServerControlleFacade.h"

std::time_t ServerTime::current_time;
std::tm ServerTime::time_info;

ServerControlleFacade::ServerControlleFacade()
{

}

ServerControlleFacade::~ServerControlleFacade()
{
	delete m_receiver;
	delete m_socket_handler;
	delete m_server_time;
	
}

//サーバプログラムの実行
void ServerControlleFacade::run()
{
	std::cout << ServerTime::get_current_time() <<"[INFO]サーバのプログラムを起動します。" << std::endl;
	//リッスンソケットの生成
	m_socket_handler->generate_listen_socket();
	m_socket_handler->active_listen_socket();
	int receive_result = 0;
	std::cout << ServerTime::get_current_time() << "[INFO]クライアントからの要求の監視を開始します。" << std::endl;

	//ディスクリプション監視スタート
	while (1) {
		int monitor_result = m_socket_handler->monitor_socket();
		switch (monitor_result)
		{
		case SELECT_TIME_OUT:
			//std::cout << "SELECT_TIME_OUT" << std::endl;
			break;

		case SOCKET_ERROR:
			break;

		//アクセプト要求があった場合
		case REQUEST_ACCEPT_CLIENT_DETECTION:
			m_socket_handler->accept_client_socket();
			break;
		
		//接続中のクライアントリストから通知があった場合
		default:
			receive_result = m_receiver->data_receive(m_socket_handler->get_client_socket_info_list(),monitor_result);
			if (receive_result == 0) {
				m_socket_handler->disconnect_socket(monitor_result);
			}
			//ソケットエラーの場合
			else if (receive_result == SOCKET_ERROR) {
				m_socket_handler->disconnect_socket(monitor_result);
			}
			//それ以外の場合は何もしない
			else {
				;
			}
			break;
		}

	}
	
}
