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

public:
	LogHandler();
	~LogHandler();

	enum class LogType {INFO, SERVER_ERROR, RECEIVE, SEND, SIGN_UP, SIGN_IN, DEBUG_INFO};
	
	static void logging_info(LogType log_type, std::ostream& os, const std::string& log_message);
};

#endif // !LOGHANDLER_H_



