#ifndef CHATROOMIDGENERATOR_HPP
#define CHATROOMIDGENERATOR_HPP
#include "Utils/common.hpp"

/**
 * @brief チャットルームID生成専用クラス
 * 
 * ULIDベースの一意なチャットルームIDを生成します。
 * MessageIDGeneratorと同様のパターンで実装されています。
 */
class ChatroomIDGenerator {
public:
    ChatroomIDGenerator();
    ~ChatroomIDGenerator();

    /**
     * @brief ULIDベースのチャットルームIDを生成（Base32形式）
     * @return 生成されたチャットルームID
     */
    static std::string issue_chatroom_id();

    /**
     * @brief ULIDベースのチャットルームIDを生成（Hex形式）
     * @return 生成されたチャットルームID（16進数形式）
     */
    static std::string issue_chatroom_id_unhex();

private:
    static std::mutex mtx;
    static uint64_t last_timestamp;
    
    // ULIDヘルパー関数
    static std::string generate_ulid_base32();
    static std::string convert_ulid_to_hex(const std::string& ulid_str);
};

#endif // !CHATROOMIDGENERATOR_HPP