#ifndef DATABASEACCESSOR_H_
#define DATABASEACCESSOR_H_
#include "common.h"
#include <sstream>
#include <memory>
#include <mysql/jdbc.h>
#include <jdbc/cppconn/exception.h>
#include "spdlog/spdlog.h"
#include "DataBaseInterface.h"


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


public:
	MySQLClient();
	MySQLClient(const std::string& host, const std::string& user_name, const std::string& password);

	~MySQLClient();

	void connect() override;
	void close() override;
	bool execute_query(const std::string& query) override;
	void print_sql_error(sql::SQLException& e);
	void create_database();
	bool create_table();
	bool insert_client_data(const std::string& cid, const std::string& name, const std::string& mail_address, const std::string& password_hash, const std::string& created_at, const std::string& last_login);
	std::tuple<std:: string, std::string, std::string> select_client_info(const std::string& cid);
	bool update_last_login(const std::string& cid, const std::string& last_login);
	bool is_client_id_exist(const std::string& cid);
	bool is_room_id_exist(const std::string& room_id);
	bool insert_chat_message(const uint64_t mid, const std::string& room_id, const std::string& sender_id, const std::string& send_time, const std::string& content);
	bool insert_unsend_message(const uint64_t mid, const std::string& cid, const std::string& room_id);
	bool insert_chatroom(const std::string& room_id, const std::string& room_name);
	bool insert_chatroom_member(const std::string& room_id, const std::string& cid);
	std::string get_client_last_login_fromDB(const std::string& cid);
	json get_unsend_message_fromDB(const std::string& cid, int limit);
	std::vector<std::string> get_client_list_from_chatroom(const std::string& room_id);
	std::string get_chatroom_name_fromDB(const std::string& room_id);
private:
	bool create_table_clients();
	bool create_table_chatrooms();
	bool create_table_chat_members();
	bool create_table_chat_messages();
	bool create_table_unsend_messages();
	
};



#endif // !DATABASEACCESSOR_H_

