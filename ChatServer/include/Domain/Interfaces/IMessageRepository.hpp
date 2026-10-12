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

    // 指定ルーム内で、room_sequenceがfrom_sequenceより大きいメッセージを
    // room_sequence昇順で取得する(SyncRequestの欠番補充用)。
    // target_uidが設定されているメッセージはrequester_uid宛のものだけを含める。
    virtual json get_room_messages_after_sequence(
        const std::string& room_id,
        uint64_t from_sequence,
        const std::string& requester_uid
    ) = 0;
};

#endif // IMESSAGEREPOSITORY_HPP