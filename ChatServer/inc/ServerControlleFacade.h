#ifndef SERVERCONTROLLFACADE_H_
#define SERVERCONTROLLFACADE_H_
#include "SocketHandler.h"
#include "MT001Receiver.h"
#include "Sender.h"
#include "ServerTime.h"
#include "ClientHandler.h"
#include "ChatRoomHandler.h"


class ServerControlleFacade
{
private:
	SocketHandler* m_socket_handler = new SocketHandler();
	MT001Receiver* m_001receiver = new MT001Receiver();
	Sender* m_sender = new Sender();
	ServerTime* m_server_time = new ServerTime();
	ClientHandler m_client_handler = ClientHandler();


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





