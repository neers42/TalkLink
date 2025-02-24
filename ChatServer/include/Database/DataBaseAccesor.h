#ifndef DATABASEACCESSOR_H_
#define DATABASEACCESSOR_H_
#include "common.h"
#include <sstream>
#include <memory>
#include <mysql/jdbc.h>
#include <jdbc/cppconn/exception.h>



class DataBaseAccesor
{
private:
	sql::Driver* m_driver = sql::mysql::get_mysql_driver_instance();
	sql::Connection* m_connection = nullptr;
	sql::Statement* m_statement = nullptr;
	sql::ResultSet* m_mysql_res = nullptr;
	sql::mysql::MySQL_Prepared_Statement* m_prep_stmt = nullptr;
	
	std::string m_host = "tcp://127.0.0.1:3306";
	std::string m_user_name = "root";
	std::string m_password = "***REMOVED***";
	std::string m_db_name = "";
	std::string m_port = "";


public:
	DataBaseAccesor();
	DataBaseAccesor(const std::string& host, const std::string& user_name, const std::string& password);

	~DataBaseAccesor();

	void connect_db();
};



#endif // !DATABASEACCESSOR_H_

