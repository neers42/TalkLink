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
	
	static std::mutex cout_mutex;
	int log_level;

	LogHandler();
	~LogHandler();


public:
	enum class LogLevel
	{	DEBUG = 10,
		INFO = 20,
		WARNING = 30,
		PROGRAM_ERROR = 40,
		CRITICAL = 50
	};
	
	static LogHandler& get_instance() {
		static LogHandler logger;
		return logger;
	}

	//コピーコンストラクタを削除
	LogHandler(const LogHandler&) = delete;

	//代入演算子を削除
	LogHandler& operator=(const LogHandler&) = delete;

	int get_log_level() { return this->log_level; }
	void set_log_level(int level) { this->log_level = level;}

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



