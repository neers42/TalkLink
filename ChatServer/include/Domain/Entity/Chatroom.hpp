#ifndef CHATROOM_HPP
#define CHATROOM_HPP
#include "Utils/common.hpp"
class Chatroom
{

private:
	std::string _chatroom_id = "";
	std::string _chatroom_name = "";
	std::string _chatroom_icon_url = "";
	std::vector<std::string> _cid_list;

public:
	mutable std::mutex mtx;

public:
	//コンストラクタ
	Chatroom();
	Chatroom(
		const std::string& chatroom_id,
		const std::string& chatroom_name,
		const std::string& icon_url,
		const std::vector<std::string>& cids
	);


	//デストラクタ
	~Chatroom();

	//==のオーバーロード
	bool operator==(const Chatroom& other) const
	{
		return this->_chatroom_id == other._chatroom_id &&
			this->_chatroom_name == other._chatroom_name &&
			this->_chatroom_icon_url == other._chatroom_icon_url &&
			this->_cid_list == other._cid_list;
	}

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

	void set_chatroom_name(const std::string& chatroom_name);
	void set_chatroom_icon(const std::string& icon_url);



private:
	std::vector<std::string>::iterator search_client_from_chatroom(const std::string& cid);

};
#endif // CHATROOM_HPP

