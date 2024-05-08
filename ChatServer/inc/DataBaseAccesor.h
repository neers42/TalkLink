#ifndef DATABASEACCESSOR_H_
#define DATABASEACCESSOR_H_
#include "common.h"
#include <sstream>
#include <memory>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <mysqlx/devapi/error.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <mysql/jdbc.h>



class DataBaseAccesor
{
private:
	sql::mysql::MySQL_Driver* m_mysql_drv = sql::mysql::get_mysql_driver_instance();
	sql::mysql::MySQL_Connection* m_mysql_con;
	sql::mysql::MySQL_Statement* m_mysql_stmt;
	sql::ResultSet* m_mysql_res;
	sql::mysql::MySQL_Prepared_Statement* m_prep_stmt;
	
	std::string m_host;
	std::string m_db_name;
	std::string m_port;
	std::string m_user;
	std::string m_password;

public:
	DataBaseAccesor();
	~DataBaseAccesor();

	void connect_db();
};



#endif // !DATABASEACCESSOR_H_

