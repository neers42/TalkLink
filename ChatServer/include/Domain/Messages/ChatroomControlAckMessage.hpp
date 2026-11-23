#ifndef CHATROOMCONTROLACKMESSAGE_HPP
#define CHATROOMCONTROLACKMESSAGE_HPP
#include "MessageBase.hpp"
#include "ISendMessage.hpp"
class ChatroomControlAckMessage :
    public MessageBase, public ISendMessage
{
public:
    enum class MessageSendStatus
    {
        SEND_SUCCESS = 0,
        SEND_PARTIAL_SUCCESS = 1,
        SEND_FAILED = 2
	};
 
private:
    uint64_t _message_id;
    MessageSendStatus _send_status;
    std::string _room_id;
    std::vector<std::string> _unsend_uid_list;
    std::uint64_t _room_sequence;

public:
    //コンストラクタ
    ChatroomControlAckMessage();
    ChatroomControlAckMessage(
        const MessageBase::CommonField& common_fields,
        MessageSendStatus send_status,
        const std::string& room_id,
        uint64_t send_message_id,
        const std::vector<std::string>& unsend_uid_list,
        std::uint64_t room_sequence = 0
    );
    //デストラクタ
    ~ChatroomControlAckMessage();

    //各種アクセッサ
    uint64_t get_message_id() const { return _message_id; }
	MessageSendStatus get_send_status() const { return _send_status; }
    const std::string& get_room_id() const { return _room_id; }
    const std::vector<std::string>& get_unsend_uid_list() const { return _unsend_uid_list; }
    std::uint64_t get_room_sequence() const { return _room_sequence; }
    void set_room_sequence(std::uint64_t sequence) { _room_sequence = sequence; }

    //ISendMessageの実装
    json serialize_payload() const override;
    bool validate_payload() const override;
private:
    bool is_valid_room_id(const std::string& room_id) const;
};

#endif // !CHATROOMCONTROLACKMESSAGE_HPP

