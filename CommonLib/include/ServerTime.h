#ifndef SERVERTIME_H_
#define SERVERTIME_H_

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

        time(&current_time);
        LOCALTIME_SAFE(&time_info, &current_time);
        //localtime_s(&time_info, &current_time);

        // 現在の時刻を取得
        auto now = std::chrono::system_clock::now();

        // 現在の時刻をミリ秒単位で表現
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

        // 現在の時刻を文字列に変換
        current_time = std::chrono::system_clock::to_time_t(now);


        char time_str[100];
        std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &time_info);

        // ミリ秒を取り出して表示
        int milliseconds_part = milliseconds % 1000;

        // ミリ秒の桁数を合わせるために左に0を埋める
        std::string milliseconds_str = std::to_string(milliseconds_part);
        milliseconds_str = std::string(3 - milliseconds_str.length(), '0') + milliseconds_str;

        std::string ss;
        ss = std::string(time_str) + "." + milliseconds_str;

        return ss;

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
	
};

#endif // !SERVERTIME_H_


