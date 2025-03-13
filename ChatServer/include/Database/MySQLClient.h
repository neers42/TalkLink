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
	sql::Driver* m_driver = sql::mysql::get_mysql_driver_instance();
	sql::Connection* m_connection = nullptr;
	sql::Statement* m_statement = nullptr;
	sql::ResultSet* m_mysql_res = nullptr;
	sql::mysql::MySQL_Prepared_Statement* m_prep_stmt = nullptr;
	
	std::string _mysql_host = "tcp://127.0.0.1:3306";
	std::string _mysql_user_name = "root";
	std::string _mysql_password = "***REMOVED***";
	std::string _mysql_db_name = "";
	std::string _mysql_port = "";


public:
	MySQLClient();
	MySQLClient(const std::string& host, const std::string& user_name, const std::string& password);

	~MySQLClient();

	void connect_mysql();
};



#endif // !DATABASEACCESSOR_H_

