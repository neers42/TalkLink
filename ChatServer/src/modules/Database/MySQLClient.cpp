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
	try
	{
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("CREATE DATABASE IF NOT EXISTS " + _mysql_db_name)
		);
		pre_stmt->executeUpdate();
	}
	catch (const sql::SQLException& e)
	{
		LOG_ERROR("MySQL:database creation failed. Error: {}", e.what());
		return;
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
	else if (this->create_table_unsend_messages() == false) {
		return false;
	}
	return true;
}

bool MySQLClient::insert_client_data(
	const std::string& cid,
	const std::string& name, 
	const std::string& mail_address,
	const std::string& password_hash, 
	const std::string& created_at, 
	const std::string& last_login
)
{
	try
	{
		std::unique_ptr < sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("INSERT INTO " + _mysql_table_name_clients +
				"(cid, name, mail_address, password_hash, created_at, last_login) "
				"VALUES(?, ?, ?, ?, ?, ?);")
		);
		pre_stmt->setString(1, cid);
		pre_stmt->setString(2, name);
		pre_stmt->setString(3, mail_address);
		pre_stmt->setString(4, password_hash);
		pre_stmt->setString(5, created_at);
		pre_stmt->setString(6, last_login);
		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:insert client data success.");
		return true;
	}
	catch (const sql::SQLException& e)
	{
		LOG_ERROR("MySQL:insert client data failed. Error: {}", e.what());
		return false;

	}
	

	
}

std::tuple<std::string, std::string, std::string>  MySQLClient::select_client_info(const std::string& cid)
{
	try
	{
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("SELECT * FROM " + _mysql_table_name_clients + " WHERE cid = ?;")
		);
		pre_stmt->setString(1, cid);
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());

		if (res != nullptr && res->next()) {
			std::string name = res->getString("name");
			std::string mail_address = res->getString("mail_address");
			std::string password_hash = res->getString("password_hash");
			spdlog::debug("MySQL:client name:{:s}, mail_address:{:s}, password_hash:{:s}", name, mail_address, password_hash);
			return std::make_tuple(name, mail_address, password_hash);
		}
		else {
			LOG_ERROR("MySQL: no data found for cid: {}", cid);
			return std::make_tuple("", "", "");
		}
	}
	catch (const sql::SQLException& e)
	{
		LOG_ERROR("MySQL:select client data failed. Error: {}", e.what());
		return std::make_tuple("", "", "");

	}
}

bool MySQLClient::update_last_login(const std::string& cid, const std::string& last_login)
{
	try
	{
		if (cid == "") {
			LOG_ERROR("MySQL:client id is empty.Skip updating client last login time.");
			return false;
		}
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("UPDATE " + _mysql_table_name_clients +
				" SET last_login = ? WHERE cid = ?;")
		);
		pre_stmt->setString(1, last_login);
		pre_stmt->setString(2, cid);
		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:update last login(cid : {}) success.", cid);
		return true;

	}
	catch (const std::exception&)
	{
		LOG_ERROR("MySQL:update last login failed.");
		return false;
	}
	catch (const sql::SQLException& e)
	{
		LOG_ERROR("MySQL:update last login failed. Error: {}", e.what());
		return false;
	}
	catch (const std::runtime_error& e) {
		LOG_ERROR("MySQL:update last login failed. Error: {}", e.what());
		return false;
	}
}

bool MySQLClient::is_client_id_exist(const std::string& cid)
{
	try
	{
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("SELECT EXISTS (SELECT 1 FROM " + _mysql_table_name_clients + " WHERE cid = ?);")
		);
		pre_stmt->setString(1, cid);
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());
		if (res != nullptr && res->next()) {
			int is_exist = res->getBoolean(1);
			if (is_exist == 1) {
				spdlog::debug("MySQL:client id exists.");
				return true;
			}
			else {
				spdlog::debug("MySQL:client id does not exist.");
				return false;
			}
		}
		else {
			LOG_ERROR("MySQL: no data found for cid: {}", cid);
			return false;
		}
	}
	catch (const sql::SQLException& e)
	{
		LOG_ERROR("MySQL:select client data failed. Error: {}", e.what());
		return false;
	}
	catch (const std::exception& e){
		LOG_ERROR("MySQL:select client data failed. Error: {}", e.what());
		return false;
	}
	catch (const std::runtime_error& e){
		LOG_ERROR("MySQL:select client data failed. Error: {}", e.what());
		return false;
	}
}

