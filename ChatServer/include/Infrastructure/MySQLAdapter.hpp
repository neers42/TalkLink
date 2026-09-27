#ifndef MYSQLADAPTER_HPP
#define MYSQLADAPTER_HPP
#include "Utils/common.hpp"
#include "Database/MySQLHelper.hpp"
#include "Domain/Interfaces/IChatroomRepository.hpp"
#include "Domain/Interfaces/IMessageRepository.hpp"
#include "Domain/Interfaces/IUserRepository.hpp"
#include "Domain/DTO/UserInfo.hpp"
#include "ServerTime.hpp"

using ChatRoomID = std::array<uint8_t, 16>;

class MySQLAdapter
	: public IChatroomRepository,
	  public IMessageRepository,
	  public IUserRepository
{
private:
	std::unique_ptr<MySQLHelper> _mysql_helper;
	mutable std::recursive_mutex _mysql_mutex; // 共有 sql::Connection はスレッドセーフでないため全DB操作を直列化する（再入可）
	std::string _mysql_host;
    std::string _mysql_user_name;
    std::string _mysql_password;
    std::string _mysql_db_name;
	std::string _mysql_table_name_users = "users";
	std::string _mysql_table_name_chatrooms = "chatrooms";
	std::string _mysql_table_name_chat_members = "chat_members";
	std::string _mysql_table_name_messages = "messages";
	std::string _mysql_table_name_unsend_messages = "unsend_messages";
	std::string _mysql_table_name_unsend_control_messages = "unsend_control_messages";


public:
	MySQLAdapter();
	~MySQLAdapter() override;


	void create_database();
	bool create_table();


	//テーブル：chat_members操作
	bool insert_chatroom_member(const std::string& room_id, const std::vector<std::string>& uids, const std::string& joined_at);
	MemberInsertResult insert_chatroom_members_within_limit(const std::string& room_id, const std::vector<std::string>& uids, const std::string& joined_at, int max_members) override;
	bool delete_chatroom_member(const std::string& room_id, const std::vector<std::string>& uids);
	std::vector<std::string> get_member_uids(const std::string& room_id);

	//テーブル：chat_messages操作
	bool insert_message(const uint64_t mid,
		const std::string& room_id,
		const std::string& sender_uid,
		const std::string& send_time,
		const std::string& message_content,
		std::uint64_t room_sequence = 0,
		const std::string& target_uid = "") override;

	json get_unreceived_messages(const std::string& uid, const uint64_t last_received_message_id);

	//テーブル：chatrooms操作
	bool chatroom_exists(const std::string& room_id);
	bool chatroom_exists_by_hex(const std::string& room_id_hex);
	std::string find_direct_room_by_members(const std::string& uid_a, const std::string& uid_b);

	bool insert_chatroom(
		const std::string& room_id, 
		const std::string& room_name, 
		const std::string& icon_url, 
		const std::string& created_at) override;

	std::string get_chatroom_name(const std::string& room_id);
	bool delete_chatroom(const std::string& room_id);
	bool update_chatroom_name(const std::string& room_id, const std::string& room_name);
	bool update_chatroom_icon(const std::string& room_id, const std::string& icon_url);

	//テーブル：users操作
	bool insert_user_info_data(
		const std::string& uid,
		const std::string& name,
		const std::string& mail_address,
		const std::string& icon_url,
		const std::string& created_at,
		const std::string& last_login);
	UserInfo get_user_info(const std::string& uid) override;
	bool update_last_login(const std::string& uid, const std::string& last_login) override;
	bool user_exists(const std::string& uid);
	std::string get_user_last_login(const std::string& uid);
	std::vector<UserInfo> search_user_infos(const std::string& query);

	//テーブル：unsend_control_messages操作
	bool insert_unsend_control_message(const uint64_t mid, const std::string& uid, const std::string& room_id, const std::string& json_payload, const std::string& request_time);
	json get_unsend_control_messages(const std::string& uid);
	bool delete_unsend_control_message(const uint64_t mid, const std::string& uid, const std::string& room_id);


	//テーブル：unsend_messages操作
	bool insert_unsend_message(const uint64_t mid, const std::string& uid, const std::string& room_id);
	json get_unsend_messages(const std::string& uid, int limit);
	bool delete_unsend_message(const uint64_t mid, const std::string& uid, const std::string& room_id);
	
	
private:
	bool create_table_users();
	bool create_table_chatrooms();
	bool create_table_chat_members();
	bool create_table_messages();
	bool create_table_unsend_messages();
	bool create_table_unsend_control_messages();
	std::vector<uint8_t> hex_to_binary(const std::string& hex_str);
	std::string to_hex_string(const std::string& bin_str);

};



#endif // !MYSQLADAPTER_HPP

