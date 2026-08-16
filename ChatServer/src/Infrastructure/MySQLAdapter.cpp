#include "Infrastructure/MySQLAdapter.hpp"

namespace {
std::string build_mysql_endpoint(const char* host, const char* port)
{
	const char* resolved_host = (host != nullptr && host[0] != '\0') ? host : "127.0.0.1";
	const char* resolved_port = (port != nullptr && port[0] != '\0') ? port : "3306";
	return std::string("tcp://") + resolved_host + ":" + resolved_port;
}
}


MySQLAdapter::MySQLAdapter()
{
	char* env_host = nullptr;
	char* env_user = nullptr;
	char* env_pass = nullptr;
	char* env_db = nullptr;
	char* env_port = nullptr;
#ifdef _WIN32
	size_t len = 0;

	if (_dupenv_s(&env_host, &len, "DB_HOST") == 0 && env_host != nullptr) {
		LOG_DEBUG("DB_HOST: {}", env_host);
	}
	else {
		LOG_ERROR("DB_HOST environment variable is not set.");
	}
	if(_dupenv_s(&env_user, &len, "DB_USER") == 0 && env_user != nullptr) {
		LOG_DEBUG("DB_USER: {}", env_user);
	}
	else {
		LOG_ERROR("DB_USER environment variable is not set.");
	}
	if(_dupenv_s(&env_pass, &len, "DB_PASSWORD") == 0 && env_pass != nullptr) {
		LOG_DEBUG("DB_PASSWORD: {}", env_pass);
	}else {
		LOG_ERROR("DB_PASSWORD environment variable is not set.");
	}
	if(_dupenv_s(&env_db, &len, "DB_NAME") == 0 && env_db != nullptr) {
		LOG_DEBUG("DB_NAME: {}", env_db);
	}else {
		LOG_ERROR("DB_NAME environment variable is not set.");
	}
	if(_dupenv_s(&env_port, &len, "DB_PORT") == 0 && env_port != nullptr) {
		LOG_DEBUG("DB_PORT: {}", env_port);
	}else {
		LOG_ERROR("DB_PORT environment variable is not set.");
	}
#else
	env_host = std::getenv("DB_HOST");
	env_user = std::getenv("DB_USER");
	env_pass = std::getenv("DB_PASSWORD");
	env_db = std::getenv("DB_NAME");
	env_port = std::getenv("DB_PORT");
#endif // _WIN32
    _mysql_host = build_mysql_endpoint(env_host, env_port);
    _mysql_user_name = env_user ? env_user : "chat_user";
    _mysql_password = env_pass ? env_pass : "chat_pass";
    _mysql_db_name = env_db ? env_db : "TalkLinkDB";
	LOG_DEBUG("MySQL: host: {}, user: {}, password: {}, db_name: {}", _mysql_host, _mysql_user_name, _mysql_password, _mysql_db_name);
#ifdef _WIN32
	free(env_host);
	free(env_user);
	free(env_pass);
	free(env_db);
	free(env_port);
#endif // _WIN32
	_mysql_helper = std::make_unique<MySQLHelper>(_mysql_host, _mysql_user_name, _mysql_password, _mysql_db_name);
	if (!_mysql_helper->connect(_mysql_db_name)) {
		LOG_ERROR("Failed to connect to MySQL database.");
		exit(1);
	}
	LOG_DEBUG("MySQLAdapter initialized successfully.");
	
}


MySQLAdapter::~MySQLAdapter()
{

}

