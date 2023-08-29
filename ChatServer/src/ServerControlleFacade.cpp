#include "ServerControlleFacade.h"

ServerControlleFacade::ServerControlleFacade()
{

}

ServerControlleFacade::~ServerControlleFacade()
{
	delete m_socket_handler;
}

//サーバプログラムの実行
void ServerControlleFacade::run()
{
	//リッスンソケットの生成
	m_socket_handler->generate_listen_socket();
	m_socket_handler->active_listen_socket();
	std::cout << "[INFO]クライアントからの要求の監視を開始します。" << std::endl;

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

			break;
		}

	}
	
}
