#ifndef CHATROOMHANDLER_H_
#define CHATROOMHANDLER_H_
#include <string>
#include <iostream>
#include <vector>
#include "ClientHandler.h"
#include "AppException.h"
#include "StringFormat.h"
#include "ServerTime.h"
#define MAX_ACTIVE_CHATROOM_NUM 1000	//activeなチャットルームの最大数
#define MAX_CLIENT_NUM_IN_CHATROOM 1000	//チャットルームに加入できるクライアントの最大数


class ChatroomHandler
{
private:
	class ChatRoom
	{

	public:
		struct ClientInfo
		{
			std::string cid;
			std::string name;

			bool operator==(const ClientInfo& other_client) const {
				return this->cid == other_client.cid && this->name == other_client.name;
			}
		};
		
		//コンストラクタ
		/*template<typename...Args>
		ChatRoom(const std::string& chatroom_name, Args... args) : _chatroom_name{chatroom_name} {
			this->_chatroom_id = ChatroomHandler::issue_chatroom_id();
			_client_list.push_back(args...);
		}*/

		ChatRoom(const std::string& chatroom_name, const std::vector<ChatroomHandler::ChatRoom::ClientInfo>& client_infos)
			: _chatroom_name{ chatroom_name }, _client_list(client_infos) {
			this->_chatroom_id = ChatroomHandler::issue_chatroom_id();
		}

		//デストラクタ
		~ChatRoom();

		//クライアントルームにクライアントを加入する
		bool add_client_chatroom(const ChatroomHandler::ChatRoom::ClientInfo&  client_info);

		//クライアントルームからクライアントを削除する
		bool delete_client_chatroom(const ChatroomHandler::ChatRoom::ClientInfo& client_info);

		std::string& get_chatroom_id();
		const std::string& get_chatroom_id() const;

		std::string& get_chatroom_name();
		const std::string& get_chatroom_name() const;

		std::vector<ClientInfo>& get_client_list();
		const std::vector <ClientInfo>& get_client_list() const;


	private:
		std::string _chatroom_id = "";
		std::string _chatroom_name = "";
		std::vector<ClientInfo> _client_list;

	private:
		std::vector<ChatRoom::ClientInfo>::iterator search_client_form_chatroom(const ChatRoom::ClientInfo& client_info);
		
	};
	
	//public関数
public:
	ChatroomHandler();
	~ChatroomHandler();
	bool add_chatroom(const ChatRoom& chatroom);
	bool delete_chatroom(const ChatRoom& chatroom);
	std::vector<ChatRoom::ClientInfo>& get_client_list(int list_num);
	std::vector<ChatRoom::ClientInfo> get_client_list(const std::string& chatroom_id);
	std::vector<ChatRoom> get_chatroom_list();
	int search_chatroom_id_from_list(const std::string& chatroom_id);
	std::vector<ChatroomHandler::ChatRoom::ClientInfo> get_client_list_from_chatroom(const std::string& chatroom_id);
	// チャットルームを生成する関数
	bool create_chatroom(const std::string& chatroom_name, const std::vector<ChatroomHandler::ChatRoom::ClientInfo>& initial_clients);

	
	//private変数
public:
	std::vector<ChatRoom> chatroom_list;

	//private関数
private:
	//chatroomIDを発行する
	static std::string issue_chatroom_id();
};
#endif // !CHATROOMHANDLER_H_