void MySQLAdapter::create_database()
{
	try
	{
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("CREATE DATABASE IF NOT EXISTS " + _mysql_db_name)
		);
		pre_stmt->executeUpdate();
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
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

bool MySQLAdapter::create_table()
{
	if (this->create_table_users() == false) {
		return false;
	}
	else if (this->create_table_chatrooms() == false) {
		return false;
	}
	else if (this->create_table_chat_members() == false) {
		return false;
	}
	else if (this->create_table_messages() == false) {
		return false;
	}
	else {
		return true;
	}
}

bool MySQLAdapter::insert_user_info_data(
	const std::string& uid,
	const std::string& name, 
	const std::string& mail_address,
	const std::string& icon_url,
	const std::string& created_at, 
	const std::string& last_login
)
{
	std::lock_guard<std::mutex> lock(_mysql_mutex);
	try
	{
		if (last_login == "" && icon_url == "") {
			std::unique_ptr < sql::PreparedStatement> pre_stmt(
				this->_mysql_helper->get_connection()->prepareStatement("INSERT INTO " + _mysql_table_name_users +
					"(uid, name, mail_address, icon_url, created_at, last_login) "
					"VALUES(?, ?, ?, NULL, ?, NULL);")
			);
			pre_stmt->setString(1, uid);
			pre_stmt->setString(2, name);
			pre_stmt->setString(3, mail_address);
			pre_stmt->setString(4, created_at);
			pre_stmt->executeUpdate();
		}
		else if (icon_url != "") {
			std::unique_ptr < sql::PreparedStatement> pre_stmt(
				this->_mysql_helper->get_connection()->prepareStatement("INSERT INTO " + _mysql_table_name_users +
					"(uid, name, mail_address, icon_url, created_at, last_login) "
					"VALUES(?, ?, ?, ?, ?, NULL);")
			);
			pre_stmt->setString(1, uid);
			pre_stmt->setString(2, name);
			pre_stmt->setString(3, mail_address);
			pre_stmt->setString(4, icon_url);
			pre_stmt->setString(5, created_at);
			pre_stmt->executeUpdate();
		}
		else {

			std::unique_ptr < sql::PreparedStatement> pre_stmt(
				this->_mysql_helper->get_connection()->prepareStatement("INSERT INTO " + _mysql_table_name_users +
					"(uid, name, mail_address, icon_url, created_at, last_login) "
					"VALUES(?, ?, ?, ?, ?, ?);")
			);
			pre_stmt->setString(1, uid);
			pre_stmt->setString(2, name);
			pre_stmt->setString(3, mail_address);
			pre_stmt->setString(4, icon_url);
			pre_stmt->setString(5, created_at);
			pre_stmt->setString(6, last_login);
			pre_stmt->executeUpdate();
		}
		
		LOG_DEBUG("MySQL:insert user data success.");
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
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

UserInfo MySQLAdapter::get_user_info(const std::string& uid)
{
	std::lock_guard<std::mutex> lock(_mysql_mutex);
	try
	{
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("SELECT * FROM " + _mysql_table_name_users + " WHERE uid = ?;")
		);
		pre_stmt->setString(1, uid);
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
			LOG_DEBUG("MySQL:user name:{}, mail_address:{}, icon_url:{}", name, mail_address, icon_url);
			return UserInfo{name, mail_address, icon_url};
		}
		else {
			LOG_ERROR("MySQL: no data found for uid: {}", uid);
			return UserInfo{};
		}
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
		LOG_ERROR("SQLException:{}", e.what());
		return UserInfo{};

	}
	catch (const std::runtime_error& e) {
		LOG_ERROR("Runtime Error:{}", e.what());
		return UserInfo{};
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return UserInfo{};
	}
}

bool MySQLAdapter::update_last_login(const std::string& uid, const std::string& last_login)
{
	std::lock_guard<std::mutex> lock(_mysql_mutex);
	try
	{
		if (uid == "") {
			LOG_ERROR("MySQL:user id is empty.Skip updating user last login time.");
			return false;
		}
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("UPDATE " + _mysql_table_name_users +
				" SET last_login = ? WHERE uid = ?;")
		);
		pre_stmt->setString(1, last_login);
		pre_stmt->setString(2, uid);
		pre_stmt->executeUpdate();
		LOG_DEBUG("MySQL:update last login(uid : {}) success.", uid);
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

bool MySQLAdapter::is_uid_exist(const std::string& uid)
{
	std::lock_guard<std::mutex> lock(_mysql_mutex);
	try
	{
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("SELECT EXISTS (SELECT 1 FROM " + _mysql_table_name_users + " WHERE uid = ?);")
		);
		pre_stmt->setString(1, uid);
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());
		if (res != nullptr && res->next()) {
			int is_exist = res->getBoolean(1);
			if (is_exist == 1) {
				LOG_DEBUG("MySQL:user id exists.");
				return true;
			}
			else {
				LOG_DEBUG("MySQL:user id does not exist.");
				return false;
			}
		}
		else {
			LOG_ERROR("MySQL: no data found for uid: {}", uid);
			return false;
		}
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
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

bool MySQLAdapter::is_room_id_exist(const std::string& room_id)
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
			this->_mysql_helper->get_connection()->prepareStatement("SELECT EXISTS (SELECT 1 FROM " + _mysql_table_name_chatrooms + " WHERE room_id = ?);")
		);
		pre_stmt->setBlob(1, &blob_stream);
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());
		if (res != nullptr && res->next()) {
			int is_exist = res->getBoolean(1);
			if (is_exist == 1) {
				LOG_DEBUG("MySQL:room id({}) exists.", room_id);
				return true;
			}
			else {
				LOG_DEBUG("MySQL:room id({}) does not exist.", room_id);
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
		this->_mysql_helper->printError(e);
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

bool MySQLAdapter::is_room_id_hex_exist(const std::string& room_id_hex)
{
	try {
		// 16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);

		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());

		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("SELECT EXISTS (SELECT 1 FROM " + _mysql_table_name_chatrooms + " WHERE room_id = ?);")
		);

		std::istringstream blob_stream(binary_str);  // バイナリストリームに変換
		pre_stmt->setBlob(1, &blob_stream);  // 16バイトのバイナリデータをセット
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());

		if (res != nullptr && res->next()) {
			int is_exist = res->getBoolean(1);
			if (is_exist == 1) {
				LOG_DEBUG("MySQL:room id({}) exists.", room_id_hex);
				return true;
			}
			else {
				LOG_DEBUG("MySQL:room id({}) does not exist.", room_id_hex);
				return false;
			}
		}
		else {
			LOG_ERROR("MySQL: no data found for room_id(response is null): {}", room_id_hex);
			return false;
		}

	}
	catch (const sql::SQLException& e) {
		this->_mysql_helper->printError(e);
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
	return false;
}

