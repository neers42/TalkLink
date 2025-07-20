#ifndef CHATROOMHANDLER_H_
#define CHATROOMHANDLER_H_
#include <string>
#include <iostream>
#include <vector>
#include "ClientHandler.h"
#include "AppException.h"
#include "StringFormat.h"
#include "ServerTime.h"
#include "ChatRoom.h"
#define MAX_ACTIVE_CHATROOM_NUM 1000	//activeなチャットルームの最大数
#define MAX_CLIENT_NUM_IN_CHATROOM 1000	//チャットルームに加入できるクライアントの最大数


class ChatroomHandler
{
private:
	//public関数
public:
	ChatroomHandler();
	~ChatroomHandler();
	std::shared_ptr<ChatRoom> create_chatroom(const std::string& chatroom_name, const std::string& icon_url, const std::vector<std::string>& room_member);
	void add_chatroom(std::shared_ptr<ChatRoom> chatroom);
	void delete_chatroom(const std::string& room_id);
	bool add_member_to_chatroom(const std::string& room_id, const std::vector<std::string>& cids);
	bool delete_member_from_chatroom(const std::string& room_id, const std::vector<std::string>& cids);
	bool change_room_name(const std::string& room_id, const std::string& room_name);
	bool change_room_icon(const std::string& room_id, const std::string& icon_url);
	

	//getter
	std::vector<std::string> get_client_infos_by_chatroom_id(int list_num);
	std::vector<std::string> get_client_infos_by_chatroom_id(const std::string& room_id);
	std::vector<std::shared_ptr<ChatRoom>> get_chatrooms_snapshot();
	int search_chatroom_id_from_list(const std::string& chatroom_id);
	std::vector<std::shared_ptr<ChatRoom>>::iterator find_chatroom_id_from_list(const std::string& room_id);
	std::vector<std::string> get_client_list_from_chatroom(const std::string& room_id);
	//private関数
private:
	//chatroomIDを発行する
	static std::string issue_chatroom_id();
	static std::string issue_chatroom_id_unhex();


private:	//private変数
	std::vector<std::shared_ptr<ChatRoom>> chatroom_list;
	
public:		//public変数
	//チャットルームの操作を保護するためのミューテックス
	mutable std::shared_mutex shared_mtx; // for read/write lock




};
#endif // !CHATROOMHANDLER_H_



