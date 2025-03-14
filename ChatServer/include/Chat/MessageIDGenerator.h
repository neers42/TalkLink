#ifndef MESSAGEIDGENERATOR
#define MESSAGEIDGENERATOR
#include "common.h"
#include "spdlog/spdlog.h"

class MessageIDGenerator
{
public:
	MessageIDGenerator();
	~MessageIDGenerator();

    uint64_t generate_message_id();

private:
    static constexpr uint64_t epoch = 1700000000000; // 任意の開始時間（2023年11月以降）
    static constexpr uint64_t machine_id = 1; // マシンID（環境に応じて変更）
    static constexpr uint64_t machine_bits = 10;
    static constexpr uint64_t sequence_bits = 12;
    static constexpr uint64_t max_sequence = (1 << sequence_bits) - 1;

    uint64_t last_timestamp = 0;
    uint64_t sequence = 0;

};
#endif // !MESSAGEIDGENERATOR