bool MySQLAdapter::insert_message(const uint64_t mid, const std::string& room_id_hex, const std::string& sender_uid, const std::string& send_time, const std::string& message_content, std::uint64_t room_sequence)
{
	try
	{
		// 16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);

		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());

		// blob に設定
		std::istringstream blob_stream(binary_str);

		//SQL文のセット
		std::unique_ptr <sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("INSERT INTO " + _mysql_table_name_messages +
				"(mid, room_id, sender_uid, send_time, `message`, room_sequence) "
				"VALUES(?, ?, ?, ?, ?, ?);")
		);

		pre_stmt->setInt64(1, mid);
		pre_stmt->setBlob(2, &blob_stream);
		pre_stmt->setString(3, sender_uid);
		pre_stmt->setString(4, send_time);
		pre_stmt->setString(5, message_content);
		pre_stmt->setInt64(6, static_cast<int64_t>(room_sequence));

		pre_stmt->executeUpdate();
		LOG_DEBUG("MySQL:insert chat message success.");
		return true;
	}

	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
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

json MySQLAdapter::get_unreceived_messages(const std::string &uid, const uint64_t last_received_message_id)
{
    json result;
	result["unreceived_messages"] = json::array();
    try
    {
		// SQL: messages と chat_members を JOIN
        std::string sql =
    		"SELECT m.mid, m.room_id, m.sender_uid, m.send_time, m.message "
    		"FROM " + _mysql_table_name_messages + " AS m "
    		"JOIN " + _mysql_table_name_chat_members + " AS cm ON m.room_id = cm.room_id "
    		"WHERE m.mid > ? AND cm.uid = ? AND BINARY m.sender_uid != ?;";
        std::unique_ptr<sql::PreparedStatement> pre_stmt(
            this->_mysql_helper->get_connection()->prepareStatement(sql)
        );

		pre_stmt->setInt64(1, last_received_message_id);
        pre_stmt->setString(2, uid);
        pre_stmt->setString(3, uid);

        std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());

        while (res != nullptr && res->next())
        {
            json message;
            message["mid"] = res->getInt64("mid");

			// room_idはそのままだとバイナリデータなので、jsonをdumpできないため、16進数文字列に変換
            std::string room_id_bin = res->getString("room_id");
            message["room_id"] = this->to_hex_string(room_id_bin);

            message["sender_uid"] = res->getString("sender_uid");
            // MySQL DATETIME形式をISO 8601形式に変換
            std::string mysql_timestamp = res->getString("send_time");
            message["send_time"] = ServerTime::mysql_datetime_to_iso8601(mysql_timestamp);
			try
			{
				// message カラムの JSON をパース
				std::string message_str = static_cast<std::string>(res->getString("message"));
				LOG_DEBUG("MySQL:message_str: {}", message_str);
    			message["message"] = json::parse(message_str, nullptr, true);
			}
			catch (const json::parse_error &e)
			{
				LOG_ERROR("Invalid JSON in message (mid={}): {}", res->getInt64("mid"), e.what());
				message["message"] = nullptr; // エラー時は null にする
			}
			
			result["unreceived_messages"].push_back(message);
			
		}
    }
    catch (const sql::SQLException& e)
    {
        this->_mysql_helper->printError(e);
    }
    catch (const std::runtime_error& e)
    {
        LOG_ERROR("Runtime Error:{}", e.what());
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Exception:{}.", e.what());
    }
    return result;
}
bool MySQLAdapter::insert_unsend_message(const uint64_t mid, const std::string &uid, const std::string &room_id_hex)
{
	try
	{
		// 16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);

		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());
		std::istringstream blob_stream(binary_str);

		std::unique_ptr <sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("INSERT INTO " + _mysql_table_name_unsend_messages +
				"(mid, uid, room_id) "
				"VALUES(?, ?, ?);")
		);
		pre_stmt->setInt64(1, mid);
		pre_stmt->setString(2, uid);
		pre_stmt->setBlob(3, &blob_stream);
		pre_stmt->executeUpdate();
		LOG_DEBUG("MySQL:insert unsend message(message id:{}, uid:{}, room_id:{}) success.", mid, uid, room_id_hex);
		return true;
	}

	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
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

