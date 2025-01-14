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
			os << StringFormat("[INFO] %s, thread id : %d, %s", ServerTime::get_current_time(), std::this_thread::get_id(), log) << std::endl;
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
		LogHandler::logging(LogLevel::CRITICAL, std::cerr, critical_message);
	}
	else {
		//ログレベルがCRITICAL以上に設定されていないと何も出力しない。
	}
}

bool LogHandler::is_ASCII(const std::string& str)
{
	//ASCII文字は0x00から0x7Fの範囲の値を持つため、範囲内であればASCII文字コードとなる
	for (char c : str) {
		if (static_cast<unsigned char>(c) > 0x7F) {
			return false;
		}
	}
	return true;
}

bool LogHandler::is_valid_UTF8(const std::string& str)
{
	//1文字が1バイトから最大4バイトである場合はUTF-8でエンコードされている
	int count = 0;
	for (unsigned char c : str) {
		if (count == 0) {
			if ((c >> 5) == 0x06) { count = 1; } // 2バイト文字
			else if ((c >> 4) == 0x0E) { count = 2; } // 3バイト文字
			else if ((c >> 3) == 0x1E) { count = 3; } // 4バイト文字
			else if ((c >> 7)) { return false; } // UTF-8ではない
		}
		else {
			if ((c >> 6) != 0x02) { return false; } // 継続バイトのチェック
			--count;
		}
	}
	return count == 0;
}

bool LogHandler::is_ShiftJIS(const std::string& str)
{
	for (size_t i = 0; i < str.length(); ++i) {
		unsigned char c = static_cast<unsigned char>(str[i]);
		// Shift-JISの1バイト目の範囲チェック
		if ((c >= 0x81 && c <= 0x9F) || (c >= 0xE0 && c <= 0xEF)) {
			// Shift-JISの2バイト目の範囲チェック
			if (i + 1 < str.length()) {
				unsigned char next = static_cast<unsigned char>(str[i + 1]);
				if ((next >= 0x40 && next <= 0x7E) || (next >= 0x80 && next <= 0xFC)) {
					i++; // 2バイト文字なのでインデックスを進める
				}
				else {
					return false;
				}
			}
			else {
				return false; // 文字列の途中で終了している
			}
		}
		else if (c < 0x20 || c > 0x7E) {
			// 制御文字やASCII範囲外は不正
			return false;
		}
	}
	return true;
}

int LogHandler::check_char_code(const std::string& str)
{
	if (is_ASCII(str)) {
		return ASCII_CODE;
	}
	else if (is_valid_UTF8(str)) {
		return UTF_8_CODE;
	}
	else if (is_ShiftJIS(str)) {
		return SHIFT_JIS_CODE;
	}
	else {
		return -1;
	}
}


