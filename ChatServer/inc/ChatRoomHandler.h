#ifndef CHATROOMHANDLER_H_
#define CHATROOMHANDLER_H_
#include <string>
#include <iostream>
#include <vector>
#include "ClientHandler.h"
#include "AppException.h"
#include "StringFormat.h"
#include "ServerTime.h"

#define DEBUG

class ChatroomHandler
{


private:
	class ChatRoom
	{

	public:
		
		//コンストラクタ
		template<typename...Args>
		ChatRoom(const std::string& chatroom_name, Args... args) : m_chatroom_name{chatroom_name} {
			this->m_chatroom_id = issue_chatroom_id();
			client_list.push_back(args...);
		}

		//デストラクタ
		~ChatRoom();

		//クライアントルームにクライアントを加入する
		bool add_client_chatroom(const ClientHandler::Client& client);

		//クライアントルームからクライアントを削除する
		bool delete_client_chatroom(const ClientHandler::Client& client);



	private:
		std::string m_chatroom_id = "";
		std::string m_chatroom_name = "";
		std::vector<ClientHandler::Client> client_list;

	private:
		std::vector<ClientHandler::Client>::iterator search_client_form_chatroom(const ClientHandler::Client& client);
		
	};
	

	//public関数
public:
	ChatroomHandler();
	~ChatroomHandler();
	bool add_chatroom(const ChatRoom& chatroom);
	bool delete_chatroom(const ChatRoom& chatroom);

	//private変数
private:
	std::vector<ChatRoom> m_chatroom_list;

	//private関数
private:
	//chatroomIDを発行する
	std::string issue_chatroom_id();
	
	



};
#endif // !CHATROOMHANDLER_H_



