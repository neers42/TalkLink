#ifndef SERVERCONTROLLFACADE_H_
#define SERVERCONTROLLFACADE_H_
#include "SocketHandler.h"
#include "MessageType001.h"
#include "MessageType002.h"
#include "MessageType003.h"
#include "ServerTime.h"
#include "ClientHandler.h"
#include "ChatRoomHandler.h"
#include "FirebaseAuthHandler.h"
#include "ThreadPoolHandler.h"
#include "MessageIDGenerator.h"
#include "common.h"
#include "LogHandler.h"
#include "MySQLClient.h"
#include "RedisClient.h"
#include "ProtocolHandler.h"
#include <mutex>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#define DISCONNECT_SOCKET 0
#define ACCEPT_FAILED 0
#define ACCEPT_SUCCEEDED 1
#define DEBUG_MODE
#define TLS_PROTOCOL

class ServerControlleFacade
{
private:
	std::unique_ptr<FirebaseAuthHandler> _firebase_auth_handler;
	std::unique_ptr<MySQLClient> _mysql_client;
	std::unique_ptr<RedisClient> _redis_client;
	std::unique_ptr<ProtocolHandler> _protocol_handler;
	std::unique_ptr<SocketHandler> _socket_handler;
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

	void handle_changed_socket_process(const SOCKET socket);

	void message_handle_process(const SOCKET socket);

	void mt000_process(std::string& receive_data, int monitor_result);

	void mt001_process(const std::unique_ptr<MessageType001>& mt001, int detection_num);

	void mt002_process(const std::unique_ptr<MessageType002>& mt002, int& receive_result);
	void heart_beat_check_process();
	void heart_beat_response_process(const ClientHandler::Client& client);
	void setting_select_process();


	
private:
	std::string sign_up_process(const std::string& mail_addr, const std::string& password, const std::string& ip);
	std::string sign_in_process(const std::string& mail_addr, const std::string& password, const std::string& ip, std::string* id);
	std::string sign_in_process(int& auth_type, const char* token, const char* secret);
	bool insert_client_info_to_db(const std::string& id, const std::string& name, const std::string& mail_addr, const std::string& password_hash);

	
};



#endif // !SERVERCONTROLLFACADE_H





