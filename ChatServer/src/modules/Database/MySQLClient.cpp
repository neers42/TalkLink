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
	if (!m_connection)
		delete m_connection;

	if (!m_statement)
		delete m_statement;

	if (!m_mysql_res)
		delete m_mysql_res;

	if (!m_prep_stmt)
		delete m_prep_stmt;

}

void MySQLClient::connect_mysql()
{
	try
	{
		m_connection = m_driver->connect(_mysql_host, _mysql_user_name, _mysql_password);
		m_statement = m_connection->createStatement();
		spdlog::info("MYSQL Database connect success.");

	}
	catch (sql::SQLException& e)
	{	
		spdlog::critical("SQL Exception Content : %s, SQL Error Code : %d, SQL State : %s", e.what(), e.getErrorCode(), e.getSQLState());
		exit(1);
	}
}

