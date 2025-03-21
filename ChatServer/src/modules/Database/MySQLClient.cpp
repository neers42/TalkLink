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
		spdlog::info("MySQL:connect success!");

	}
	catch (sql::SQLException& e)
	{	
		spdlog::critical("SQL Exception Content : %s, SQL Error Code : %d, SQL State : %s", e.what(), e.getErrorCode(), e.getSQLState());
		exit(1);
	}
}

void MySQLClient::close()
{
	try
	{
		if (!_connection) {
			throw std::runtime_error("MySQL:database is not connected.");
		}
	}
	catch (const std::runtime_error e)
	{
		spdlog::error(e.what());
		return;
	}
	catch (const std::exception e){
		spdlog::error(e.what());
		return;
	}
	_connection->close();
}

bool MySQLClient::execute_query(const std::string& query)
{
	try
	{
		if (!_connection) {
			throw std::runtime_error("MySQL:database is not connected.");
		}
		_statement->execute(query);
	}
	catch (const std::runtime_error e)
	{
		LOG_ERROR(e.what());
		return false;
	}
	catch (const sql::SQLException e) {
		LOG_ERROR("MySQL:SQL Exception Content : %s, SQL Error Code : %d, SQL State : %s", e.what(), e.getErrorCode(), e.getSQLState());
		return false;
	}
	catch (const std::exception e) {
		LOG_ERROR(e.what());
		return false;
	}
	return true;
	
}

void MySQLClient::print_sql_error(sql::SQLException& e)
{
	spdlog::error("MySQL:SQL Exception Content : %s, SQL Error Code : %d, SQL State : %s", e.what(), e.getErrorCode(), e.getSQLState());
}

void MySQLClient::create_database()
{
	std::string query = "CREATE DATABASE IF NOT EXISTS " + _mysql_db_name;

	if (this->execute_query(query) == true) {
		spdlog::info("MySQL:database created successfully.");
	}
	else {
		spdlog::error("MySQL:database creation failed.");
	}
	
}

bool MySQLClient::create_table()
{
	if (this->create_table_clients() == false) {
		return false;
	}
	else if (this->create_table_chatrooms() == false) {
		return false;
	}
	else if (this->create_table_chat_members() == false) {
		return false;
	}
	else if (this->create_table_chat_messages() == false) {
		return false;
	}
	else if (this->create_table_unread_messages() == false) {
		return false;
	}
	return true;
}

bool MySQLClient::insert_client_data(const std::string& cid,const std::string& name,  const std::string& email, const std::string& password_hash, const std::string& created_at)
{
	std::string query = "INSERT INTO " + _mysql_table_name_clients + 
		"(cid, name, email, password_hash, created_at) "
		"VALUES('" + cid + "','"+ name + "','" + email + "','" + password_hash + "','" + created_at + "');";

	if (this->execute_query(query) == true) {
		spdlog::debug("MySQL:insert client data success.");
		return true;
	}
	else {
		LOG_ERROR("MySQL:insert client data failed.");
		//spdlog::error("MySQL:insert client data failed.");
		return false;
	}
}

std::tuple<std::string, std::string, std::string>  MySQLClient::select_client_info(const std::string& cid)
{
	std::string query = "SELECT * FROM " + _mysql_table_name_clients + " WHERE cid = '" + cid + "';";
	if (this->execute_query(query) == true) {
		spdlog::debug("MySQL:select client data success.");
		_mysql_res = _statement->getResultSet();
		while (_mysql_res->next())
		{
			if (_mysql_res != nullptr) {
				std::string name = _mysql_res->getString("name");
				std::string email = _mysql_res->getString("email");
				std::string password_hash = _mysql_res->getString("password_hash");
				spdlog::debug("MySQL:client name:{:s}, email:{:s}, password_hash:{:s}", name, email, password_hash);
				return std::make_tuple(name, email, password_hash);
			}
			else {
				LOG_ERROR("MySQL: ResultSet is null.");
				return std::make_tuple("", "", "");
			}
		}
		
	}
	else {
		LOG_ERROR("MySQL:select client data failed.");
		//spdlog::error("MySQL:select client data failed.");
		return std::make_tuple("", "", "");
	}
}