bool MySQLClient::is_room_id_exist(const std::string& room_id)
{
	try
	{
		std::vector<uint8_t> decode = base64::decode(room_id);
		if (decode.size() != 16) {
			throw std::runtime_error("Invalid room_id size");
		}
		ChatRoomID decoded_room_id;
		std::copy(decode.begin(), decode.end(), decoded_room_id.begin());

		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("SELECT EXISTS (SELECT 1 FROM " + _mysql_table_name_chatrooms + " WHERE room_id = ?);")
		);
		//std::string binary_str(reinterpret_cast<const char*>(room_id.data()), room_id.size());
		//std::istringstream binary_stream(binary_str);
		//pre_stmt->setBlob(1, reinterpret_cast<const char*>(room_id.data()), room_id.size());
		//pre_stmt->setString(1, &binary_stream);
		std::string binary_str(reinterpret_cast<const char*>(room_id.data()), room_id.size());
		std::unique_ptr<std::istringstream> binary_stream = std::make_unique<std::istringstream>(binary_str);
		pre_stmt->setBlob(1, binary_stream.get());
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());
		if (res != nullptr && res->next()) {
			int is_exist = res->getBoolean(1);
			if (is_exist == 1) {
				spdlog::debug("MySQL:room id({}) exists.", room_id);
				return true;
			}
			else {
				spdlog::debug("MySQL:room id({}) does not exist.", room_id);
				return false;
			}
		}
		else {
			LOG_ERROR("MySQL: no data found for room_id(response is null): {}", room_id);
			return false;
		}
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR("MySQL:Failed to decode room id. Error: {}", e.what());
		return false;
	}
	catch (const sql::SQLException& e)
	{
		LOG_ERROR("MySQL:select room id failed. Error: {}", e.what());
		return false;
	}
}

bool MySQLClient::insert_chat_message(const uint64_t mid, const std::string& room_id, const std::string& sender_id, const std::string& send_time, const std::string& content)
{
	try
	{
		std::unique_ptr <sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("INSERT INTO " + _mysql_table_name_chat_messages +
				"(mid, room_id, sender_id, send_time, `message`) "
				"VALUES(?, ?, ?, ?, ?);")
		);
		pre_stmt->setInt64(1, mid);
		pre_stmt->setString(2, room_id);
		pre_stmt->setString(3, sender_id);
		pre_stmt->setString(4, send_time);
		pre_stmt->setString(5, content);

		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:insert chat message success.");
		return true;
	}
	catch (const sql::SQLException& e)
	{
		LOG_ERROR("MySQL:insert chat message failed. Error: {}", e.what());
		return false;
	}
}

bool MySQLClient::insert_unsend_message(const uint64_t mid, const std::string& cid, const std::string& room_id)
{
	try
	{
		std::unique_ptr <sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("INSERT INTO " + _mysql_table_name_unsend_messages +
				"(mid, cid, room_id) "
				"VALUES(?, ?, ?);")
		);
		pre_stmt->setInt64(1, mid);
		pre_stmt->setString(2, cid);
		pre_stmt->setString(3, room_id);
		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:insert unsend message(message id:{}, cid:{}, room_id:{}) success.", mid, cid, room_id);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		LOG_ERROR("MySQL:insert unsend message failed. Error: {}", e.what());
		return false;
	}
}

bool MySQLClient::insert_chatroom(const std::string& room_id, const std::string& room_name)
{
	return false;
}

bool MySQLClient::insert_chatroom_member(const std::string& room_id, const std::string& cid)
{
	return false;
}



