#ifndef DATABASEACCESSOR_H_
#define DATABASEACCESSOR_H_
#include "common.h"
#include <sstream>
#include <memory>
#include <mysql/jdbc.h>
#include <jdbc/cppconn/exception.h>
#include "spdlog/spdlog.h"
#include "DataBaseInterface.h"

using ChatRoomID = std::array<uint8_t, 16>;
class MySQLClient : public DataBaseInterface
{
private:
	sql::Driver* _driver = sql::mysql::get_mysql_driver_instance();
	sql::Connection* _connection = nullptr;
	sql::Statement* _statement = nullptr;
	sql::ResultSet* _mysql_res = nullptr;
	sql::mysql::MySQL_Prepared_Statement* m_prep_stmt = nullptr;
	
	std::string _mysql_host = "tcp://127.0.0.1:3306";
	std::string _mysql_user_name = "root";
	std::string _mysql_password = "***REMOVED***";
	std::string _mysql_db_name = "MyChat";
	std::string _mysql_port = "";
	std::string _mysql_table_name_clients = "clients";
	std::string _mysql_table_name_chatrooms = "chatrooms";
	std::string _mysql_table_name_chat_members = "chat_members";
	std::string _mysql_table_name_chat_messages = "chat_messages";
	std::string _mysql_table_name_unsend_messages = "unsend_messages";
	std::string _mysql_table_name_unsend_control_messages = "unsend_control_messages";


public:
	MySQLClient();
	MySQLClient(const std::string& host, const std::string& user_name, const std::string& password);

	~MySQLClient();


	//データベース接続及び初期化
	void connect() override;
	void close() override;
	bool execute_query(const std::string& query) override;
	void print_sql_exception(const sql::SQLException& e);
	void create_database();
	bool create_table();


	//テーブル：chat_members操作
	bool insert_chatroom_member(const std::string& room_id, const std::vector<std::string>& cids, const std::string& joined_at);
	bool delete_chatroom_member(const std::string& room_id, const std::vector<std::string>& cids);
	std::vector<std::string> get_client_list_from_chatroom(const std::string& room_id);

	//テーブル：chat_messages操作
	bool insert_chat_message(const uint64_t mid, const std::string& room_id, const std::string& sender_id, const std::string& send_time, const std::string& content);

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
		const std::string& password_hash,
		const std::string& icon_url,
		const std::string& created_at,
		const std::string& last_login);
	std::tuple<std:: string, std::string, std::string> get_client_info_fromDB(const std::string& cid);
	bool update_last_login(const std::string& cid, const std::string& last_login);
	bool is_client_id_exist(const std::string& cid);
	std::string get_client_last_login_fromDB(const std::string& cid);

	//テーブル：unsend_control_messages操作
	bool insert_unsend_control_message(const uint64_t mid, const std::string& cid, const std::string& room_id, const std::string& json_payload, const std::string& created_at);
	json get_unsend_control_message_fromDB(const std::string& cid);
	bool delete_unsend_control_message(const uint64_t mid, const std::string& cid, const std::string& room_id);


	//テーブル：unsend_messages操作
	bool insert_unsend_message(const uint64_t mid, const std::string& cid, const std::string& room_id);
	json get_unsend_message_fromDB(const std::string& cid, int limit);
	
	
private:
	bool create_table_clients();
	bool create_table_chatrooms();
	bool create_table_chat_members();
	bool create_table_chat_messages();
	bool create_table_unsend_messages();
	bool create_table_unsend_control_messages();
	std::vector<uint8_t> hex_to_binary(const std::string& hex_str);
	
};



#endif // !DATABASEACCESSOR_H_

