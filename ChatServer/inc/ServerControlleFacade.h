#ifndef SERVERCONTROLLFACADE_H_
#define SERVERCONTROLLFACADE_H_
#include "SocketHandler.h"
#include "ClientInfomationListHandler.h"


class ServerControlleFacade
{
private:
	SocketHandler* m_socket_handler = new SocketHandler();


public:
	//コンストラクタ
	ServerControlleFacade();

	//デストラクタ
	~ServerControlleFacade();


	SocketHandler* get_m_socket_handler() {
		return m_socket_handler;
	}

	//サーバプログラム実行
	void run();


};



#endif // !SERVERCONTROLLFACADE_H