std::string MySQLClient::get_client_last_login_fromDB(const std::string& cid)
{
	std::string last_login_time = "";

	try {
		// クエリ: クライアントIDから最終ログイン日時を取得
		std::string query = "SELECT last_login FROM " + _mysql_table_name_clients + " WHERE cid = ?";

		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement(query)
		);

		// パラメータを設定
		pre_stmt->setString(1, cid); // クライアントID

		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());

		// 結果を読み取る
		if (res->next()) {
			last_login_time = res->getString("last_login");
		}

		spdlog::debug("MySQL: Fetched last login time for client {}: {}", cid, last_login_time);

	}
	catch (const sql::SQLException& e) {
		LOG_ERROR("MySQL: Failed to fetch last login time. Error: {}", e.what());
	}

	return last_login_time;
}

json MySQLClient::get_unsend_message_fromDB(const std::string& cid, int limit)
{
	try
	{
		// 最終ログイン日時を取得
		std::string last_login_time = get_client_last_login_fromDB(cid);
		if (last_login_time.empty()) {
			LOG_ERROR("MySQL: No last login time found for client {}", cid);
			return json();
		}
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement(
				"SELECT unsend_messages.mid, unsend_messages.cid, unsend_messages.room_id, chat_messages.message "
				"FROM " + _mysql_table_name_unsend_messages + " "
				"JOIN chat_messages ON unsend_messages.mid = chat_messages.mid "
				"WHERE unsend_messages.cid = ? AND chat_messages.send_time > ?"
			)
		);
		pre_stmt->setString(1, cid);
		pre_stmt->setString(2, last_login_time);
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());
		//未読メッセージの配列を作成
		json unsend_messages_json = json::array();
		while (res -> next())
		{
			json message;
			message["mid"] = res->getInt64("mid");
			message["cid"] = res->getString("cid");
			message["room_id"] = res->getString("room_id");
			message["message"] = res->getString("message");
			unsend_messages_json.push_back(message);

		}
		spdlog::debug("MySQL: Unsend messages fetched successfully for client {}", cid);
		return unsend_messages_json;
	}
	catch (const sql::SQLException& e)
	{
		LOG_ERROR("MySQL: Failed to fetch unsend messages. Error: {}", e.what());
		return json();
	}
}

std::vector<std::string> MySQLClient::get_client_list_from_chatroom(const std::string& room_id)
{
	try
	{
		std::vector<std::string> client_list;
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("SELECT cid FROM " + _mysql_table_name_chat_members + " WHERE room_id = ?;")
		);
		pre_stmt->setString(1, room_id);
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());

		while (res->next())
		{
			std::string cid = res->getString("cid");
			client_list.push_back(cid);
		}
		return client_list;
	}
	catch (const sql::SQLException& e)
	{
		LOG_ERROR("MySQL: Failed to fetch client list from chatroom. Error: {}", e.what());
		return std::vector<std::string>();
	}
}

std::string MySQLClient::get_chatroom_name_fromDB(const std::string& room_id)
{
	try
	{
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("SELECT room_name FROM " + _mysql_table_name_chatrooms + " WHERE room_id = ?;")
		);
		pre_stmt->setString(1, room_id);
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());
		if (res != nullptr && res->next()) {
			std::string room_name = res->getString("room_name");
			spdlog::debug("MySQL:chatroom name:{:s}", room_name);
			return room_name;
		}
		else {
			LOG_ERROR("MySQL: no data found for room_id: {}", room_id);
			return std::string();
		}
	}
	catch (const sql::SQLException& e)
	{
		LOG_ERROR("MySQL:select chatroom name failed. Error: {}", e.what());
		return std::string();
	}
}

bool MySQLClient::create_table_clients()
{
	try
	{
		std::unique_ptr < sql::PreparedStatement > pre_stmt(
			this->_connection->prepareStatement("CREATE TABLE IF NOT EXISTS " + _mysql_table_name_clients +
				"(cid VARCHAR(28) NOT NULL PRIMARY KEY,"
				"name VARCHAR(28) NOT NULL,"
				"mail_address VARCHAR(256) NOT NULL,"
				"password_hash VARCHAR(255) NOT NULL,"
				"created_at DATETIME NOT NULL,"
				"last_login DATETIME);")
		);
		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:create table(table name:{:s}) success.", _mysql_table_name_clients);
	}
	catch (const sql::SQLException& e)
	{
		LOG_ERROR("MySQL:create table(table name:{:s}) failed. Error: {}", _mysql_table_name_clients, e.what());
		return false;
	}
}

