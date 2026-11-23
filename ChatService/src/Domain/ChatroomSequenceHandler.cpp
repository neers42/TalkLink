#include "Domain/ChatroomSequenceHandler.hpp"
#include "Domain/Interfaces/IRoomSequenceRepository.hpp"

ChatroomSequenceHandler::ChatroomSequenceHandler(std::shared_ptr<IRoomSequenceRepository> redis_adapter)
    : _redis_adapter(std::move(redis_adapter))
{
    if (!_redis_adapter) {
        throw std::invalid_argument("ChatroomSequenceHandler requires a RedisAdapter");
    }
}

bool ChatroomSequenceHandler::initialize_room_sequence(const std::string& room_id, std::uint64_t initial_value)
{
    if (!is_valid_room_id(room_id)) {
        LOG_ERROR("[ChatroomSequenceHandler] Invalid room id for initialization");
        return false;
    }

    return _redis_adapter->initialize_counter_key(build_sequence_key(room_id), initial_value);
}

bool ChatroomSequenceHandler::delete_room_sequence(const std::string& room_id)
{
    if (!is_valid_room_id(room_id)) {
        LOG_ERROR("[ChatroomSequenceHandler] Invalid room id for deletion");
        return false;
    }

    return _redis_adapter->delete_key(build_sequence_key(room_id));
}

std::uint64_t ChatroomSequenceHandler::get_current_sequence(const std::string& room_id) const
{
    if (!is_valid_room_id(room_id)) {
        LOG_ERROR("[ChatroomSequenceHandler] Invalid room id for lookup");
        return 0;
    }

    return _redis_adapter->get_counter_value(build_sequence_key(room_id));
}

bool ChatroomSequenceHandler::increment_room_sequence(const std::string& room_id, std::uint64_t& next_sequence)
{
    next_sequence = 0;
    if (!is_valid_room_id(room_id)) {
        LOG_ERROR("[ChatroomSequenceHandler] Invalid room id for increment");
        return false;
    }

    const std::string sequence_key = build_sequence_key(room_id);
    if (!_redis_adapter->initialize_counter_key(sequence_key, 0)) {
        LOG_ERROR("[ChatroomSequenceHandler] Failed to initialize sequence key for room {}", room_id);
        return false;
    }

    return _redis_adapter->increment_counter_key(sequence_key, next_sequence);
}

std::string ChatroomSequenceHandler::build_sequence_key(const std::string& room_id)
{
    return "chatroom_sequence:" + room_id;
}

bool ChatroomSequenceHandler::is_valid_room_id(const std::string& room_id)
{
    return !room_id.empty();
}
