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
	/*	struct ClientInfo
		{
			std::string cid;
			std::string name;

			bool operator==(const ClientInfo& other_client) const {
				return this->cid == other_client.cid && this->name == other_client.name;
			}
		};*/
		
		//コンストラクタ
		/*template<typename...Args>
		ChatRoom(const std::string& chatroom_name, Args... args) : _chatroom_name{chatroom_name} {
			this->_chatroom_id = ChatroomHandler::issue_chatroom_id();
			_client_list.push_back(args...);
		}*/
		//コンストラクタ
		ChatRoom();
		ChatRoom(const std::string& chatroom_name,
			const std::string& icon_url,
			const std::vector<std::string>& cids
			);
		

		//デストラクタ
		~ChatRoom();

		//クライアントルームにクライアントを加入する
		bool add_client_chatroom(const std::string& cid);

		//クライアントルームからクライアントを削除する
		bool delete_client_chatroom(const std::string& cid);

		std::string& get_chatroom_id();
		const std::string& get_chatroom_id() const;

		std::string& get_chatroom_name();
		const std::string& get_chatroom_name() const;

		std::string& get_chatroom_icon();
		const std::string& get_chatroom_icon() const;

		std::vector<std::string>& get_client_list();
		const std::vector <std::string>& get_client_list() const;


	private:
		std::string _chatroom_id = "";
		std::string _chatroom_name = "";
		std::string _chatroom_icon_url = "";
		std::vector<std::string> _cid_list;

	private:
		std::vector<std::string>::iterator search_client_form_chatroom(const std::string& cid);
		
	};
	
	//public関数
public:
	ChatroomHandler();
	~ChatroomHandler();
	ChatRoom create_chatroom(const std::string& chatroom_name, const std::string& icon_url, const std::vector<std::string>& room_member);
	void add_chatroom(const ChatRoom& chatroom);
	void delete_chatroom(const ChatRoom& chatroom, const std::string& room_id);
	std::vector<std::string> get_client_infos_by_chatroom_id(int list_num);
	std::vector<std::string> get_client_infos_by_chatroom_id(const std::string& room_id);
	std::vector<ChatRoom> get_chatroom_list();
	int search_chatroom_id_from_list(const std::string& chatroom_id);
	std::vector<ChatroomHandler::ChatRoom>::iterator find_chatroom_id_from_list(const std::string& room_id);
	std::vector<std::string> get_client_list_from_chatroom(const std::string& room_id);
	// チャットルームを生成する関数
	//ChatRoom create_chatroom(const std::string& chatroom_name, const std::vector<ChatroomHandler::ChatRoom::ClientInfo>& initial_clients);

	
	//private変数
public:
	std::vector<ChatRoom> chatroom_list;

	//private関数
private:
	//chatroomIDを発行する
	static std::string issue_chatroom_id();
};
#endif // !CHATROOMHANDLER_H_



