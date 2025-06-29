#include "Chat/MessageIDGenerator.h"
uint64_t MessageIDGenerator::last_timestamp = 0;
uint64_t MessageIDGenerator::sequence = 0;
std::mutex MessageIDGenerator::mtx;
MessageIDGenerator::MessageIDGenerator()
{
}

MessageIDGenerator::~MessageIDGenerator()
{
}

uint64_t MessageIDGenerator::generate_message_id()
{
	
	std::lock_guard<std::mutex> lock(mtx); // スレッドセーフにするためのロック

    uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch())
        .count() - epoch;

    if (timestamp == last_timestamp) {
        sequence = (sequence + 1) & max_sequence;
        if (sequence == 0) {
            while (timestamp <= last_timestamp) {
                timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count() - epoch;
            }
        }
    }
    else {
        sequence = 0;
    }

    last_timestamp = timestamp;

    return (static_cast<uint64_t>(timestamp) << (machine_bits + sequence_bits)) |
        (static_cast<uint64_t>(machine_id) << sequence_bits) |
        static_cast<uint64_t>(sequence);
}
