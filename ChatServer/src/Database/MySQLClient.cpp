#include "Database/MySQLClient.h"


MySQLClient::MySQLClient()
{

}

MySQLClient::MySQLClient(const std::string& host, const std::string& user_name, const std::string& password)
	:_mysql_host{host}, _mysql_user_name{user_name}, _mysql_password{password}
{
}

MySQLClient::~MySQLClient()
{
	if (_connection)
		delete _connection;

	if (_statement)
		delete _statement;

	if (_mysql_res)
		delete _mysql_res;

	if (_prep_stmt)
		delete _prep_stmt;

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
		spdlog::critical("SQL Exception Content : {}, SQL Error Code : {}, SQL State : {}", e.what(), e.getErrorCode(), e.getSQLState());
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
	catch (const std::runtime_error& e)
	{

		spdlog::error(e.what());
		LOG_ERROR("Runtime Error:{}", e.what());
		return;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
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

	catch (const sql::SQLException& e) {
		LOG_ERROR("SQLException: {}, SQL Error Code : {}, SQL State : {}", e.what(), e.getErrorCode(), e.getSQLState());
		return false;
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
	return true;
	
}

void MySQLClient::print_sql_exception(const sql::SQLException& e)
{
	LOG_ERROR("SQLException Content:{}, SQL Error Code:{}, SQL State:{}", e.what(), e.getErrorCode(), e.getSQLState());
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
		this->print_sql_exception(e);
		return;
	}	
	catch (const std::runtime_error& e) {
		LOG_ERROR("Runtime Error:{}", e.what());
		return;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return ;
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
	else if (this->create_table_unsend_control_messages() == false) {
		return false;
	}
	else {
		return true;
	}
}

bool MySQLClient::insert_client_data(
	const std::string& cid,
	const std::string& name, 
	const std::string& mail_address,
	const std::string& password_hash,
	const std::string& icon_url,
	const std::string& created_at, 
	const std::string& last_login
)
{
	try
	{
		if (last_login == "" && icon_url == "") {
			std::unique_ptr < sql::PreparedStatement> pre_stmt(
				this->_connection->prepareStatement("INSERT INTO " + _mysql_table_name_clients +
					"(cid, name, mail_address, password_hash, icon_url, created_at, last_login) "
					"VALUES(?, ?, ?, ?, NULL, ?, NULL);")
			);
			pre_stmt->setString(1, cid);
			pre_stmt->setString(2, name);
			pre_stmt->setString(3, mail_address);
			pre_stmt->setString(4, password_hash);
			pre_stmt->setString(6, created_at);
			pre_stmt->executeUpdate();
		}
		else if (icon_url != "") {
			std::unique_ptr < sql::PreparedStatement> pre_stmt(
				this->_connection->prepareStatement("INSERT INTO " + _mysql_table_name_clients +
					"(cid, name, mail_address, password_hash, icon_url, created_at, last_login) "
					"VALUES(?, ?, ?, ?, ?, ?, NULL);")
			);
			pre_stmt->setString(1, cid);
			pre_stmt->setString(2, name);
			pre_stmt->setString(3, mail_address);
			pre_stmt->setString(4, password_hash);
			pre_stmt->setString(5, icon_url);
			pre_stmt->setString(6, created_at);
			pre_stmt->executeUpdate();
		}
		else {

			std::unique_ptr < sql::PreparedStatement> pre_stmt(
				this->_connection->prepareStatement("INSERT INTO " + _mysql_table_name_clients +
					"(cid, name, mail_address, password_hash, icon_url, created_at, last_login) "
					"VALUES(?, ?, ?, ?, ?, ?, ?);")
			);
			pre_stmt->setString(1, cid);
			pre_stmt->setString(2, name);
			pre_stmt->setString(3, mail_address);
			pre_stmt->setString(4, password_hash);
			pre_stmt->setString(5, icon_url);
			pre_stmt->setString(6, created_at);
			pre_stmt->setString(7, last_login);
			pre_stmt->executeUpdate();
		}
		
		spdlog::debug("MySQL:insert client data success.");
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return false;

	}
	catch (const std::runtime_error& e) {
		LOG_ERROR("Runtime Error:{}", e.what());
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
	

	
}

std::tuple<std::string, std::string, std::string>  MySQLClient::get_client_info_fromDB(const std::string& cid)
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
			std::string icon_url;
			if (res->isNull("icon_url") == true) {
				icon_url = "";
			}
			else {
				icon_url = res->getString("icon_url");
			}
			spdlog::debug("MySQL:client name:{}, mail_address:{}, icon_url:{}", name, mail_address, icon_url);
			return std::make_tuple(name, mail_address, icon_url);
		}
		else {
			LOG_ERROR("MySQL: no data found for cid: {}", cid);
			return std::make_tuple("", "", "");
		}
	}
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		LOG_ERROR("SQLException:{}", e.what());
		return std::make_tuple("", "", "");

	}
	catch (const std::runtime_error& e) {
		LOG_ERROR("Runtime Error:{}", e.what());
		return std::make_tuple("", "", "");
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
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
	catch (const sql::SQLException& e)
	{
		LOG_ERROR("SQLException:{}", e.what());
		return false;
	}
	catch (const std::runtime_error& e) {
		LOG_ERROR("Runtime Error:{}", e.what());
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
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
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::runtime_error& e) {
		LOG_ERROR("Runtime Error:{}", e.what());
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
	
}

bool MySQLClient::is_room_id_exist(const std::string& room_id)
{
	try
	{
		//room_idをBinary(16)に変換
		auto arr = base32::decode(room_id);
		std::vector<uint8_t> vec(arr.begin(), arr.end());

		// blob に設定
		std::string binary_str(reinterpret_cast<const char*>(vec.data()), vec.size());
		std::istringstream blob_stream(binary_str);

		for (uint8_t byte : vec) {
			std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
		}
		std::cout << std::dec << std::endl;  // 出力後に10進数へ戻す
		
	
	
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("SELECT EXISTS (SELECT 1 FROM " + _mysql_table_name_chatrooms + " WHERE room_id = ?);")
		);
		pre_stmt->setBlob(1, &blob_stream);
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
	
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR("Runtime Error:{}", e.what());
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
}

bool MySQLClient::is_room_id_hex_exist(const std::string& room_id_hex)
{
	try {
		// 16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);

		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());

		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("SELECT EXISTS (SELECT 1 FROM " + _mysql_table_name_chatrooms + " WHERE room_id = ?);")
		);

		std::istringstream blob_stream(binary_str);  // バイナリストリームに変換
		pre_stmt->setBlob(1, &blob_stream);  // 16バイトのバイナリデータをセット
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());

		if (res != nullptr && res->next()) {
			int is_exist = res->getBoolean(1);
			if (is_exist == 1) {
				spdlog::debug("MySQL:room id({}) exists.", room_id_hex);
				return true;
			}
			else {
				spdlog::debug("MySQL:room id({}) does not exist.", room_id_hex);
				return false;
			}
		}
		else {
			LOG_ERROR("MySQL: no data found for room_id(response is null): {}", room_id_hex);
			return false;
		}

	}
	catch (const sql::SQLException& e) {
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR("Runtime Error:{}", e.what());
		return false;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return false;
	}
	return false;
}