bool MySQLAdapter::insert_chatroom(const std::string& room_id_hex, const std::string& room_name, const std::string& icon_url, const std::string& created_at)
{
	try
	{
		// 16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);

		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());
	
		std::istringstream blob_stream(binary_str);

		std::unique_ptr <sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("INSERT INTO " + _mysql_table_name_chatrooms +
				"(room_id, room_name, room_icon_url, created_at) "
				"VALUES(?, ?, ?, ?);")
		);

		pre_stmt->setBlob(1, &blob_stream);
		pre_stmt->setString(2, room_name);
		pre_stmt->setString(3, icon_url);
		pre_stmt->setString(4, created_at);
		pre_stmt->executeUpdate();
		LOG_DEBUG("MySQL:insert chatroom(room_id:{}, room_name:{}, created_at:{}) success.", room_id_hex, room_name, created_at);
		return true;

	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
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

bool MySQLAdapter::insert_chatroom_member(const std::string& room_id_hex, const std::vector<std::string>& uids, const std::string& joined_at)
{
	try
	{
	
		std::unique_ptr <sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("INSERT INTO " + _mysql_table_name_chat_members +
				"(room_id, uid, joined_at) "
				"VALUES(?, ?, ?);")
		);
		for (const auto& uid : uids) {
			// 16進数文字列をバイナリデータに変換
			std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);
			// バイナリデータを string に変換（MySQL 用）
			std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());
			std::istringstream blob_stream(binary_str);
			pre_stmt->setBlob(1, &blob_stream);
			pre_stmt->setString(2, uid);
			pre_stmt->setString(3, joined_at);
			pre_stmt->executeUpdate();
			LOG_DEBUG("MySQL:insert chatroom member(room_id:{}, uid:{}, joined_at:{}) success.", room_id_hex, uid, joined_at);
		}	
		LOG_DEBUG("MySQL:insert chatroom member(room id : {})", room_id_hex);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
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

bool MySQLAdapter::delete_chatroom_member(const std::string& room_id_hex, const std::vector<std::string>& uids)
{
	try
	{
		
		std::unique_ptr <sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("DELETE FROM " + _mysql_table_name_chat_members +
				" WHERE room_id = ? AND uid = ?;")
		);
		for (const auto& uid : uids){
			// 16進数文字列をバイナリデータに変換
			std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);
			// バイナリデータを string に変換（MySQL 用）
			std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());
			std::istringstream blob_stream(binary_str);
			pre_stmt->setBlob(1, &blob_stream);
			pre_stmt->setString(2, uid);
			pre_stmt->executeUpdate();
			LOG_DEBUG("MySQL:delete chatroom member(room_id:{}, uid:{}) success.", room_id_hex, uid);
		}
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
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



