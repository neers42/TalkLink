#ifndef MYSQLCLIENT_HPP
#define MYSQLCLIENT_HPP
#include "Utils/common.hpp"
#include "MySQLHelper.hpp"

using ChatRoomID = std::array<uint8_t, 16>;
class MySQLClient
{
private:
	std::unique_ptr<MySQLHelper> _mysql_helper;
	std::string _mysql_host;
    std::string _mysql_user_name;
    std::string _mysql_password;
    std::string _mysql_db_name;
	std::string _mysql_table_name_clients = "clients";
	std::string _mysql_table_name_chatrooms = "chatrooms";
	std::string _mysql_table_name_chat_members = "chat_members";
	std::string _mysql_table_name_messages = "messages";
	std::string _mysql_table_name_unsend_messages = "unsend_messages";
	std::string _mysql_table_name_unsend_control_messages = "unsend_control_messages";


public:
	MySQLClient();
	~MySQLClient();


	void create_database();
	bool create_table();


	//テーブル：chat_members操作
	bool insert_chatroom_member(const std::string& room_id, const std::vector<std::string>& cids, const std::string& joined_at);
	bool delete_chatroom_member(const std::string& room_id, const std::vector<std::string>& cids);
	std::vector<std::string> get_client_list_from_chatroom(const std::string& room_id);

	//テーブル：chat_messages操作
	bool insert_message(const uint64_t mid, const std::string& room_id, const std::string& sender_cid, const std::string& send_time, const std::string& message_content);
	json get_unreceived_messages(const std::string& cid, const uint64_t last_received_message_id);

	//テーブル：chatrooms操作
	bool is_room_id_exist(const std::string& room_id);
	bool is_room_id_hex_exist(const std::string& room_id_hex);
	bool insert_chatroom(const std::string& room_id, const std::string& room_name, const std::string& icon_url, const std::string& created_at);
	std::string get_chatroom_name_fromDB(const std::string& room_id);
	bool delete_chatroom_fromDB(const std::string& room_id);
	bool update_chatroom_name(const std::string& room_id, const std::string& room_name);
	bool update_chatroom_icon(const std::string& room_id, const std::string& icon_url);

	//テーブル：clients操作
	bool insert_client_data(
		const std::string& cid,
		const std::string& name,
		const std::string& mail_address,
		const std::string& icon_url,
		const std::string& created_at,
		const std::string& last_login);
	std::tuple<std:: string, std::string, std::string> get_client_info_fromDB(const std::string& cid);
	bool update_last_login(const std::string& cid, const std::string& last_login);
	bool is_client_id_exist(const std::string& cid);
	std::string get_client_last_login_fromDB(const std::string& cid);

	//テーブル：unsend_control_messages操作
	bool insert_unsend_control_message(const uint64_t mid, const std::string& cid, const std::string& room_id, const std::string& json_payload, const std::string& request_time);
	json get_unsend_control_message_fromDB(const std::string& cid);
	bool delete_unsend_control_message(const uint64_t mid, const std::string& cid, const std::string& room_id);


	//テーブル：unsend_messages操作
	bool insert_unsend_message(const uint64_t mid, const std::string& cid, const std::string& room_id);
	json get_unsend_message_fromDB(const std::string& cid, int limit);
	bool delete_unsend_message(const uint64_t mid, const std::string& cid, const std::string& room_id);
	
	
private:
	bool create_table_clients();
	bool create_table_chatrooms();
	bool create_table_chat_members();
	bool create_table_messages();
	bool create_table_unsend_messages();
	bool create_table_unsend_control_messages();
	std::vector<uint8_t> hex_to_binary(const std::string& hex_str);
	std::string to_hex_string(const std::string& bin_str);

};



#endif // !MYSQLCLIENT_HPP

