#ifndef LOGHANDLER_H_
#define LOGHANDLER_H_

#include <iostream>
#include <string>
#include <mutex>
#include "ServerTime.h"
#include "StringFormat.h"
#define NC "\e[0m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define CYN "\e[0;36m"
#define REDB "\e[41m"

class LogHandler
{
	enum class LogLevel { DEBUG = 10, INFO = 20, WARNING = 30, PROGRAM_ERROR = 40, CRITICAL = 50 };
	static std::mutex cout_mutex;
	uint8_t log_level;

public:
	LogHandler();
	~LogHandler();
	uint8_t get_log_level() { return this->log_level; }
	void set_log_level(LogHandler::LogLevel level) { this->log_level = static_cast<uint8_t>(level); }

	enum class LogType {INFO, SERVER_ERROR, RECEIVE, SEND, SIGN_UP, SIGN_IN, SIGN_OUT, DEBUG_INFO};
	
	
	static void logging(LogType log_type, std::ostream& os, const std::string& log_message);
	static void logging(LogLevel level, std::ostream& os, const std::string& log);
	void logging_debug(const std::string& debug_message);
	void logging_info(const std::string& info_message);
	void logging_warning(const std::string& warning_message);
	void logging_error(const std::string& error_message);
	void logging_critical(const std::string& critical_message);
	
};

#endif // !LOGHANDLER_H_



