#ifndef CHATSERVERFACADE_H_
#define CHATSERVERFACADE_H_
#include "SocketHandler.h"
#include "Network/MessageType001.h"
#include "Network/MessageType002.h"
#include "Network/MessageType003.h"
#include "Network/MessageType004.h"
#include "Network/MessageType005.h"
#include "ServerTime.h"
#include "Chat/ClientHandler.h"
#include "Chat/ChatRoomHandler.h"
#include "Authentication/FirebaseAuthHandler.h"
#include "ThreadPoolHandler.h"
#include "Chat/MessageIDGenerator.h"
#include "Utils/common.h"
#include "LogHandler.h"
#include "Database/MySQLClient.h"
#include "Database/RedisClient.h"
#include "Network/ProtocolHandler.h"
#include <mutex>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#ifdef _WIN32
using socket_t = SOCKET;
#define INVALID_SOCKET_FD INVALID_SOCKET
#else
using socket_t = int; // POSIX systems use int for sockets
#define INVALID_SOCKET_FD -1
#endif // _WIN32

#ifdef __linux__
#define USE_POLL
#else
#define USE_SELECT
#endif

#define DISCONNECT_SOCKET 0
#define ACCEPT_FAILED 0
#define ACCEPT_SUCCEEDED 1
#define DEBUG_MODE
#define TLS_PROTOCOL
#define LOAD_MESSAGE_NUM 20
#define MAX_RESEND_COUNT 3

class ChatServerFacade
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
	std::mutex _connect_client_list_mutex, _chatroom_list_mutex;
	std::mutex _hb_connect_list_mutex;
	std::mutex _cout_mutex;
	std::mutex _mutex;
	std::string _server_ip;

public:
	// コンストラクタ
	ChatServerFacade();

	// デストラクタ
	~ChatServerFacade();

	// サーバプログラム実行
	void run();

#ifdef USE_SELECT
	void prepare_client_fd_set();
	void setting_select_process();
#endif

	void show_server_running_info();
	const std::string &get_server_ip();
	void set_server_ip(const std::string &ip);

	//シーケンス関連
	void allocate_process(const socket_t socket);
	bool accept_client_socket_process();
	void message_handle_process(const socket_t socket);
	void heartbeat_check_process();
	void heartbeat_nonresponse_process(const std::shared_ptr<Client> &client);
	void handle_disconnect_process(const std::shared_ptr<Client> client);
	void handle_disconnect_process(const socket_t socket);

