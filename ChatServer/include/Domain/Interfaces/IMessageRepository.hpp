#ifndef IMESSAGEREPOSITORY_HPP
#define IMESSAGEREPOSITORY_HPP

class IMessageRepository
{
public:
    virtual ~IMessageRepository() = default;

    virtual bool insert_message(
        uint64_t mid,
        const std::string& room_id,
        const std::string& sender_uid,
        const std::string& send_time,
        const std::string& message_content,
        uint64_t room_sequence = 0,
        const std::string& target_uid = ""
    ) = 0;

    virtual json get_unreceived_messages(
        const std::string& uid,
        uint64_t last_received_message_id
    ) = 0;
};

#endif // IMESSAGEREPOSITORY_HPP