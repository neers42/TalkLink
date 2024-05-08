#include "DataBaseAccesor.h"

DataBaseAccesor::DataBaseAccesor()
{
}

DataBaseAccesor::~DataBaseAccesor()
{
	if (!m_mysql_res)
		delete m_mysql_res;

	if (!m_mysql_stmt)
		delete m_mysql_res;

	if (!m_prep_stmt)
		delete m_prep_stmt;

	if (!m_mysql_con)
		delete m_mysql_con;

}

void DataBaseAccesor::connect_db()
{
	try
	{
		//std::unique_ptr<sql::Connection> connection(m_mysql_driver->connect())
	}
	catch (const std::exception&)
	{

	}
}

