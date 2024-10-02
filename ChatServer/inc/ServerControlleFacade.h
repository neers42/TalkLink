#ifndef SERVERCONTROLLFACADE_H_
#define SERVERCONTROLLFACADE_H_
#include "SocketHandler.h"
#include "MT000Receiver.h"
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
	std::unique_ptr<FirebaseAuthHandler> m_firebase_auth_handler;
	std::unique_ptr<SocketHandler> m_socket_handler;
	std::unique_ptr<MT000Receiver> m_mt000receiver;
	std::unique_ptr<MT001Receiver> m_mt001receiver;
	std::unique_ptr<MT002Receiver> m_mt002receiver;
	std::unique_ptr<Sender> m_sender;
	std::unique_ptr<ServerTime> m_server_time;
	std::unique_ptr<ClientHandler> m_client_handler;
	std::unique_ptr<ThreadPoolHandler> m_thread_pool_handler;
	std::unique_ptr<ChatroomHandler> m_chat_room_handler;
	std::mutex m_connect_client_list_mutex;
	std::mutex m_hb_connect_list_mutex;
	std::mutex m_cout_mutex;
	std::mutex m_mutex;

public:
	//コンストラクタ
	ServerControlleFacade();

	//デストラクタ
	~ServerControlleFacade();


	//サーバプログラム実行
	void run();
	
	void setting_select_info_to_client_msg();

	void setting_select_info_to_client_hb();

	void show_server_running_info();

	bool accept_client_socket_for_msg_process();


	void message_receive_process(int detection_num);

	void mt000_process(std::string& receive_data, int monitor_result);

	void mt001_process(std::string& receive_data, int& monitor_result);

	void mt002_process(std::string& receive_data, int& receive_result);

	void heart_beat_send_process();

	void heart_beat_check_process();

	void setting_select_process();

	
private:
	std::string sign_up_process(const std::string& mail_addr, const std::string& password);
	std::string sign_in_process(const std::string& mail_addr, const std::string& password);
	std::string sign_in_process(int& auth_type, const char* token, const char* secret);

};



#endif // !SERVERCONTROLLFACADE_H