bool MySQLClient::create_table_clients()
{

	std::string query = "CREATE TABLE IF NOT EXISTS " + _mysql_table_name_clients +
		"(cid VARCHAR(255) NOT NULL PRIMARY KEY,"
		"name VARCHAR(255) NOT NULL,"
		"email VARCHAR(255) NOT NULL,"
		"password_hash VARCHAR(255) NOT NULL,"
		"created_at DATETIME NOT NULL);";

	if (this->execute_query(query) == true) {
		spdlog::debug("MySQL:create table(table name:{:s}) success.", _mysql_table_name_clients);
		return true;
	}
	else {
		spdlog::error("MySQL:create tbale(table name:{:s})  users failed.", _mysql_table_name_clients);
		return false;
	}

}

bool MySQLClient::create_table_chatrooms()
{
	std::string query = "CREATE TABLE IF NOT EXISTS " + _mysql_table_name_chatrooms + 
		"(room_id INT NOT NULL PRIMARY KEY,"
		"room_name VARCHAR(255),"
		"created_at DATETIME);";
	if (this->execute_query(query) == true) {
		spdlog::debug("MySQL:create table(table name:{:s}) success.", _mysql_table_name_chatrooms);
		return true;
	}
	else {
		spdlog::error("MySQL:create tbale(table name:{:s}) failed.", _mysql_table_name_chatrooms);
		return false;
	}
}

bool MySQLClient::create_table_chat_members()
{
	std::string query = "CREATE TABLE IF NOT EXISTS " + _mysql_table_name_chat_members +
		"(room_id INT NOT NULL,"
		"cid VARCHAR(255) NOT NULL,"
		"joined_at DATETIME,"
		"PRIMARY KEY(room_id, cid),"
		"FOREIGN KEY(room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE, "
		"FOREIGN KEY (cid) REFERENCES clients(cid) ON DELETE CASCADE);";

	if (this->execute_query(query) == true) {
		spdlog::debug("MySQL:create table(table name:{:s}) success.", _mysql_table_name_chat_members);
		return true;
	}
	else {
		spdlog::error("MySQL:create tbale(table name:{:s}) failed.", _mysql_table_name_chat_members);
		return false;
	}
}

bool MySQLClient::create_table_chat_messages()
{
	std::string query = "CREATE TABLE IF NOT EXISTS " + _mysql_table_name_chat_messages +
		"(mid INT PRIMARY KEY,"
		"room_id INT NOT NULL,"
		"sender_id VARCHAR(255) NOT NULL,"
		"message TEXT NOT NULL,"
		"created_at DATETIME NOT NULL,"
		"FOREIGN KEY (room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE,"
		"FOREIGN KEY (sender_id) REFERENCES clients(cid) ON DELETE CASCADE);";
	if (this->execute_query(query) == true){
		spdlog::debug("MySQL:create table(table name:{:s}) success.", _mysql_table_name_chat_messages);
		return true;
	}
	else {
		spdlog::error("MySQL:create tbale(table name:{:s}) failed.", _mysql_table_name_chat_messages);
		return false;
	}
}

bool MySQLClient::create_table_unread_messages()
{
	std::string query = "CREATE TABLE IF NOT EXISTS " + _mysql_table_name_unread_messages +
		"(mid INT NOT NULL,"
		"cid VARCHAR(255) NOT NULL,"
		"room_id INT NOT NULL,"
		"count INT NOT NULL,"
		"PRIMARY KEY(mid, cid, room_id),"
		"FOREIGN KEY (mid) REFERENCES chat_messages(mid) ON DELETE CASCADE ON UPDATE CASCADE,"
		"FOREIGN KEY (cid) REFERENCES clients(cid) ON DELETE CASCADE,"
		"FOREIGN KEY (room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE);";
	if (this->execute_query(query) == true) {
		spdlog::debug("MySQL:create table(table name:{:s}) success.", _mysql_table_name_unread_messages);
		return true;
	}
	else {
		spdlog::error("MySQL:create tbale(table name:{:s}) failed.", _mysql_table_name_unread_messages);
		return false;
	}
}