bool MySQLClient::insert_chat_message(const uint64_t mid, const std::string& room_id_hex, const std::string& sender_id, const std::string& send_time, const std::string& content)
{
	try
	{
	
		// 16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);

		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());

		// blob に設定
		//std::string binary_str(reinterpret_cast<const char*>(vec.data()), vec.size());
		std::istringstream blob_stream(binary_str);

		//SQL文のセット
		std::unique_ptr <sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("INSERT INTO " + _mysql_table_name_chat_messages +
				"(mid, room_id, sender_id, send_time, `message`) "
				"VALUES(?, ?, ?, ?, ?);")
		);
		pre_stmt->setInt64(1, mid);
		pre_stmt->setBlob(2, &blob_stream);
		pre_stmt->setString(3, sender_id);
		pre_stmt->setString(4, send_time);
		pre_stmt->setString(5, content);

		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:insert chat message success.");
		return true;
	}

	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR("Runtime Error:{}", e.what());
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
}

bool MySQLClient::insert_unsend_message(const uint64_t mid, const std::string& cid, const std::string& room_id_hex)
{
	try
	{
		// 16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);

		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());
		std::istringstream blob_stream(binary_str);

		std::unique_ptr <sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("INSERT INTO " + _mysql_table_name_unsend_messages +
				"(mid, cid, room_id) "
				"VALUES(?, ?, ?);")
		);
		pre_stmt->setInt64(1, mid);
		pre_stmt->setString(2, cid);
		pre_stmt->setBlob(3, &blob_stream);
		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:insert unsend message(message id:{}, cid:{}, room_id:{}) success.", mid, cid, room_id_hex);
		return true;
	}

	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR("Runtime Error:{}", e.what());
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
}

