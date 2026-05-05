#include "ServerTime.hpp"
std::tm ServerTime::time_info;
std::time_t ServerTime::current_time;

time_t ServerTime::get_current_unix_time() const
{
    try {
        auto now = std::chrono::system_clock::now();
        return std::chrono::system_clock::to_time_t(now);
    } catch (const std::exception& e) {
        return std::time(nullptr); // フォールバック
    }
}
