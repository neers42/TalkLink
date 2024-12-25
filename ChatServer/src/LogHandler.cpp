#include "LogHandler.h"
std::mutex LogHandler::cout_mutex;

LogHandler::LogHandler() : log_level{10}
{

}

LogHandler::~LogHandler()
{
}

void LogHandler::logging(LogType log_type, std::ostream& os, const std::string& log_message)
{
	{
		std::unique_lock<std::mutex> lock(cout_mutex);
		switch (log_type)
		{
		case LogHandler::LogType::INFO:
			os << StringFormat("[%s][INFO]", ServerTime::get_current_time()) << log_message << std::endl;
			break;
		case LogHandler::LogType::SERVER_ERROR:
			os << StringFormat("[%s]\033[31m[ERROR]\033[m", ServerTime::get_current_time()) << log_message << std::endl;
			break;
		case LogHandler::LogType::RECEIVE:
		os << StringFormat("[%s]\033[32m[RECEIVE]\033[m", ServerTime::get_current_time()) << log_message << std::endl;
			break;
		case LogHandler::LogType::SEND:
			os << StringFormat("[%s]\033[32m[SEND]\033[m", ServerTime::get_current_time()) << log_message << std::endl;
			break;

		case LogHandler::LogType::SIGN_UP:
			os << StringFormat("[%s][SIGN UP]", ServerTime::get_current_time()) << log_message << std::endl;
			break;
	
		case LogHandler::LogType::SIGN_IN:
			os << StringFormat("[%s][SIGN IN]", ServerTime::get_current_time()) << log_message << std::endl;
			break;

		case LogType::SIGN_OUT:
			os << StringFormat("[%s][SIGN OUT]", ServerTime::get_current_time()) << log_message << std::endl;
			break;
		case LogType::DEBUG_INFO:
			os << StringFormat("[%s][DEBUG INFO]", ServerTime::get_current_time()) << log_message << std::endl;
			break;
		default:
			break;
		}
	}
}

void LogHandler::logging(LogLevel level, std::ostream& os, const std::string& log)
{
	{
		std::unique_lock<std::mutex> lock(cout_mutex);
		switch (level)
		{
		case LogHandler::LogLevel::DEBUG:
			os << StringFormat("[DEBUG] %s, thread id : %d, %s", ServerTime::get_current_time(), std::this_thread::get_id(), log) << std::endl;
			break;
		case LogHandler::LogLevel::INFO:
			os << StringFormat("[INFO] %s, thread id : %d ,%s", ServerTime::get_current_time(), std::this_thread::get_id(), log) << std::endl;
			break;
		case LogHandler::LogLevel::WARNING:
			os << StringFormat("\033[33m[WARNING] %s, thread id : %d, %s\033[m", ServerTime::get_current_time(), std::this_thread::get_id(), log) << std::endl;
			break;
		case LogHandler::LogLevel::PROGRAM_ERROR:
			os << StringFormat("\033[31m[ERROR] %s, thread id : %d, %s\033[m", ServerTime::get_current_time(), std::this_thread::get_id(), log) << std::endl;
			break;
		case LogHandler::LogLevel::CRITICAL:
			os << StringFormat("\033[31m[CRITICAL] %s, thread id : %d, %s\033[m", ServerTime::get_current_time(), std::this_thread::get_id(), log) << std::endl;
			break;
		default:
			break;
		}
	}
}

void LogHandler::logging_debug(const std::string& debug_message)
{
	//ログレベルがDEBUG以上に設定されていた場合はログを出力する。
	if (this->log_level <= static_cast<uint8_t>(LogLevel::DEBUG)) {
		LogHandler::logging(LogLevel::DEBUG, std::cout, debug_message);
	}
	else {
		//ログレベルがDEBUG以上に設定されていないと何も出力しない。
	}
}

void LogHandler::logging_info(const std::string& info_message)
{
	//ログレベルがDEBUG以上に設定されていた場合はログを出力する。
	if (this->log_level <= static_cast<uint8_t>(LogLevel::INFO)) {
		LogHandler::logging(LogLevel::INFO, std::cout, info_message);
	}
	else {
		//ログレベルがDEBUG以上に設定されていないと何も出力しない。
	}
}

void LogHandler::logging_warning(const std::string& warning_message)
{
	//ログレベルがINFO以上に設定されていた場合はログを出力する。
	if (this->log_level <= static_cast<uint8_t>(LogLevel::WARNING)) {
		LogHandler::logging(LogLevel::WARNING, std::cout, warning_message);
	}
	else {
		//ログレベルがINFO以上に設定されていないと何も出力しない。
	}
}

void LogHandler::logging_error(const std::string& error_message)
{
	//ログレベルがPROGRAM_ERROR以上に設定されていた場合はログを出力する。
	if (this->log_level <= static_cast<uint8_t>(LogLevel::PROGRAM_ERROR)) {
		LogHandler::logging(LogLevel::PROGRAM_ERROR, std::cerr, error_message);
	}
	else {
		//ログレベルがPROGRAM_ERROR以上に設定されていないと何も出力しない。
	}
}

void LogHandler::logging_critical(const std::string& critical_message)
{
	//ログレベルがCRITICAL以上に設定されていた場合はログを出力する。
	if (this->log_level <= static_cast<uint8_t>(LogLevel::CRITICAL)) {
		LogHandler::logging(LogLevel::DEBUG, std::cerr, critical_message);
	}
	else {
		//ログレベルがCRITICAL以上に設定されていないと何も出力しない。
	}
}


