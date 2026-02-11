#ifndef SERVERTIME_HPP
#define SERVERTIME_HPP
#include <chrono>
#include <iostream>
#include <string>
#include <ctime>
#include "StringFormat.h"
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#ifdef _WIN32
    #define LOCALTIME_SAFE(tm, time) localtime_s((tm), (time))
#else
    #define LOCALTIME_SAFE(tm, time) localtime_r((time), (tm))
#endif
class ServerTime
{
public:
	static std::time_t current_time;

	static std::tm time_info;

	int millisecondsPart;

public:
	ServerTime(){}
	~ServerTime() {}


    static std::string get_current_time_to_milliseconds()
    {
        // 現在の時刻を取得
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        
        // ミリ秒部分を取得
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        // ローカル時間に変換
        std::tm tm;
        LOCALTIME_SAFE(&tm, &time_t_now);
        
        // ISO 8601形式の文字列を作成
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
        oss << "." << std::setfill('0') << std::setw(7) << (milliseconds.count() * 10000); // 7桁のフラクション秒
        oss << "+09:00"; // 日本標準時のタイムゾーン
        
        return oss.str();
    }

    static std::string get_current_time_to_seconds()
    {
        // 現在時刻を取得してDATETIME形式に変換
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
        LOCALTIME_SAFE(&tm, &now_c);
        //localtime_s(&tm, &now_c);  // 安全な関数 localtime_s を使用

        std::ostringstream datetime_ss;
        datetime_ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        std::string datetime_to_seconds = datetime_ss.str();
        return datetime_to_seconds;
    }

    // ISO 8601形式からMySQL DATETIME形式に変換
    static std::string iso8601_to_mysql_datetime(const std::string& iso_timestamp)
    {
        if (iso_timestamp.empty()) {
            return get_current_time_to_seconds();
        }

        try {
            // "2025-12-19T22:36:27.0733540+09:00" -> "2025-12-19 22:36:27.073354"
            std::string result = iso_timestamp;
            
            // 'T'をスペースに置換
            size_t t_pos = result.find('T');
            if (t_pos != std::string::npos) {
                result[t_pos] = ' ';
            }
            
            // タイムゾーン部分を削除（+09:00 または Z）
            size_t tz_pos = result.find_last_of("+-Z");
            if (tz_pos != std::string::npos && tz_pos > 10) { // 日付部分の'-'を除外
                result = result.substr(0, tz_pos);
            }
            
            // フラクション秒を6桁に調整（MySQLのDATETIME(6)対応）
            size_t dot_pos = result.find('.');
            if (dot_pos != std::string::npos) {
                std::string fraction = result.substr(dot_pos + 1);
                if (fraction.length() > 6) {
                    result = result.substr(0, dot_pos + 1) + fraction.substr(0, 6);
                } else if (fraction.length() < 6) {
                    result = result.substr(0, dot_pos + 1) + fraction + std::string(6 - fraction.length(), '0');
                }
            }
            
            return result;
        } catch (const std::exception& e) {
            return get_current_time_to_seconds();
        }
    }

    // MySQL DATETIME形式からISO 8601形式に変換
    static std::string mysql_datetime_to_iso8601(const std::string& mysql_timestamp)
    {
        if (mysql_timestamp.empty()) {
            return get_current_time_to_milliseconds();
        }

        try {
            // "2025-12-19 22:36:27.073354" -> "2025-12-19T22:36:27.0733540+09:00"
            std::string result = mysql_timestamp;
            
            // スペースを'T'に置換
            size_t space_pos = result.find(' ');
            if (space_pos != std::string::npos) {
                result[space_pos] = 'T';
            }
            
            // フラクション秒を7桁に調整
            size_t dot_pos = result.find('.');
            if (dot_pos != std::string::npos) {
                std::string fraction = result.substr(dot_pos + 1);
                if (fraction.length() == 6) {
                    result = result.substr(0, dot_pos + 1) + fraction + "0";
                }
            } else {
                result += ".0000000";
            }
            
            // タイムゾーンを追加
            result += "+09:00";
            
            return result;
        } catch (const std::exception& e) {
            return get_current_time_to_milliseconds();
        }
    }
	
};

#endif // !SERVERTIME_HPP


