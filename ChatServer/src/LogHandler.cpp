#include "LogHandler.h"
std::mutex LogHandler::cout_mutex;

LogHandler::LogHandler()
{
}

LogHandler::~LogHandler()
{
}

void LogHandler::logging_info(LogType log_type, std::ostream& os, const std::string& log_message)
{
	{
		std::unique_lock<std::mutex> lock(cout_mutex);
		switch (log_type)
		{
		case LogHandler::LogType::INFO:
			os << StringFormat("%s[INFO]", ServerTime::get_current_time()) << log_message << std::endl;
			break;
		case LogHandler::LogType::SERVER_ERROR:
			os << StringFormat("%s[ERROR]", ServerTime::get_current_time()) << log_message << std::endl;
			break;
		case LogHandler::LogType::RECEIVE:
		os << StringFormat("%s[RECEIVE]", ServerTime::get_current_time()) << log_message << std::endl;
			break;
		case LogHandler::LogType::SEND:
			os << StringFormat("%s[SEND]", ServerTime::get_current_time()) << log_message << std::endl;
			break;

		case LogHandler::LogType::SIGN_UP:
			os << StringFormat("%s[SIGN UP]", ServerTime::get_current_time()) << log_message << std::endl;
			break;
	
		case LogHandler::LogType::SIGN_IN:
			os << StringFormat("%s[SIGN IN]", ServerTime::get_current_time()) << log_message << std::endl;
			break;
		default:
			break;
		}
	}
}

