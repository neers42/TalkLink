#ifndef MYSQL_HELPER_HPP
#define MYSQL_HELPER_HPP
#include "common.hpp"
#include <mysql/jdbc.h>

class MySQLHelper {
public:
    MySQLHelper();
    MySQLHelper(const std::string& host, const std::string& user_name, const std::string& password, const std::string& db_name);
    ~MySQLHelper();

    bool connect(const std::string& db_name);
    void disconnect();

    bool executeQuery(const std::string& query);
    void printError(const sql::SQLException& e);
    sql::Connection* get_connection() const { return _connection; }

private:
    sql::Driver* _driver = sql::mysql::get_mysql_driver_instance();
	sql::Connection* _connection = nullptr;
    sql::Statement* _statement = nullptr;
	sql::mysql::MySQL_Prepared_Statement* _prep_stmt = nullptr;
	std::string _mysql_host;
	std::string _mysql_user_name;
	std::string _mysql_password;
	std::string _mysql_db_name;
};

#endif // MYSQL_HELPER_HPP