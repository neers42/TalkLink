#include "DataBaseAccesor.h"

DataBaseAccesor::DataBaseAccesor()
{
}

DataBaseAccesor::DataBaseAccesor(const std::string& host, const std::string& user_name, const std::string& password)
	:m_host{host}, m_user_name{user_name}, m_password{password}
{
}

DataBaseAccesor::~DataBaseAccesor()
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

void DataBaseAccesor::connect_db()
{
	try
	{
		m_connection = m_driver->connect(m_host, m_user_name, m_password);
		m_statement = m_connection->createStatement();
		LogHandler::get_instance().logging_info(StringFormat("MYSQL, success, Database connect."));

	}
	catch (sql::SQLException& e)
	{	
		LogHandler::get_instance().logging_critical(StringFormat("SQL Exception Content : %s, SQL Error Code : %d, SQL State : %s", e.what(), e.getErrorCode(), e.getSQLState()));
		exit(1);
	}
}

