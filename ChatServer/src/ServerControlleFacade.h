#ifndef SERVERCONTROLLFACADE_H_
#define SERVERCONTROLLFACADE_H_
#include "SocketHandler.h"
#include "MT000Receiver.h"
#include "MT001Receiver.h"
#include "MT002Receiver.h"
#include "MessageType001.h"
#include "MessageType002.h"
#include "MessageType003.h"
#include "Sender.h"
#include "ServerTime.h"
#include "ClientHandler.h"
#include "ChatRoomHandler.h"
#include "FirebaseAuthHandler.h"
#include "ThreadPoolHandler.h"
#include "common.h"
#include "LogHandler.h"
#include "DataBaseAccesor.h"
#include "ProtocolHandler.h"
#include <mutex>

#define DISCONNECT_SOCKET 0
#define ACCEPT_FAILED 0
#define ACCEPT_SUCCEEDED 1

#define DEBUG_MODE
#define TLS_PROTOCOL

class ServerControlleFacade
{
private:
	std::unique_ptr<FirebaseAuthHandler> _firebase_auth_handler;
	std::unique_ptr<DataBaseAccesor> _db_accesor;
	std::unique_ptr<ProtocolHandler> _protocol_handler;
	std::unique_ptr<SocketHandler> _socket_handler;
	std::unique_ptr<MT000Receiver> _mt000receiver;
	std::unique_ptr<MT001Receiver> _mt001receiver;
	std::unique_ptr<MT002Receiver> _mt002receiver;
	std::unique_ptr<Sender> _sender;
	std::unique_ptr<ServerTime> _server_time;
	std::unique_ptr<ClientHandler> _client_handler;
	std::unique_ptr<ThreadPoolHandler> _thread_pool_handler;
	std::unique_ptr<ChatroomHandler> _chat_room_handler;
	std::mutex _connect_client_list_mutex;
	std::mutex _hb_connect_list_mutex;
	std::mutex _cout_mutex;
	std::mutex _mutex;
	std::string _server_ip;

public:
	//コンストラクタ
	ServerControlleFacade();

	//デストラクタ
	~ServerControlleFacade();


	//サーバプログラム実行
	void run();
	
	void setting_select_info();

	void show_server_running_info();

	bool accept_client_socket_process();

	const std::string& get_server_ip();
	void set_server_ip(const std::string& ip);

	void message_handle_process(int detection_num);

	void mt000_process(std::string& receive_data, int monitor_result);

	void mt001_process(const json& receive_json, int& monitor_result);

	void mt002_process(const json& receive_json, int& receive_result);

	void mt002_process(const std::unique_ptr<MessageType002>& mt002, int& receive_result);

	void heart_beat_send_process();

	void heart_beat_check_process();

	void heart_beat_no_response_process(const int hb_zero_client_num);

	void setting_select_process();

	
private:
	std::string sign_up_process(const std::string& mail_addr, const std::string& password, const std::string& ip);
	std::string sign_in_process(const std::string& mail_addr, const std::string& password, const std::string& ip, std::string* id);
	std::string sign_in_process(int& auth_type, const char* token, const char* secret);
	
	
};



#endif // !SERVERCONTROLLFACADE_H





