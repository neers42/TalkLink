#ifndef SERVERCONTROLLFACADE_H_
#define SERVERCONTROLLFACADE_H_
#include "SocketHandler.h"
#include "ClientInfomationListHandler.h"


class ServerControlleFacade
{
private:
	ServerControlleFacade* m_sever_controlle_facade;
	SocketHandler* m_socket_handler;
	ClientInfomationListHandler* m_client_infomation_list_handler;


public:
	//コンストラクタ
	ServerControlleFacade();

	//デストラクタ
	~ServerControlleFacade();

	//サーバプログラム実行
	void run();


};



#endif // !SERVERCONTROLLFACADE_H