bool MySQLClient::insert_chatroom(const std::string& room_id_hex, const std::string& room_name, const std::string& icon_url, const std::string& created_at)
{
	try
	{
		// 16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);

		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());
	
		std::istringstream blob_stream(binary_str);

		std::unique_ptr <sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("INSERT INTO " + _mysql_table_name_chatrooms +
				"(room_id, room_name, room_icon_url, created_at) "
				"VALUES(?, ?, ?, ?);")
		);

		pre_stmt->setBlob(1, &blob_stream);
		pre_stmt->setString(2, room_name);
		pre_stmt->setString(3, icon_url);
		pre_stmt->setString(4, created_at);
		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:insert chatroom(room_id:{}, room_name:{}, created_at:{}) success.", room_id_hex, room_name, created_at);
		return true;

	}
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR("Runtime Error:{}", e.what());
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
}

bool MySQLClient::insert_chatroom_member(const std::string& room_id_hex, const std::vector<std::string>& cids, const std::string& joined_at)
{
	try
	{
	
		std::unique_ptr <sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("INSERT INTO " + _mysql_table_name_chat_members +
				"(room_id, cid, joined_at) "
				"VALUES(?, ?, ?);")
		);
		for (const auto& cid : cids) {
			// 16進数文字列をバイナリデータに変換
			std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);
			// バイナリデータを string に変換（MySQL 用）
			std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());
			std::istringstream blob_stream(binary_str);
			pre_stmt->setBlob(1, &blob_stream);
			pre_stmt->setString(2, cid);
			pre_stmt->setString(3, joined_at);
			pre_stmt->executeUpdate();
			spdlog::debug("MySQL:insert chatroom member(room_id:{}, cid:{}, joined_at:{}) success.", room_id_hex, cid, joined_at);
		}	
		spdlog::info("MySQL:insert chatroom member(room id : {})", room_id_hex);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR("Runtime Error:{}", e.what());
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
}

bool MySQLClient::delete_chatroom_member(const std::string& room_id_hex, const std::vector<std::string>& cids)
{
	try
	{
		
		std::unique_ptr <sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("DELETE FROM " + _mysql_table_name_chat_members +
				" WHERE room_id = ? AND cid = ?;")
		);
		for (const auto& cid : cids){
			// 16進数文字列をバイナリデータに変換
			std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);
			// バイナリデータを string に変換（MySQL 用）
			std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());
			std::istringstream blob_stream(binary_str);
			pre_stmt->setBlob(1, &blob_stream);
			pre_stmt->setString(2, cid);
			pre_stmt->executeUpdate();
			spdlog::debug("MySQL:delete chatroom member(room_id:{}, cid:{}) success.", room_id_hex, cid);
		}
		spdlog::info("MySQL:delete chatroom member(room id : {})", room_id_hex);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR("Runtime Error:Failed to decode room id. Error: {}", e.what());
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
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
		this->print_sql_exception(e);
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
	}

	return last_login_time;
}

