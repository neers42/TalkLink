#include "Database/MySQLHelper.hpp"

#ifndef _WIN32
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#endif

namespace {
std::string build_mysql_endpoint(const char* host, const char* port)
{
	const char* resolved_host = (host != nullptr && host[0] != '\0') ? host : "127.0.0.1";
	const char* resolved_port = (port != nullptr && port[0] != '\0') ? port : "3306";
	return std::string("tcp://") + resolved_host + ":" + resolved_port;
}

#ifndef _WIN32
std::string resolve_mysql_endpoint(const std::string& endpoint)
{
	constexpr const char* prefix = "tcp://";
	if (endpoint.rfind(prefix, 0) != 0) {
		return endpoint;
	}

	const std::string host_and_port = endpoint.substr(std::char_traits<char>::length(prefix));
	const std::size_t separator_pos = host_and_port.rfind(':');
	if (separator_pos == std::string::npos) {
		return endpoint;
	}

	const std::string host = host_and_port.substr(0, separator_pos);
	const std::string port = host_and_port.substr(separator_pos + 1);
	if (host.empty() || port.empty()) {
		return endpoint;
	}

		addrinfo hints{};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	addrinfo* result = nullptr;
	const int status = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
	if (status != 0 || result == nullptr) {
		spdlog::warn("MySQL host resolution failed for {}: {}", host, gai_strerror(status));
		return endpoint;
	}

	char address_buffer[INET_ADDRSTRLEN] = {};
	const auto* sockaddr = reinterpret_cast<sockaddr_in*>(result->ai_addr);
	const char* address = inet_ntop(AF_INET, &(sockaddr->sin_addr), address_buffer, sizeof(address_buffer));
	freeaddrinfo(result);
	if (address == nullptr) {
		return endpoint;
	}

	return std::string(prefix) + address + ":" + port;
}
#endif
}

MySQLHelper::MySQLHelper()
{
    char* env_host = nullptr;
	char* env_user = nullptr;
	char* env_pass = nullptr;
	char* env_db = nullptr;
	char* env_port = nullptr;
#ifdef _WIN32
	size_t len = 0;

	if (_dupenv_s(&env_host, &len, "DB_HOST") == 0 && env_host != nullptr) {
		spdlog::debug("DB_HOST: {}", env_host);
	}
	else {
		LOG_ERROR("DB_HOST environment variable is not set.");
	}
	if(_dupenv_s(&env_user, &len, "DB_USER") == 0 && env_user != nullptr) {
		spdlog::debug("DB_USER: {}", env_user);
	}
	else {
		LOG_ERROR("DB_USER environment variable is not set.");
	}
	if(_dupenv_s(&env_pass, &len, "DB_PASSWORD") == 0 && env_pass != nullptr) {
		spdlog::debug("DB_PASSWORD: {}", env_pass);
	}else {
		LOG_ERROR("DB_PASSWORD environment variable is not set.");
	}
	if(_dupenv_s(&env_db, &len, "DB_NAME") == 0 && env_db != nullptr) {
		spdlog::debug("DB_NAME: {}", env_db);
	}else {
		LOG_ERROR("DB_NAME environment variable is not set.");
	}
	if(_dupenv_s(&env_port, &len, "DB_PORT") == 0 && env_port != nullptr) {
		spdlog::debug("DB_PORT: {}", env_port);
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
#ifdef _WIN32
	free(env_host);
	free(env_user);
	free(env_pass);
	free(env_db);
	free(env_port);
#endif // _WIN32
}

MySQLHelper::MySQLHelper(const std::string& host, const std::string& user_name, const std::string& password, const std::string& db_name)
	:_mysql_host{host}, _mysql_user_name{user_name}, _mysql_password{password}, _mysql_db_name{db_name}
{

}

MySQLHelper::~MySQLHelper() {
    disconnect();
}

bool MySQLHelper::connect(const std::string& db_name) {
    const int max_retries = 10;
	const int retry_delay_ms = 2000;  // 2 seconds
	std::string connect_target = _mysql_host;
#ifndef _WIN32
	connect_target = resolve_mysql_endpoint(_mysql_host);
	if (connect_target != _mysql_host) {
		spdlog::debug("Resolved MySQL host from {} to {}", _mysql_host, connect_target);
	}
#endif
	for (int attempt = 1; attempt <= max_retries; ++attempt)
	{
		try
		{
			_connection = _driver->connect(connect_target, _mysql_user_name, _mysql_password);
			_connection->setClientOption("characterEncoding", "utf8mb4");
			_connection->setClientOption("characterSetResults", "utf8mb4");
			_statement = _connection->createStatement();
			_statement->execute("SET NAMES utf8mb4");
			spdlog::debug("host: {}, username: {}", connect_target, _mysql_user_name);
			spdlog::info("MySQL: Connection established successfully.");
			_connection->setSchema(db_name);
			return true; // Success
		}
		catch (sql::SQLException& e)
		{
			spdlog::error("[Attempt {}] Failed to connect to MySQL: {} (Error Code: {}, SQL State: {})",
			              attempt, e.what(), e.getErrorCode(), e.getSQLState());

			if (attempt == max_retries)
			{
				spdlog::critical("Reached maximum number of connection attempts. Terminating the application.");
				exit(1);
			}

			spdlog::warn("Retrying to connect in {} ms...", retry_delay_ms);
			std::this_thread::sleep_for(std::chrono::milliseconds(retry_delay_ms));
		}
	}
    return false; // ここまで接続できない場合はfalseを返す
}

void MySQLHelper::disconnect() {
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

bool MySQLHelper::executeQuery(const std::string& query) {
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
		LOG_ERROR("Runtime Error:{}.",e.what());
		return false;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Exception:{}.", e.what());
		return false;
	}
	return true;
}

void MySQLHelper::printError(const sql::SQLException& e) {
    LOG_ERROR("SQLException Content:{}, SQL Error Code:{}, SQL State:{}", e.what(), e.getErrorCode(), e.getSQLState());
}