std::string MySQLAdapter::get_user_last_login_fromDB(const std::string& uid)
{
	std::string last_login_time = "";

	try {
		// クエリ: クライアントIDから最終ログイン日時を取得
		std::string query = "SELECT last_login FROM " + _mysql_table_name_users + " WHERE uid = ?";

		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement(query)
		);

		// パラメータを設定
		pre_stmt->setString(1, uid); // クライアントID

		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());

		// 結果を読み取る
		if (res->next()) {
			last_login_time = res->getString("last_login");
		}

		LOG_DEBUG("MySQL: Fetched last login time for user {}: {}", uid, last_login_time);

	}
	catch (const sql::SQLException& e) {
		this->_mysql_helper->printError(e);
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
	}

	return last_login_time;
}

std::vector<UserInfo> MySQLAdapter::search_user_infos(const std::string& query)
{
	try
	{
		std::unique_ptr <sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("SELECT uid, name, mail_address, icon_url FROM " + _mysql_table_name_users +
				" WHERE uid LIKE CONCAT(?, '%') OR name LIKE CONCAT(?, '%');")
		);
		pre_stmt->setString(1, query);
		pre_stmt->setString(2, query);
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());
		std::vector<UserInfo> user_infos;
		while (res->next()) {
			UserInfo user_info;
			user_info.uid = res->getString("uid");
			user_info.name = res->getString("name");
			user_info.mail_address = res->getString("mail_address");
			user_info.icon_url = res->getString("icon_url");
			user_infos.push_back(user_info);
		}
		return user_infos;
	}
	catch(const std::exception& e)
	{
		LOG_ERROR("[MySQLAdapter] Exception:{}.", e.what());
	}
	
    return std::vector<UserInfo>();
}


bool MySQLAdapter::insert_unsend_control_message(
    const uint64_t mid,
    const std::string &uid,
    const std::string &room_id,
    const std::string &json_payload,
    const std::string &request_time)
{
	try
	{
		std::string query = "INSERT INTO " + _mysql_table_name_unsend_control_messages +
			"(mid, uid, room_id, json_payload, request_time) VALUES(?, ?, ?, ?, ?);";
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement(query)
		);
		// room_idを16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id);
		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());
		std::istringstream blob_stream(binary_str);


		//パラメータを設定
		pre_stmt->setInt64(1, mid); // メッセージID
		pre_stmt->setString(2, uid); // クライアントID
		pre_stmt->setBlob(3, &blob_stream);	//ルームID
		pre_stmt->setString(4, json_payload); // JSONペイロード
		pre_stmt->setString(5, request_time); // 作成日時
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());
		LOG_DEBUG("MySQL:insert unsend control message(mid:{}, uid:{}, room_id:{}) success.", mid, uid, room_id);
	}
	catch (const sql::SQLException& e) {
		this->_mysql_helper->printError(e);
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

json MySQLAdapter::get_unsend_control_message_fromDB(const std::string& uid)
{
	try
	{
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement(
				"SELECT " + _mysql_table_name_unsend_control_messages + ".mid, " +
				_mysql_table_name_unsend_control_messages + ".uid, " +
				_mysql_table_name_unsend_control_messages + ".room_id, " +
				_mysql_table_name_unsend_control_messages + ".json_payload, " +
				_mysql_table_name_unsend_control_messages + ".request_time " +
				"FROM " + _mysql_table_name_unsend_control_messages + " " +
				"WHERE " + _mysql_table_name_unsend_control_messages + ".uid = ?;"
			)
		);
		pre_stmt->setString(1, uid);
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());
		json unsend_control_messages_json = json::array();
		while (res->next())
		{
			json message;
			message["mid"] = res->getInt64("mid");
			message["uid"] = res->getString("uid");
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
			// MySQL DATETIME形式をISO 8601形式に変換
			std::string mysql_timestamp = res->getString("request_time");
			message["request_time"] = ServerTime::mysql_datetime_to_iso8601(mysql_timestamp);
			unsend_control_messages_json.push_back(message);
		}
		return unsend_control_messages_json;
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
		return json();
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return json();
	}
}

