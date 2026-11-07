#ifndef CHATROOMSEQUENCEHANDLER_HPP
#define CHATROOMSEQUENCEHANDLER_HPP

#include "Utils/common.hpp"

class IRoomSequenceRepository;

class ChatroomSequenceHandler
{
public:
    explicit ChatroomSequenceHandler(std::shared_ptr<IRoomSequenceRepository> redis_adapter);
    ~ChatroomSequenceHandler() = default;

    bool initialize_room_sequence(const std::string& room_id, std::uint64_t initial_value = 0);
    bool delete_room_sequence(const std::string& room_id);
    std::uint64_t get_current_sequence(const std::string& room_id) const;
    bool increment_room_sequence(const std::string& room_id, std::uint64_t& next_sequence);

private:
    std::shared_ptr<IRoomSequenceRepository> _redis_adapter;

    static std::string build_sequence_key(const std::string& room_id);
    static bool is_valid_room_id(const std::string& room_id);
};

#endif // CHATROOMSEQUENCEHANDLER_HPP