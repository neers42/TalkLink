#include "MySQLClient.h"

MySQLClient::MySQLClient()
{
}

MySQLClient::MySQLClient(const std::string& host, const std::string& user_name, const std::string& password)
	:_mysql_host{host}, _mysql_user_name{user_name}, _mysql_password{password}
{
}

MySQLClient::~MySQLClient()
{
	if (!_connection)
		delete _connection;

	if (!_statement)
		delete _statement;

	if (!_mysql_res)
		delete _mysql_res;

	if (!m_prep_stmt)
		delete m_prep_stmt;

}

void MySQLClient::connect()
{
	try
	{
		_connection = _driver->connect(_mysql_host, _mysql_user_name, _mysql_password);
		_statement = _connection->createStatement();
		spdlog::info("MYSQL Database connect success.");

	}
	catch (sql::SQLException& e)
	{	
		spdlog::critical("SQL Exception Content : %s, SQL Error Code : %d, SQL State : %s", e.what(), e.getErrorCode(), e.getSQLState());
		exit(1);
	}
}

void MySQLClient::close()
{

}