bool MySQLClient::insert_unsend_control_message(
	const uint64_t mid, 
	const std::string& cid, 
	const std::string& room_id, 
	const std::string& json_payload, 
	const std::string& request_time
)
{
	try
	{
		std::string query = "INSERT INTO " + _mysql_table_name_unsend_control_messages +
			"(mid, cid, room_id, json_payload, request_time) VALUES(?, ?, ?, ?, ?);";
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement(query)
		);
		// room_idを16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id);
		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());
		std::istringstream blob_stream(binary_str);


		//パラメータを設定
		pre_stmt->setInt64(1, mid); // メッセージID
		pre_stmt->setString(2, cid); // クライアントID
		pre_stmt->setBlob(3, &blob_stream);	//ルームID
		pre_stmt->setString(4, json_payload); // JSONペイロード
		pre_stmt->setString(5, request_time); // 作成日時
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());
		spdlog::debug("MySQL:insert unsend control message(mid:{}, cid:{}, room_id:{}) success.", mid, cid, room_id);
	}
	catch (const sql::SQLException& e) {
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::runtime_error& e) {
		LOG_ERROR("Runtime Error:{}", e.what());
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
	return true;
}

json MySQLClient::get_unsend_control_message_fromDB(const std::string& cid)
{
	try
	{
		/*std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement(
				"SELECT " + _mysql_table_name_unsend_control_messages + ".mid, " + _mysql_table_name_unsend_control_messages + ".cid, " +
				  _mysql_table_name_unsend_control_messages + ".room_id, " + _mysql_table_name_unsend_control_messages + ".json_payload, " +
				  _mysql_table_name_unsend_control_messages + ".created_at "
				"FROM " + _mysql_table_name_unsend_control_messages + " "
				"WHERE " + _mysql_table_name_unsend_control_messages + ".cid = ?;"
			)
		);*/

		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement(
				"SELECT " + _mysql_table_name_unsend_control_messages + ".mid, " +
				_mysql_table_name_unsend_control_messages + ".cid, " +
				_mysql_table_name_unsend_control_messages + ".room_id, " +
				_mysql_table_name_unsend_control_messages + ".json_payload, " +
				_mysql_table_name_unsend_control_messages + ".request_time " +
				"FROM " + _mysql_table_name_unsend_control_messages + " " +
				"WHERE " + _mysql_table_name_unsend_control_messages + ".cid = ?;"
			)
		);
		pre_stmt->setString(1, cid);
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());
		json unsend_control_messages_json = json::array();
		while (res->next())
		{
			json message;
			message["mid"] = res->getInt64("mid");
			message["cid"] = res->getString("cid");
			// BINARY(16)のroom_idをバイト列として取得し、ULID文字列に変換
			std::string room_id_bin = res->getString("room_id");
			if (room_id_bin.size() == 16) {
				std::ostringstream oss;
				for (unsigned char c : room_id_bin) {
					oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
				}
				message["room_id"] = oss.str(); // ULID を HEX 文字列で返す
			}
			else {
				message["room_id"] = "";
			}
			message["json_payload"] = res->getString("json_payload");
			message["request_time"] = res->getString("request_time");
			unsend_control_messages_json.push_back(message);
		}
		return unsend_control_messages_json;
	}
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return json();
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return json();
	}
}

