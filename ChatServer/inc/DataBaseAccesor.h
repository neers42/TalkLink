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
	sql::mysql::MySQL_Driver* m_mysql_driver = sql::mysql::get_mysql_driver_instance();

public:
	DataBaseAccesor();
	~DataBaseAccesor();

	void connect_db();
};



#endif // !DATABASEACCESSOR_H_

