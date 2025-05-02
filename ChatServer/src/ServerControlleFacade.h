#ifndef SERVERCONTROLLFACADE_H_
#define SERVERCONTROLLFACADE_H_
#include "SocketHandler.h"
#include "MessageType001.h"
#include "MessageType002.h"
#include "MessageType003.h"
#include "MessageType004.h"
#include "MessageType005.h"
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
#define LOAD_MESSAGE_NUM 20
#define MAX_RESEND_COUNT 3

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
	void heart_beat_check_process();
	void heart_beat_response_process(const std::unique_ptr<ClientHandler::Client>& client);
	void setting_select_process();


	
private:
	//ログイン関連
	std::tuple<std::string, std::string> sign_up_process(const std::string& mail_addr, const std::string& password, const std::string& ip);
	std::tuple<std::string, std::string> sign_in_process(const std::string& mail_addr, const std::string& password, const std::string& ip, std::string* id, std::string* error_msg);
	std::string sign_in_process(int& auth_type, const char* token, const char* secret);

	//メッセージ処理関連
	void mt000_process(const std::unique_ptr<MessageType000>& mt000, std::unique_ptr<ClientHandler::Client>& client);
	void mt001_process(const std::unique_ptr<MessageType001>& mt001, std::unique_ptr<ClientHandler::Client>& client, std::string* error_msg_text, bool* login_result);
	void mt003_process(const std::unique_ptr<MessageType003>& mt003, std::string* error_msg, bool* action_result);
	void mt004_process(const std::unique_ptr<MessageType004>& mt004);
	void mt101_process(const std::unique_ptr<ClientHandler::Client>& client, const std::string& error_msg, const bool login_result);
	void mt104_process(const std::unique_ptr<MessageType004>& mt004);
	std::vector<std::string> mt104A_process(const std::unique_ptr<MessageType004>& mt004);
	void mt104B_process(const std::unique_ptr<MessageType004>& mt004, const std::vector<std::string>& unsend_cids);
	void resend_message_process(const json& message, const std::string& cid);
	
};



#endif // !SERVERCONTROLLFACADE_H