bool MySQLAdapter::delete_unsend_control_message(const uint64_t mid, const std::string& uid, const std::string& room_id)
{
	try
	{
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("DELETE FROM " + _mysql_table_name_unsend_control_messages +
				" WHERE mid = ? AND uid = ? AND room_id = ?;")
		);
		// room_idを16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id);
		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());
		std::istringstream blob_stream(binary_str);

		pre_stmt->setInt64(1, mid);
		pre_stmt->setString(2, uid);
		pre_stmt->setBlob(3, &blob_stream);	//ルームID
		pre_stmt->executeUpdate();
		LOG_DEBUG("MySQL:delete unsend control message(mid:{}, uid:{}, room_id:{}) success.", mid, uid, room_id);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
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

json MySQLAdapter::get_unsend_message_fromDB(const std::string& uid, int limit)
{
	try
	{
		// 最終ログイン日時を取得
		std::string last_login_time = get_user_last_login_fromDB(uid);
		if (last_login_time.empty()) {
			LOG_ERROR("MySQL: No last login time found for user {}", uid);
			return json();
		}

		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement(
				"SELECT " + _mysql_table_name_unsend_messages + ".mid, " +
				_mysql_table_name_unsend_messages + ".uid, " +
				_mysql_table_name_unsend_messages + ".room_id, " +
				_mysql_table_name_messages + ".message, " +
				_mysql_table_name_messages + ".send_time " +
				"FROM " + _mysql_table_name_unsend_messages + " " +
				"JOIN " + _mysql_table_name_messages + " ON " +
				_mysql_table_name_unsend_messages + ".mid = " +
				_mysql_table_name_messages + ".mid " +
				"WHERE " + _mysql_table_name_unsend_messages + ".uid = ?"
			)
		);
		pre_stmt->setString(1, uid);
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());
		//未読メッセージの配列を作成
		json unsend_messages_json = json::array();
		while (res -> next())
		{
			json message;
			message["mid"] = res->getInt64("mid");
			message["sender_uid"] = res->getString("uid");

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
			// MySQL DATETIME形式をISO 8601形式に変換
			std::string mysql_timestamp = res->getString("send_time");
			message["send_time"] = ServerTime::mysql_datetime_to_iso8601(mysql_timestamp);
			message["message"] = res->getString("message");
			unsend_messages_json.push_back(message);


		}
		LOG_DEBUG("MySQL: Unsend messages fetched successfully for user {}", uid);
		return unsend_messages_json;
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
		return json();
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return json();
	}
}

bool MySQLAdapter::delete_unsend_message(const uint64_t mid, const std::string& uid, const std::string& room_id)
{
	try
	{
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("DELETE FROM " + _mysql_table_name_unsend_messages +
				" WHERE mid = ? AND uid = ? AND room_id = ?;")
		);
		// room_idを16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id);
		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());
		std::istringstream blob_stream(binary_str);
		pre_stmt->setInt64(1, mid);
		pre_stmt->setString(2, uid);
		pre_stmt->setBlob(3, &blob_stream);	//ルームID
		pre_stmt->executeUpdate();
		LOG_DEBUG("MySQL:delete unsend message(mid:{}, uid:{}, room_id:{}) success.", mid, uid, room_id);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
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

std::vector<std::string> MySQLAdapter::get_users_from_chatroom(const std::string& room_id_hex)
{
	try
	{
		// 16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);

		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());
		std::istringstream blob_stream(binary_str);
		std::vector<std::string> users;

		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("SELECT uid FROM " + _mysql_table_name_chat_members + " WHERE room_id = ?;")
		);
		pre_stmt->setBlob(1, &blob_stream);
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());

		while (res->next())
		{
			std::string uid = res->getString("uid");
			users.push_back(uid);
		}
		return users;
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
		return std::vector<std::string>();
	}
	catch (const std::runtime_error& e) {
		LOG_ERROR("Runtime Error : Failed to fetch user list from chatrrom. Error: {}", e.what());
		return std::vector<std::string>();
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return std::vector<std::string>();
	}
}

