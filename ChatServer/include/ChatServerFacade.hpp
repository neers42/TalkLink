#ifndef CHATSERVERFACADE_HPP
#define CHATSERVERFACADE_HPP
#include "Chat/ClientHandler.hpp"
#include "Chat/ChatRoomHandler.hpp"
#include "Chat/MessageIDGenerator.hpp"
#include "Utils/common.hpp"
#include "Database/MySQLClient.hpp"
#include "Database/RedisClient.hpp"
#include "Messages/BaseMessage.hpp"
#include "Messages/HeartBeatMessage.hpp"
#include "Messages/ClientSessionMessage.hpp"
#include "Messages/ClientSettingControlMessage.hpp"
#include "Messages/ChatRoomControlMessage.hpp"
#include "Messages/ChatMessage.hpp"
#include "Messages/ClientSessionResponseMessage.hpp"
#include "Messages/ClientSettingControlResponseMessage.hpp"
#include "Messages/ChatRoomControlDeliveryMessage.hpp"
#include "Messages/ChatRoomControlAckMessage.hpp"
#include "Messages/ChatDeliveryMessage.hpp"
#include "Messages/ChatAckMessage.hpp"
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
#define LOAD_MESSAGE_NUM 20
#define MAX_RESEND_COUNT 3
#define SERVER_CERT_FILE_PATH "/app/ChatServer/certs/server-cert.pem"
#define SERVER_PRIVATEKEY_FILE_PATH "/app/ChatServer/certs/server-key.pem"
#define SOCKET_CONFIG_FILE_PATH "/app/ChatServer/conf/socket_setting.ini"

class ChatServerFacade
{
private:
	std::unique_ptr<MySQLClient> _mysql_client;
	std::unique_ptr<RedisClient> _redis_client;
	std::unique_ptr<ProtocolHandler> _protocol_handler;
	std::unique_ptr<SocketHandler> _socket_handler;
	std::unique_ptr<ServerTime> _server_time;
	std::unique_ptr<ClientHandler> _client_handler;
	std::unique_ptr<ThreadPoolHandler> _thread_pool_handler;
	std::unique_ptr<ChatroomHandler> _chat_room_handler;
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
	void handle_disconnect_process(const socket_t socket);

private:
	// メッセージ処理関連
	void heartbeat_process(const std::unique_ptr<HeartBeatMessage> &heartbeat_message, std::shared_ptr<Client> &client);
	void heartbeat_process(const std::unique_ptr<HeartBeatMessage> &heartbeat_message, const socket_t socket);
	bool session_process(const std::unique_ptr<ClientSessionMessage> &session_message, const socket_t socket, std::string& cid);
	void chatroom_control_process(const std::unique_ptr<ChatRoomControlMessage> &chatroom_control_message, std::string *error_msg, bool *action_result);
	void chat_message_process(const std::unique_ptr<ChatMessage> &chat_message);
	void session_response_process(const std::unique_ptr<ClientSessionMessage> &session_message, const socket_t socket, const std::string &cid, const bool login_result, const bool recv_flag);
	void chatroom_control_response_process(const std::unique_ptr<ChatRoomControlMessage> &chatroom_control_message, const std::shared_ptr<Client> &client, socket_t socket);
	const std::tuple<int, std::vector<std::string>> chatroom_control_delivery_process(const std::unique_ptr<ChatRoomControlMessage> &chatroom_control_message, socket_t socket);
	void chatroom_control_ack_process(const std::unique_ptr<ChatRoomControlMessage> &chatroom_control_message, const std::vector<std::string> &unsend_cids, const std::shared_ptr<Client> &client);
	std::vector<ChatRoomControlDeliveryMessage::ClientInfo> generate_client_infos(const std::vector<std::string> &cids);
	void chat_message_response_process(const std::unique_ptr<ChatMessage> &chat_message, const socket_t socket);
	const std::tuple<int, std::vector<std::string>> chat_message_delivery_process(const std::unique_ptr<ChatMessage> &chat_message, socket_t socket);
	void chat_message_ack_process(const std::unique_ptr<ChatMessage> &chat_message, const int send_cid_num, const std::vector<std::string> &unsend_cids, const socket_t socket);
	void resend_message_process(const json &message, const socket_t socket);

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
				LOG_ERROR("Sender with ID '{}' not found. Cannot send message.", message->get_sender_cid());
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
							spdlog::warn("The MT{} message was not sent to the client(id:{}).", message->get_message_type(), cid);
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
						if constexpr (std::is_same<T, ChatRoomControlDeliveryMessage>::value)
						{
							unsend_cids.push_back(receiver_cid);
						}
						else if constexpr (std::is_same<T, ChatDeliveryMessage>::value)
						{
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
							spdlog::warn("The MT{} message was not sent to the client(id:{}).", message->get_message_type(), receiver_cid);
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

#endif // !CHATSERVERFACADE_HPP
