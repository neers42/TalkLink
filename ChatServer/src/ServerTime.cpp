#include "ServerTime.h"

ServerTime::ServerTime()
{

}

ServerTime::~ServerTime()
{
}

std::string ServerTime::get_current_time()
{

    time(&current_time);
    localtime_s(&time_info, &current_time);

    // 現在の時刻を取得
    auto now = std::chrono::system_clock::now();

    // 現在の時刻をミリ秒単位で表現
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    // 現在の時刻を文字列に変換
    current_time = std::chrono::system_clock::to_time_t(now);

    //timeInfo = std::localtime_s(&currentTime);

    char time_str[100];
    std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &time_info);

    // ミリ秒を取り出して表示
    int milliseconds_part = milliseconds % 1000;

    // ミリ秒の桁数を合わせるために左に0を埋める
    std::string milliseconds_str = std::to_string(milliseconds_part);
    milliseconds_str = std::string(3 - milliseconds_str.length(), '0') + milliseconds_str;
    
    std::string ss;
    ss = "[" + std::string(time_str) + "." + milliseconds_str + "]";

    return ss;


    // 現在の日付と時刻を表示
    //std::cout << "[" << timeStr << "." << millisecondsPart << "]" << std::endl;

}