private:
	// ログイン関連
	std::tuple<std::string, std::string> sign_up_process(const std::string &mail_addr, const std::string &password, const std::string &ip, std::string *id, std::string *error_msg);
	std::tuple<std::string, std::string> sign_in_process(const std::string &mail_addr, const std::string &password, const std::string &ip, std::string *id, std::string *error_msg);
	std::string sign_in_process(int &auth_type, const char *token, const char *secret);

	// メッセージ処理関連
	void mt000_process(const std::unique_ptr<MessageType000> &mt000, std::shared_ptr<Client> &client);
	void mt000_process(const std::unique_ptr<MessageType000> &mt000, const socket_t socket);
	std::string mt001_process(const std::unique_ptr<MessageType001> &mt001, std::shared_ptr<Client> &client, std::string *error_msg_text, bool *login_result);
	std::string mt001_process(const std::unique_ptr<MessageType001> &mt001, const socket_t socket, std::string *error_msg_text, bool *login_result);
	void mt003_process(const std::unique_ptr<MessageType003> &mt003, std::string *error_msg, bool *action_result);
	void mt004_process(const std::unique_ptr<MessageType004> &mt004);
	void mt101_process(const socket_t socket, const std::string &cid, const std::string &error_msg, const bool login_result, const bool recv_flag);
	void mt103_process(const std::unique_ptr<MessageType003> &mt003, const std::shared_ptr<Client> &client, socket_t socket);
	const std::tuple<int, std::vector<std::string>> mt103A_process(const std::unique_ptr<MessageType003> &mt003, socket_t socket);
	void mt103B_process(const std::unique_ptr<MessageType003> &mt003, const std::vector<std::string> &unsend_cids, const std::shared_ptr<Client> &client);
	std::vector<MessageType103A::ClientInfo> generate_client_infos(const std::vector<std::string> &cids);
	void mt104_process(const std::unique_ptr<MessageType004> &mt004, const socket_t socket);
	const std::tuple<int, std::vector<std::string>> mt104A_process(const std::unique_ptr<MessageType004> &mt004, socket_t socket);
	void mt104B_process(const std::unique_ptr<MessageType004> &mt004, const int send_cid_num, const std::vector<std::string> &unsend_cids, const socket_t socket);
	void resend_message_process(const json &message, const std::string &cid);

	template <typename T>
	const std::tuple<int, std::vector<std::string>> send_message_to_room(const std::unique_ptr<T> &message, socket_t socket, const std::string& sender_cid)
	{
		std::vector<std::string> unsend_cids;
		try
		{
			// メッセージをデシリアライズする
			json json_message = T::deserialize_message(message);

			if (socket == INVALID_SOCKET_FD)
			{
				LOG_ERROR("Sender with ID '{}' not found. Cannot send message.", message->get_sender_id());
				return std::make_tuple(0, std::vector<std::string>()); // 送信者が見つからない場合は処理を終了
			}
			// 送り先のclientの情報を取得する
			auto receivers_cids = this->_chat_room_handler->get_client_infos_by_chatroom_id(message->get_room_id());

			// ChatroomHandlerが保持しているクライアントリストからチャットルームが検出できない場合
			if (receivers_cids.size() == 0)
			{
				//データベースからcidリストをgetする。
				std::vector<std::string> send_cids = this->_mysql_client->get_client_list_from_chatroom(message->get_room_id());
				// データベースにもチャットルームIDが存在しない場合
				if (send_cids.size() == 0)
				{
					LOG_ERROR("Invalid receiver cid.Interruputed Send message to Chatroom.");
					return std::make_tuple(0, std::vector<std::string>());
				}

				for (const auto &cid : send_cids)
				{
					int receiver_list_num = this->_client_handler->search_cid_from_clients(cid);
					// 接続しているクライアントリストに送信先のクライアントが存在しない場合
					if (receiver_list_num == NOT_EXIST_CLIENT_ID)
					{
						if constexpr (std::is_same<T, MessageType103A>::value)
						{
							this->_mysql_client->insert_unsend_control_message(message->get_message_id(), cid, message->get_room_id(), json_message, message->get_created_at());
						}
						else if constexpr (std::is_same<T, MessageType104A>::value)
						{
							this->_mysql_client->insert_unsend_message(message->get_send_message_id(), cid, message->get_room_id());
						}
						else
						{
							LOG_ERROR("Unsupported message type for unsend message insertion.");
							return std::make_tuple(0, std::vector<std::string>());
						}
						unsend_cids.push_back(cid);
						continue;
					}
					else
					{ // 接続しているクライアントリストに送信先のクライアントが存在する場合
						
						if (cid == sender_cid)
						{
							//送信元のsocketと同一であった場合はスキップする。
							continue;
						}
						auto send_socket = this->_client_handler->get_client_socket_by_cid(cid);
						if(send_socket == INVALID_SOCKET_FD){
							this->_mysql_client->insert_unsend_message(message->get_message_id(), cid, message->get_room_id());
							unsend_cids.push_back(cid);
							continue;
						}
						bool send_result = this->_client_handler->client_message_send(this->_client_handler->get_client_socket_by_cid(cid), json_message);
						if (send_result == true)
						{
							spdlog::info("The MT{} message was successfully sent to the client(id:{}).", message->get_message_type(), cid);
						}
						else
						{
							LOG_ERROR("The MT{} message was not sent to the client(id:{}).", message->get_message_type(), cid);
							this->_mysql_client->insert_unsend_message(message->get_message_id(), cid, message->get_room_id());
							unsend_cids.push_back(cid);
						}
					}
				}
				if (unsend_cids.size() == 0)
				{
					spdlog::info("The MT{} message was successfully sent to all clients.", message->get_message_type());
				}
				else if (unsend_cids.size() > 0)
				{
					spdlog::warn("The MT{} message was not sent to all clients.", message->get_message_type());
				}
				return std::make_tuple(send_cids.size(), unsend_cids);
			}
			// ChatroomHandlerが保持しているクライアントリストからチャットルームが検出できた場合
			else
			{
				for (const auto &receiver_cid : receivers_cids)
				{
					int receiver_list_num = this->_client_handler->search_cid_from_clients(receiver_cid);
					if (receiver_list_num == NOT_EXIST_CLIENT_ID)
					{
						// データベースにcidが存在するか確認
						if (!this->_mysql_client->is_client_id_exist(receiver_cid))
						{ // cidがデータベースに存在しない場合
							LOG_ERROR("The client ID '{}' does not exist in the database. Skipping unsend message insertion.", receiver_cid);
							continue;
						}

						// クライアントの情報が見つからない場合、unsendメッセージをデータベースに保存する
						if constexpr (std::is_same<T, MessageType103A>::value)
						{
							this->_mysql_client->insert_unsend_control_message(message->get_message_id(), receiver_cid, message->get_room_id(), json_message, message->get_created_at());
							unsend_cids.push_back(receiver_cid);
						}
						else if constexpr (std::is_same<T, MessageType104A>::value)
						{
							this->_mysql_client->insert_unsend_message(message->get_send_message_id(), receiver_cid, message->get_room_id());
							unsend_cids.push_back(receiver_cid);
						}
						continue;
					}
					else
					{
						//送信先と受信先のクライアントが一致する場合スキップする。
						if (socket == this->_client_handler->get_client_socket_by_cid(receiver_cid))
						{
							continue;
						}
						//メッセージを送信
						bool send_result = this->_client_handler->client_message_send(this->_client_handler->get_client_socket_by_cid(receiver_cid), json_message);
						if (send_result == true)	//送信が成功した場合
						{
							spdlog::info("The MT{} message was successfully sent to the client(id:{}).", message->get_message_type(), receiver_cid);
						}
						else
						{
							LOG_ERROR("The MT{} message was not sent to the client(id:{}).", message->get_message_type(), receiver_cid);
							this->_mysql_client->insert_unsend_message(message->get_message_id(), receiver_cid, message->get_room_id());
							unsend_cids.push_back(receiver_cid);
						}
					}
				}
				if (unsend_cids.size() == 0)
				{
					spdlog::info("The MT{} message was successfully sent to all clients.", message->get_message_type());
				}
				else if (unsend_cids.size() > 0)
				{
					spdlog::warn("The MT{} message was not sent to all clients.", message->get_message_type());
				}
				return std::make_tuple(receivers_cids.size(), unsend_cids);
			}
			
		}
		catch (const std::out_of_range &e)
		{
			// エラーハンドリング
			LOG_ERROR("Sender ID not found. exception content: {}", e.what());
			return std::make_tuple(0, std::vector<std::string>());
		}
	}
};

#endif // !CHATSERVERFACADE_H
