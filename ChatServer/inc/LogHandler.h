#ifndef LOGHANDLER_H_
#define LOGHANDLER_H_

#include <iostream>
#include <string>
#define NC "\e[0m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define CYN "\e[0;36m"
#define REDB "\e[41m"

class LogHandler
{
	enum class LogLebel { INFO, SERVER_ERROR, RECEIVE, SEND };

public:
	LogHandler();
	~LogHandler();

	static void logging_info();

};

#endif // !LOGHANDLER_H_