std::string MySQLAdapter::get_chatroom_name_fromDB(const std::string& room_id_hex)
{
	try
	{
		// 16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);

		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());

		std::istringstream blob_stream(binary_str);
		std::vector<std::string> users;

		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("SELECT room_name FROM " + _mysql_table_name_chatrooms + " WHERE room_id = ?;")
		);
		pre_stmt->setBlob(1, &blob_stream);
		std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());
		if (res != nullptr && res->next()) {
			std::string room_name = res->getString("room_name");
			LOG_DEBUG("MySQL:chatroom name:{}", room_name);
			return room_name;
		}
		else {
			LOG_ERROR("MySQL: no data found for room_id: {}", room_id_hex);
			return std::string();
		}
	}

	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
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

bool MySQLAdapter::delete_chatroom_fromDB(const std::string& room_id_hex)
{
	try
	{
		// 16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);

		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());

		std::istringstream blob_stream(binary_str);
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("DELETE FROM " + _mysql_table_name_chatrooms + " WHERE room_id = ?;")
		);
		pre_stmt->setBlob(1, &blob_stream);
		pre_stmt->executeUpdate();
		LOG_DEBUG("MySQL:delete chatroom(room id : {}) success.", room_id_hex);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
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

bool MySQLAdapter::update_chatroom_name(const std::string& room_id_hex, const std::string& room_name)
{
	try
	{
		// 16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);

		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());

		std::istringstream blob_stream(binary_str);
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("UPDATE " + _mysql_table_name_chatrooms +
				" SET room_name = ? WHERE room_id = ?;")
		);
		pre_stmt->setString(1, room_name);
		pre_stmt->setBlob(2, &blob_stream);
		pre_stmt->executeUpdate();
		LOG_DEBUG("MySQL:update chatroom name(room id : {}) success.", room_id_hex);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
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

