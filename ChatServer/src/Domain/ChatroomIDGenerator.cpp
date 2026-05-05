#include "Domain/ChatroomIDGenerator.hpp"

std::mutex ChatroomIDGenerator::mtx;
uint64_t ChatroomIDGenerator::last_timestamp = 0;

ChatroomIDGenerator::ChatroomIDGenerator() {
}

ChatroomIDGenerator::~ChatroomIDGenerator() {
}

std::string ChatroomIDGenerator::issue_chatroom_id() {
    std::lock_guard<std::mutex> lock(mtx);
    return generate_ulid_base32();
}

std::string ChatroomIDGenerator::issue_chatroom_id_unhex() {
    std::lock_guard<std::mutex> lock(mtx);
    std::string ulid_str = generate_ulid_base32();
    return convert_ulid_to_hex(ulid_str);
}

std::string ChatroomIDGenerator::generate_ulid_base32() {
    // ランダムバイトジェネレータを作成
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);

    // ミリ秒単位の現在時刻
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    // ランダムバイト生成関数
    auto random_func = [&]() -> uint8_t {
        return static_cast<uint8_t>(dist(gen));
    };

    // ULIDを作成
    ulid::ULID ulid = ulid::Create(now, random_func);

    // 文字列に変換して返す
    return ulid::Marshal(ulid);
}

std::string ChatroomIDGenerator::convert_ulid_to_hex(const std::string& ulid_str) {
    auto decode_ulid = base32::decode(ulid_str);
    std::ostringstream oss;
    for (auto b : decode_ulid) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    LOG_INFO("Create Chatroom ID: {}", oss.str());
    return oss.str();
}