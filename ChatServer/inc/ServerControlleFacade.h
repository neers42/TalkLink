#ifndef SERVERCONTROLLFACADE_H_
#define SERVERCONTROLLFACADE_H_
#include "SocketHandler.h"
#include "MT001Receiver.h"
#include "MT002Receiver.h"
#include "Sender.h"
#include "ServerTime.h"
#include "ClientHandler.h"
#include "ChatRoomHandler.h"
#include "FirebaseAuthHandler.h"

class ServerControlleFacade
{
private:
	FirebaseAuthHandler* m_firebase_auth_handler = new FirebaseAuthHandler();
	SocketHandler* m_socket_handler = new SocketHandler();
	MT001Receiver* m_mt001receiver = new MT001Receiver();
	MT002Receiver* m_mt002receiver = new MT002Receiver();
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

	void client_select_setting();


};



#endif // !SERVERCONTROLLFACADE_H





