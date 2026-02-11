#ifndef CHATROOMCONTROLACKMESSAGE_HPP
#define CHATROOMCONTROLACKMESSAGE_HPP
#include "MessageBase.hpp"
#include "ISendMessage.hpp"
class ChatRoomControlAckMessage :
    public MessageBase, public ISendMessage
{
public:
    enum class MessageSendStatus
    {
        SEND_SUCCESS = 0,
        SEND_PARTICAL_SUCCESS = 1,
        SEND_FAILED = 2
	};
 
private:
    uint64_t _message_id;
    MessageSendStatus _send_status;
    std::string _room_id;
    std::vector<std::string> _unsend_cid_list;

public:
    //コンストラクタ
    ChatRoomControlAckMessage();
    ChatRoomControlAckMessage(
        const MessageBase::CommonField& common_fields,
        MessageSendStatus send_status,
        const std::string& room_id,
        uint64_t send_message_id,
        const std::vector<std::string>& unsend_cid_list
    );
    //デストラクタ
    ~ChatRoomControlAckMessage();

    //各種アクセッサ
    uint64_t get_message_id() const { return _message_id; }
	MessageSendStatus get_send_status() const { return _send_status; }
    const std::string& get_room_id() const { return _room_id; }
    const std::vector<std::string>& get_unsend_cid_list() const { return _unsend_cid_list; }

    //ISendMessageの実装
    json serialize_payload() const override;
    bool validate_payload() const override;
private:
    bool is_valid_room_id(const std::string& room_id) const;
};

#endif // !CHATROOMCONTROLACKMESSAGE_HPP

