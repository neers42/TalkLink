#ifndef CHATROOM_HPP
#define CHATROOM_HPP
#include "Utils/common.hpp"
class Chatroom
{

private:
	std::string _chatroom_id = "";
	std::string _chatroom_name = "";
	std::string _chatroom_icon_url = "";
	std::vector<std::string> _uid_list;

public:
	mutable std::mutex mtx;

public:
	//コンストラクタ
	Chatroom();
	Chatroom(
		const std::string& chatroom_id,
		const std::string& chatroom_name,
		const std::string& icon_url,
		const std::vector<std::string>& uids
	);


	//デストラクタ
	~Chatroom();

	//==のオーバーロード
	bool operator==(const Chatroom& other) const
	{
		return this->_chatroom_id == other._chatroom_id &&
			this->_chatroom_name == other._chatroom_name &&
			this->_chatroom_icon_url == other._chatroom_icon_url &&
			this->_uid_list == other._uid_list;
	}

	//クライアントルームにクライアントを加入する
	bool add_user_chatroom(const std::string& uid);

	//クライアントルームからクライアントを削除する
	bool delete_user_chatroom(const std::string& uid);

	std::string& get_chatroom_id();
	const std::string& get_chatroom_id() const;

	std::string& get_chatroom_name();
	const std::string& get_chatroom_name() const;

	std::string& get_chatroom_icon();
	const std::string& get_chatroom_icon() const;

	std::vector<std::string>& get_users();
	const std::vector <std::string>& get_users() const;

	void set_chatroom_name(const std::string& chatroom_name);
	void set_chatroom_icon(const std::string& icon_url);



private:
	std::vector<std::string>::iterator search_user_from_chatroom(const std::string& uid);

};
#endif // CHATROOM_HPP

