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
#include "ThreadPoolHandler.h"
#include "common.h"
#include "LogHandler.h"
#include <mutex>

#define DISCONNECT_SOCKET 0
#define ACCEPT_FAILED 0
#define ACCEPT_SUCCEEDED 1

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
	ThreadPoolHandler* m_thread_pool_handler = new ThreadPoolHandler(10);
	ChatroomHandler* m_chat_room_handler = new ChatroomHandler();


	std::mutex m_connect_client_list_mutex;
	std::mutex m_hb_connect_list_mutex;
	std::mutex m_cout_mutex;

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

	void setting_select_info_to_client_msg();

	void setting_select_info_to_client_hb();

	void show_server_running_info();


	bool accept_client_socket_for_msg_process();

	bool accept_client_socket_for_heart_beat_process();

	void message_receive_process(int detection_num);

	void mt001_process(std::string& receive_data, int& monitor_result);

	void mt002_process(std::string& receive_data, int& receive_result);

	void heart_beat_monitor_process();

	void heart_beat_send_process();

	void heart_beat_receive_process(int detection_num);

	void heart_beat_check_process();

	
private:
	std::string sign_up_process(const std::string& mail_addr, const std::string& password);
	std::string sign_in_process(const std::string& mail_addr, const std::string& password);
	std::string sign_in_process(int& auth_type, const char* token, const char* secret);

};



#endif // !SERVERCONTROLLFACADE_H