bool MySQLClient::delete_unsend_control_message(const uint64_t mid, const std::string& cid, const std::string& room_id)
{
	try
	{
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("DELETE FROM " + _mysql_table_name_unsend_control_messages +
				" WHERE mid = ? AND cid = ? AND room_id = ?;")
		);
		// room_idを16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id);
		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());
		std::istringstream blob_stream(binary_str);

		pre_stmt->setInt64(1, mid);
		pre_stmt->setString(2, cid);
		pre_stmt->setBlob(3, &blob_stream);	//ルームID
		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:delete unsend control message(mid:{}, cid:{}, room_id:{}) success.", mid, cid, room_id);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::runtime_error& e) {
		LOG_ERROR("Runtime Error:{}", e.what());
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
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
	/*	std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement(
				"SELECT " + _mysql_table_name_unsend_messages + ".mid, " + _mysql_table_name_unsend_messages + ".cid, "
				+_mysql_table_name_unsend_messages + ".room_id, " + _mysql_table_name_chat_messages + ".message "
				"FROM " + _mysql_table_name_unsend_messages + " "
				"JOIN " + _mysql_table_name_chat_messages + " ON " + _mysql_table_name_unsend_messages + ".mid = " + _mysql_table_name_chat_messages + ".mid "
				"WHERE "_mysql_table_name_unsend_messages + ".cid = ? AND " + _mysql_table_name_chat_messages + ".send_time > ? "
			)
		);*/
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement(
				"SELECT " + _mysql_table_name_unsend_messages + ".mid, " +
				_mysql_table_name_unsend_messages + ".cid, " +
				_mysql_table_name_unsend_messages + ".room_id, " +
				_mysql_table_name_chat_messages + ".message, " +
				_mysql_table_name_chat_messages + ".send_time " +
				"FROM " + _mysql_table_name_unsend_messages + " " +
				"JOIN " + _mysql_table_name_chat_messages + " ON " +
				_mysql_table_name_unsend_messages + ".mid = " +
				_mysql_table_name_chat_messages + ".mid " +
				"WHERE " + _mysql_table_name_unsend_messages + ".cid = ?"
			)
		);
		pre_stmt->setString(1, cid);
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());
		//未読メッセージの配列を作成
		json unsend_messages_json = json::array();
		while (res -> next())
		{
			json message;
			message["mid"] = res->getInt64("mid");
			message["sender_cid"] = res->getString("cid");

			// BINARY(16)のroom_idをバイト列として取得し、ULID文字列に変換
			std::string room_id_bin = res->getString("room_id");
			if (room_id_bin.size() == 16) {
				std::ostringstream oss;
				for (unsigned char c : room_id_bin) {
					oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
				}
				message["send_room_id"] = oss.str(); // ULID を HEX 文字列で返す
			}
			else {
				message["send_room_id"] = "";
			}
			message["send_time"] = res->getString("send_time");
			message["message"] = res->getString("message");
			unsend_messages_json.push_back(message);


		}
		spdlog::debug("MySQL: Unsend messages fetched successfully for client {}", cid);
		return unsend_messages_json;
	}
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return json();
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return json();
	}
}

bool MySQLClient::delete_unsend_message(const uint64_t mid, const std::string& cid, const std::string& room_id)
{
	try
	{
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("DELETE FROM " + _mysql_table_name_unsend_messages +
				" WHERE mid = ? AND cid = ? AND room_id = ?;")
		);
		// room_idを16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id);
		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());
		std::istringstream blob_stream(binary_str);
		pre_stmt->setInt64(1, mid);
		pre_stmt->setString(2, cid);
		pre_stmt->setBlob(3, &blob_stream);	//ルームID
		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:delete unsend message(mid:{}, cid:{}, room_id:{}) success.", mid, cid, room_id);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::runtime_error& e) {
		LOG_ERROR("Runtime Error:{}", e.what());
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
}

std::vector<std::string> MySQLClient::get_client_list_from_chatroom(const std::string& room_id_hex)
{
	try
	{
		// 16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);

		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());
		std::istringstream blob_stream(binary_str);
		std::vector<std::string> client_list;

		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("SELECT cid FROM " + _mysql_table_name_chat_members + " WHERE room_id = ?;")
		);
		pre_stmt->setBlob(1, &blob_stream);
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
		this->print_sql_exception(e);
		return std::vector<std::string>();
	}
	catch (const std::runtime_error& e) {
		LOG_ERROR("Runtime Error : Failed to fetch client list from chatrrom. Error: {}", e.what());
		return std::vector<std::string>();
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return std::vector<std::string>();
	}
}

std::string MySQLClient::get_chatroom_name_fromDB(const std::string& room_id_hex)
{
	try
	{
		// 16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);

		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());

		std::istringstream blob_stream(binary_str);
		std::vector<std::string> client_list;

		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("SELECT room_name FROM " + _mysql_table_name_chatrooms + " WHERE room_id = ?;")
		);
		pre_stmt->setBlob(1, &blob_stream);
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());
		if (res != nullptr && res->next()) {
			std::string room_name = res->getString("room_name");
			spdlog::debug("MySQL:chatroom name:{}", room_name);
			return room_name;
		}
		else {
			LOG_ERROR("MySQL: no data found for room_id: {}", room_id_hex);
			return std::string();
		}
	}

	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return std::string();
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR("Runtime Error:Failed to decode room id. Error: {}", e.what());
		return std::string();
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return std::string();
	}
}