bool MySQLClient::create_table_chatrooms()
{
	try
	{
		std::unique_ptr < sql::PreparedStatement > pre_stmt(
			this->_connection->prepareStatement("CREATE TABLE IF NOT EXISTS " + _mysql_table_name_chatrooms +
				"(room_id BINARY(16) NOT NULL,"
				"room_name VARCHAR(50) NOT NULL,"
				"created_at DATETIME,"
				"PRIMARY KEY (room_id));")
		);
		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:create table(table name:{:s}) success.", _mysql_table_name_chatrooms);
	}
	catch (const sql::SQLException& e)
	{
		LOG_ERROR("MySQL:create table(table name:{:s}) failed. Error: {}", _mysql_table_name_chatrooms, e.what());
		return false;
	}
}

bool MySQLClient::create_table_chat_members()
{
	try
	{
		std::unique_ptr < sql::PreparedStatement > pre_stmt(
			this->_connection->prepareStatement("CREATE TABLE IF NOT EXISTS " + _mysql_table_name_chat_members +
				"(room_id BINARY(16) NOT NULL,"
				"cid VARCHAR(28) NOT NULL,"
				"joined_at DATETIME,"
				"PRIMARY KEY(room_id, cid),"
				"FOREIGN KEY(room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE, "
				"FOREIGN KEY (cid) REFERENCES clients(cid) ON DELETE CASCADE);")
		);
		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:create table(table name:{:s}) success.", _mysql_table_name_chat_members);
	}
	catch (const sql::SQLException& e)
	{
		LOG_ERROR("MySQL:create table(table name:{:s}) failed. Error: {}", _mysql_table_name_chat_members, e.what());
		return false;
	}
}

bool MySQLClient::create_table_chat_messages()
{
	try
	{
		std::unique_ptr < sql::PreparedStatement > pre_stmt(
			this->_connection->prepareStatement("CREATE TABLE IF NOT EXISTS " + _mysql_table_name_chat_messages +
				"(mid BIGINT PRIMARY KEY,"
				"room_id BINARY(16) NOT NULL,"
				"sender_id VARCHAR(28) NOT NULL,"
				"send_time DATETIME NOT NULL,"
				"message TEXT NOT NULL,"
				"FOREIGN KEY (room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE,"
				"FOREIGN KEY (sender_id) REFERENCES clients(cid) ON DELETE CASCADE);")
		);
		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:create table(table name:{:s}) success.", _mysql_table_name_chat_messages);
	}
	catch (const sql::SQLException& e)
	{
		LOG_ERROR("MySQL:create table(table name:{:s}) failed. Error: {}", _mysql_table_name_chat_messages, e.what());
		return false;
	}
}

bool MySQLClient::create_table_unsend_messages()
{
	try
	{
		std::unique_ptr < sql::PreparedStatement > pre_stmt(
			this->_connection->prepareStatement("CREATE TABLE IF NOT EXISTS " + _mysql_table_name_unsend_messages +
				"(mid BIGINT NOT NULL,"
				"cid VARCHAR(28) NOT NULL,"
				"room_id BINARY(16) NOT NULL,"
				"PRIMARY KEY(mid, cid, room_id),"
				"FOREIGN KEY (mid) REFERENCES chat_messages(mid) ON DELETE CASCADE ON UPDATE CASCADE,"
				"FOREIGN KEY (cid) REFERENCES clients(cid) ON DELETE CASCADE,"
				"FOREIGN KEY (room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE);")
		);
		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:create table(table name:{:s}) success.", _mysql_table_name_unsend_messages);
	}
	catch (const sql::SQLException& e)
	{
		LOG_ERROR("MySQL:create table(table name:{:s}) failed. Error: {}", _mysql_table_name_unsend_messages, e.what());
		return false;
	}
}

