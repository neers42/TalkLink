#ifndef MESSAGEIDGENERATOR_HPP
#define MESSAGEIDGENERATOR_HPP
#include "Utils/common.hpp"

class MessageIDGenerator
{
public:
	MessageIDGenerator();
	~MessageIDGenerator();

    static uint64_t generate_message_id();

    // 環境変数CHAT_SERVER_MACHINE_IDから取得するマシンID。
    // 複数インスタンスで稼働させる場合、ID衝突を避けるため各インスタンスで
    // 一意な値を設定する必要がある。
    // サーバー起動時に一度呼び出し、設定不備を早期に検出するためpublicにしている。
    static uint64_t resolve_machine_id();

private:
    static constexpr uint64_t epoch = 1700000000000; // 任意の開始時間（2023年11月以降）
    static constexpr uint64_t machine_bits = 10;
    static constexpr uint64_t sequence_bits = 12;
    static constexpr uint64_t max_sequence = (1 << sequence_bits) - 1;

    static std::mutex mtx;
    static uint64_t last_timestamp;
    static uint64_t sequence;

};
#endif // !MESSAGEIDGENERATOR_HPP