bool MySQLClient::delete_chatroom_fromDB(const std::string& room_id_hex)
{
	try
	{
		// 16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);

		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());

		std::istringstream blob_stream(binary_str);
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("DELETE FROM " + _mysql_table_name_chatrooms + " WHERE room_id = ?;")
		);
		pre_stmt->setBlob(1, &blob_stream);
		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:delete chatroom(room id : {}) success.", room_id_hex);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR("Runtime Error:Failed to decode room id. Error: {}", e.what());
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
}

bool MySQLClient::update_chatroom_name(const std::string& room_id_hex, const std::string& room_name)
{
	try
	{
		// 16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);

		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());

		std::istringstream blob_stream(binary_str);
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("UPDATE " + _mysql_table_name_chatrooms +
				" SET room_name = ? WHERE room_id = ?;")
		);
		pre_stmt->setString(1, room_name);
		pre_stmt->setBlob(2, &blob_stream);
		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:update chatroom name(room id : {}) success.", room_id_hex);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR("Runtime Error:Failed to decode room id. Error: {}", e.what());
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
}

bool MySQLClient::update_chatroom_icon(const std::string& room_id_hex, const std::string& icon_url)
{
	try
	{
		// 16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);

		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());
	
		std::istringstream blob_stream(binary_str);
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_connection->prepareStatement("UPDATE " + _mysql_table_name_chatrooms +
				" SET room_icon_url = ? WHERE room_id = ?;")
		);
		pre_stmt->setString(1, icon_url);
		pre_stmt->setBlob(2, &blob_stream);
		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:update chatroom icon(room id : {}) success.", room_id_hex);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR("Runtime Error:Failed to decode room id. Error: {}", e.what());
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
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
				"icon_url VARCHAR(255),"
				"created_at DATETIME NOT NULL,"
				"last_login DATETIME);")
		);
		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:create table(table name:{}) success.", _mysql_table_name_clients);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
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
				"room_icon_url VARCHAR(255),"
				"created_at DATETIME,"
				"PRIMARY KEY (room_id));")
		);
		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:create table(table name:{}) success.", _mysql_table_name_chatrooms);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
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
		spdlog::debug("MySQL:create table(table name:{}) success.", _mysql_table_name_chat_members);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
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
		spdlog::debug("MySQL:create table(table name:{}) success.", _mysql_table_name_chat_messages);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
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
		spdlog::debug("MySQL:create table(table name:{}) success.", _mysql_table_name_unsend_messages);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
}

bool MySQLClient::create_table_unsend_control_messages()
{
	try
	{
		std::unique_ptr<sql::PreparedStatement > pre_stmt(
			this->_connection->prepareStatement("CREATE TABLE IF NOT EXISTS " + _mysql_table_name_unsend_control_messages +
				"(mid BIGINT NOT NULL,"
				"cid VARCHAR(28) NOT NULL,"
				"room_id BINARY(16) NOT NULL,"
				"json_payload JSON NOT NULL,"
				"request_time DATETIME NOT NULL,"
				"PRIMARY KEY(mid, cid, room_id),"
				"FOREIGN KEY (mid) REFERENCES chat_messages(mid) ON DELETE CASCADE ON UPDATE CASCADE,"
				"FOREIGN KEY (cid) REFERENCES clients(cid) ON DELETE CASCADE,"
				"FOREIGN KEY (room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE);")
		);
		pre_stmt->executeUpdate();
		spdlog::debug("MySQL:create table(table name:{}) success.", _mysql_table_name_unsend_control_messages);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->print_sql_exception(e);
		return false;
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
}

std::vector<uint8_t> MySQLClient::hex_to_binary(const std::string& hex_str)
{
	std::vector<uint8_t> binary_data;

	for (size_t i = 0; i < hex_str.length(); i += 2) {
		std::string byte_str = hex_str.substr(i, 2);
		uint8_t byte = static_cast<uint8_t>(std::stoi(byte_str, nullptr, 16));
		binary_data.push_back(byte);
	}

	return binary_data;
}

