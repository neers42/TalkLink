#ifndef IROOMSEQUENCEREPOSITORY_HPP
#define IROOMSEQUENCEREPOSITORY_HPP
#include <string>
#include <cstdint>

// ルームのシーケンス番号カウンター(Redis)アクセスの抽象化。テストでは実装
// (RedisAdapter)を差し替えて、実際のRedisサーバーに接続せずに
// ChatroomSequenceHandlerのロジックをテストできる。
class IRoomSequenceRepository
{
public:
    virtual ~IRoomSequenceRepository() = default;
    virtual bool initialize_counter_key(const std::string& key, std::uint64_t initial_value = 0) = 0;
    virtual bool increment_counter_key(const std::string& key, std::uint64_t& next_value) = 0;
    virtual std::uint64_t get_counter_value(const std::string& key) = 0;
    virtual bool delete_key(const std::string& key) = 0;
};

#endif // IROOMSEQUENCEREPOSITORY_HPP