bool MySQLAdapter::update_chatroom_icon(const std::string& room_id_hex, const std::string& icon_url)
{
	try
	{
		// 16進数文字列をバイナリデータに変換
		std::vector<uint8_t> room_id_bin = hex_to_binary(room_id_hex);

		// バイナリデータを string に変換（MySQL 用）
		std::string binary_str(reinterpret_cast<const char*>(room_id_bin.data()), room_id_bin.size());
	
		std::istringstream blob_stream(binary_str);
		std::unique_ptr<sql::PreparedStatement> pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("UPDATE " + _mysql_table_name_chatrooms +
				" SET room_icon_url = ? WHERE room_id = ?;")
		);
		pre_stmt->setString(1, icon_url);
		pre_stmt->setBlob(2, &blob_stream);
		pre_stmt->executeUpdate();
		LOG_DEBUG("MySQL:update chatroom icon(room id : {}) success.", room_id_hex);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
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

bool MySQLAdapter::create_table_users()
{
	try
	{
		std::unique_ptr < sql::PreparedStatement > pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("CREATE TABLE IF NOT EXISTS " + _mysql_table_name_users +
				"(uid VARCHAR(28) NOT NULL PRIMARY KEY,"
				"name VARCHAR(28) NOT NULL,"
				"mail_address VARCHAR(256) NOT NULL,"
				"icon_url VARCHAR(255),"
				"created_at DATETIME NOT NULL,"
				"last_login DATETIME);")
		);
		pre_stmt->executeUpdate();
		LOG_DEBUG("MySQL:create table(table name:{}) success.", _mysql_table_name_users);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
}

bool MySQLAdapter::create_table_chatrooms()
{
	try
	{
		std::unique_ptr < sql::PreparedStatement > pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("CREATE TABLE IF NOT EXISTS " + _mysql_table_name_chatrooms +
				"(room_id BINARY(16) NOT NULL,"
				"room_name VARCHAR(50) NOT NULL,"
				"room_icon_url VARCHAR(255),"
				"created_at DATETIME,"
				"PRIMARY KEY (room_id));")
		);
		pre_stmt->executeUpdate();
		LOG_DEBUG("MySQL:create table(table name:{}) success.", _mysql_table_name_chatrooms);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
}


bool MySQLAdapter::create_table_chat_members()
{
	try
	{
		std::unique_ptr < sql::PreparedStatement > pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("CREATE TABLE IF NOT EXISTS " + _mysql_table_name_chat_members +
				"(room_id BINARY(16) NOT NULL,"
				"uid VARCHAR(28) NOT NULL,"
				"joined_at DATETIME,"
				"PRIMARY KEY(room_id, uid),"
				"FOREIGN KEY(room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE, "
				"FOREIGN KEY (uid) REFERENCES users(uid) ON DELETE CASCADE);")
		);
		pre_stmt->executeUpdate();
		LOG_DEBUG("MySQL:create table(table name:{}) success.", _mysql_table_name_chat_members);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
}

bool MySQLAdapter::create_table_messages()
{
	try
	{
		std::unique_ptr < sql::PreparedStatement > pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("CREATE TABLE IF NOT EXISTS " + _mysql_table_name_messages +
				"(mid BIGINT PRIMARY KEY,"
				"room_id BINARY(16) NOT NULL,"
				"sender_uid VARCHAR(28) NOT NULL,"
				"send_time DATETIME NOT NULL,"
				"message JSON NOT NULL,"
				"room_sequence BIGINT UNSIGNED NOT NULL DEFAULT 0,"
				"FOREIGN KEY (room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE,"
				"FOREIGN KEY (sender_uid) REFERENCES users(uid) ON DELETE CASCADE);")
		);
		pre_stmt->executeUpdate();
		LOG_DEBUG("MySQL:create table(table name:{}) success.", _mysql_table_name_messages);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
}

bool MySQLAdapter::create_table_unsend_messages()
{
	try
	{
		std::unique_ptr < sql::PreparedStatement > pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("CREATE TABLE IF NOT EXISTS " + _mysql_table_name_unsend_messages +
				"(mid BIGINT NOT NULL,"
				"uid VARCHAR(28) NOT NULL,"
				"room_id BINARY(16) NOT NULL,"
				"PRIMARY KEY(mid, uid, room_id),"
				"FOREIGN KEY (mid) REFERENCES messages(mid) ON DELETE CASCADE ON UPDATE CASCADE,"
				"FOREIGN KEY (uid) REFERENCES users(uid) ON DELETE CASCADE,"
				"FOREIGN KEY (room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE);")
		);
		pre_stmt->executeUpdate();
		LOG_DEBUG("MySQL:create table(table name:{}) success.", _mysql_table_name_unsend_messages);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
}

bool MySQLAdapter::create_table_unsend_control_messages()
{
	try
	{
		std::unique_ptr<sql::PreparedStatement > pre_stmt(
			this->_mysql_helper->get_connection()->prepareStatement("CREATE TABLE IF NOT EXISTS " + _mysql_table_name_unsend_control_messages +
				"(mid BIGINT NOT NULL,"
				"uid VARCHAR(28) NOT NULL,"
				"room_id BINARY(16) NOT NULL,"
				"json_payload JSON NOT NULL,"
				"request_time DATETIME NOT NULL,"
				"PRIMARY KEY(mid, uid, room_id),"
				"FOREIGN KEY (mid) REFERENCES messages(mid) ON DELETE CASCADE ON UPDATE CASCADE,"
				"FOREIGN KEY (uid) REFERENCES users(uid) ON DELETE CASCADE,"
				"FOREIGN KEY (room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE);")
		);
		pre_stmt->executeUpdate();
		LOG_DEBUG("MySQL:create table(table name:{}) success.", _mysql_table_name_unsend_control_messages);
		return true;
	}
	catch (const sql::SQLException& e)
	{
		this->_mysql_helper->printError(e);
		return false;
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
}

std::vector<uint8_t> MySQLAdapter::hex_to_binary(const std::string& hex_str)
{
	std::vector<uint8_t> binary_data;

	for (size_t i = 0; i < hex_str.length(); i += 2) {
		std::string byte_str = hex_str.substr(i, 2);
		uint8_t byte = static_cast<uint8_t>(std::stoi(byte_str, nullptr, 16));
		binary_data.push_back(byte);
	}

	return binary_data;
}

std::string MySQLAdapter::to_hex_string(const std::string &bin_str)
{
    std::ostringstream oss;
    for (unsigned char c : bin_str) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    return oss.str();
}