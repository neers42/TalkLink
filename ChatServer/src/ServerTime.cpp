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

    char timeStr[100];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &time_info);

    // ミリ秒を取り出して表示
    int millisecondsPart = milliseconds % 1000;
    
    std::string ss;
    ss = "[" + std::string(timeStr) + "." + std::to_string(millisecondsPart) + "]";

    return ss;


    // 現在の日付と時刻を表示
    //std::cout << "[" << timeStr << "." << millisecondsPart << "]" << std::endl;

}

