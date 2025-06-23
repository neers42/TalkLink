#ifndef CHATSERVERFACADE_H_
#define CHATSERVERFACADE_H_
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
	//コンストラクタ
	ChatServerFacade();

	//デストラクタ
	~ChatServerFacade();


	//サーバプログラム実行
	void run();
	
	void prepare_client_fd_set();

	void show_server_running_info();

	bool accept_client_socket_process();

	const std::string& get_server_ip();
	void set_server_ip(const std::string& ip);
	void handle_changed_socket_process(const SOCKET socket);
	void message_handle_process(const SOCKET socket);	
	void heartbeat_check_process();
	void heartbeat_nonresponse_process(const std::shared_ptr<Client>& client);
	void setting_select_process();
	void handle_disconnect_process(const std::shared_ptr<Client> client);


	
private:
	//ログイン関連
	std::tuple<std::string, std::string> sign_up_process(const std::string& mail_addr, const std::string& password, const std::string& ip, std::string* id, std::string* error_msg);
	std::tuple<std::string, std::string> sign_in_process(const std::string& mail_addr, const std::string& password, const std::string& ip, std::string* id, std::string* error_msg);
	std::string sign_in_process(int& auth_type, const char* token, const char* secret);

	//メッセージ処理関連
	void mt000_process(const std::unique_ptr<MessageType000>& mt000, std::shared_ptr<Client>& client);
	void mt001_process(const std::unique_ptr<MessageType001>& mt001, std::shared_ptr<Client>& client, std::string* error_msg_text, bool* login_result);
	void mt003_process(const std::unique_ptr<MessageType003>& mt003, std::string* error_msg, bool* action_result);
	void mt004_process(const std::unique_ptr<MessageType004>& mt004);
	void mt101_process(std::shared_ptr<Client>& client, const std::string& error_msg, const bool login_result, const bool recv_flag);
	void mt103_process(const std::unique_ptr<MessageType003>& mt003, const std::shared_ptr<Client>& client);
	std::vector<std::string> mt103A_process(const std::unique_ptr<MessageType003>& mt003);
	void mt103B_process(const std::unique_ptr<MessageType003>& mt003, const std::vector<std::string>& unsend_cids);
	std::vector<MessageType103A::ClientInfo> generate_client_infos(const std::vector < std::string >& cids);
	void mt104_process(const std::unique_ptr<MessageType004>& mt004, const std::shared_ptr<Client>& client);
	std::vector<std::string> mt104A_process(const std::unique_ptr<MessageType004>& mt004);
	void mt104B_process(const std::unique_ptr<MessageType004>& mt004, const std::vector<std::string>& unsend_cids, const std::shared_ptr<Client>& client);
	void resend_message_process(const json& message, const std::string& cid);

	template <typename T>
	void send_message_to_room(const std::unique_ptr<T>& message, std::vector<std::string>* unsend_cids)
	{
		try
	{
			//メッセージをデシリアライズする
			json json_message = T::deserialize_message(message);
			auto sender = this->_client_handler->get_client_by_cid(message->get_sender_id());
			if (sender == nullptr) {
				LOG_ERROR("Sender with ID '{}' not found. Cannot send message.", message->get_sender_id());
				return; // 送信者が見つからない場合は処理を終了
			}
			//送り先のclientの情報を取得する
			auto receivers_cids = this->_chat_room_handler->get_client_infos_by_chatroom_id(message->get_room_id());

			//ChatroomHandlerが保持しているクライアントリストからチャットルームが検出できない場合
			if (receivers_cids.size() == 0) {
				std::vector<std::string> send_cids = this->_mysql_client->get_client_list_from_chatroom(message->get_room_id());
				//データベースにもチャットルームIDが存在しない場合
				if (send_cids.size() == 0) {
					return;
				}


				for (const auto& cid : send_cids) {
					int receiver_list_num = this->_client_handler->search_cid_from_clients(cid);
					//接続しているクライアントリストに送信先のクライアントが存在しない場合
					if (receiver_list_num == NOT_EXIST_CLIENT_ID) {
						if constexpr (std::is_same<T, MessageType103A>::value) {
							this->_mysql_client->insert_unsend_control_message(message->get_message_id(), cid, message->get_room_id(), json_message, message->get_created_at());
						}else if constexpr (std::is_same<T, MessageType104A>::value) {
							this->_mysql_client->insert_unsend_message(message->get_send_message_id(), cid, message->get_room_id());
						}
						else {
							LOG_ERROR("Unsupported message type for unsend message insertion.");
							return;
						}
						unsend_cids->push_back(cid);
						continue;
					}
					else {
						//クライアントの情報を取得する
						auto receiver = this->_client_handler->get_client_by_index(receiver_list_num);
						//jsonメッセージを送信する(送信先のsslがnullではない、かつ送り先と送信先のsslが一致しないこと)
						if (receiver != nullptr && receiver->get_ssl() != nullptr && receiver->get_ssl() != sender->get_ssl()) {
							if (this->_protocol_handler->send_message_tls(receiver, json_message)) {
								spdlog::info("The MT{} message was successfully sent to the client(ip:{}, id:{}).", message->get_message_type(), sender->get_ipv4_address(), sender->get_id());
							}
							else {
								LOG_ERROR("The MT{} message was not sent to the client(ip:{}, id:{}).", message->get_message_type(), sender->get_ipv4_address(), sender->get_id());
								this->_mysql_client->insert_unsend_message(message->get_message_id(), cid, message->get_room_id());
								unsend_cids->push_back(receiver->get_id());

							}
						}
						else if (receiver->get_ssl() == sender->get_ssl()) {
							//送信先と送信元のsslが一致する場合
#define DEBUG
#ifdef DEBUG
						//this->_protocol_handler->send_message_tls(sender->ssl, json_message);
#endif // DEBUG
#undef DEBUG
							continue;
						}
						else if (receiver->get_ssl() == nullptr) {
							//クライアントの情報が見つからない場合、unsendメッセージをデータベースに保存する
							if constexpr (std::is_same<T, MessageType103A>::value) {
								this->_mysql_client->insert_unsend_control_message(message->get_message_id(), cid, message->get_room_id(), json_message, message->get_created_at());
							}
							else if constexpr (std::is_same<T, MessageType104A>::value) {
								this->_mysql_client->insert_unsend_message(message->get_send_message_id(), cid, message->get_room_id());
							}
							unsend_cids->push_back(receiver->get_id());
							continue;
						}
						else {
							LOG_ERROR("Failed to send MT{}(mid:{}).", message->get_message_type(), message->get_message_id());
						}
					}
				}

			}
			else {
				//ChatroomHandlerが保持しているクライアントリストからチャットルームを検出できた場合
				for (const auto& receiver_cid : receivers_cids) {
					int receiver_list_num = this->_client_handler->search_cid_from_clients(receiver_cid);
					if (receiver_list_num == NOT_EXIST_CLIENT_ID) {
						// データベースにcidが存在するか確認
						if (!this->_mysql_client->is_client_id_exist(receiver_cid)) {	//cidがデータベースに存在しない場合
							LOG_ERROR("The client ID '{}' does not exist in the database. Skipping unsend message insertion.", receiver_cid);
							continue;
						}

						// クライアントの情報が見つからない場合、unsendメッセージをデータベースに保存する
						if constexpr (std::is_same<T, MessageType103A>::value) {
							this->_mysql_client->insert_unsend_control_message(message->get_message_id(), receiver_cid, message->get_room_id(), json_message, message->get_created_at());
							
						}
						else if constexpr (std::is_same<T, MessageType104A>::value) {
							this->_mysql_client->insert_unsend_message(message->get_send_message_id(), receiver_cid, message->get_room_id());
						}
						continue;
					}
					else {
						//クライアントの情報を取得する
						auto receiver = this->_client_handler->get_client_by_index(receiver_list_num);
						if (receiver == nullptr) {
							LOG_ERROR("Receiver with ID '{}' not found in client list. Cannot send message.", receiver_cid);

						}
						//jsonメッセージを送信する(送信先のsslがnullではない、かつ送り先と送信先のsslが一致しないこと)
						if (receiver->get_ssl() != nullptr && receiver->get_ssl() != sender->get_ssl()) {
							if (this->_protocol_handler->send_message_tls(receiver, json_message)) {
								spdlog::info("The MT{} message was successfully sent to the client(ip:{}, id:{}).", message->get_message_type(), sender->get_ipv4_address(), sender->get_id());
							}
							else {
								LOG_ERROR("The MT{} message was not sent to the client(ip:{}, id:{}).", message->get_message_type(), sender->get_ipv4_address(), sender->get_id());
								unsend_cids->push_back(receiver->get_id());
							}
						}
						else if (receiver->get_ssl() == sender->get_ssl()) {
							//送信先と送信元のsslが一致する場合
#define DEBUG
#ifdef DEBUG
							this->_protocol_handler->send_message_tls(sender, json_message);
#endif // DEBUG
#undef DEBUG
							continue;
						}
						else if (receiver->get_ssl() == nullptr) {
							// データベースにcidが存在するか確認
							if (!this->_mysql_client->is_client_id_exist(receiver_cid)) {
								LOG_ERROR("The client ID '{}' does not exist in the database. Skipping unsend message insertion.", receiver_cid);
								continue;
							}
							//クライアントの情報が見つからない場合、unsendメッセージをデータベースに保存する
							if (!this->_mysql_client->insert_unsend_message(message->get_message_id(), receiver->get_id(), message->get_room_id())) {
								LOG_ERROR("Failed to insert unsend message for client ID '{}'.", receiver->get_id());
							}
							continue;
						}
					}

				}
			}

			if (unsend_cids->size() == 0) {
				spdlog::info("The MT{} message was successfully sent to all clients.", message->get_message_type());
			}
			else if (unsend_cids->size() > 0) {
				spdlog::warn("The MT{} message was not sent to all clients.", message->get_message_type());
			}

		}
		catch (const std::out_of_range& e)
		{
			// エラーハンドリング
			LOG_ERROR("Sender ID not found. exception content: {}", e.what());
			return ;

		}
	}
	
};



#endif // !CHATSERVERFACADE_H





