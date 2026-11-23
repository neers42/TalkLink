#include "Domain/MessageIDGenerator.hpp"
#include "EnvHelper.hpp"
uint64_t MessageIDGenerator::last_timestamp = 0;
uint64_t MessageIDGenerator::sequence = 0;
std::mutex MessageIDGenerator::mtx;
MessageIDGenerator::MessageIDGenerator()
{
}

MessageIDGenerator::~MessageIDGenerator()
{
}

uint64_t MessageIDGenerator::resolve_machine_id()
{
	try
	{
		const std::string value = get_env("CHAT_SERVICE_MACHINE_ID");
		unsigned long long parsed = std::stoull(value);
		constexpr uint64_t max_machine_id = (1ULL << machine_bits) - 1;
		if (parsed > max_machine_id)
		{
			LOG_ERROR("[MessageIDGenerator] CHAT_SERVICE_MACHINE_ID ({}) exceeds the maximum allowed value ({}).", parsed, max_machine_id);
			exit(1);
		}
		return static_cast<uint64_t>(parsed);
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("[MessageIDGenerator] Failed to resolve CHAT_SERVICE_MACHINE_ID: {}", e.what());
		exit(1);
	}
}

uint64_t MessageIDGenerator::generate_message_id()
{
	// 初回呼び出し時に一度だけ環境変数から解決する(スレッドセーフな遅延初期化)
	static const uint64_t machine_id = resolve_machine_id();

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
