#ifndef CHATROOMCONTROLACKMESSAGE_HPP
#define CHATROOMCONTROLACKMESSAGE_HPP
#include "BaseMessage.hpp"
class ChatRoomControlAckMessage :
    public BaseMessage
{
public:
    enum class MessageSendStatus
    {
        SEND_SUCCESS = 0,
        SEND_PARTICAL_SUCCESS = 1,
        SEND_FAILED = 2
	};
 
private:
    MessageSendStatus _send_status;
    std::string _room_id;
    uint64_t _send_message_id;
    std::vector<std::string> _unsend_cid_list;

public:
    //コンストラクタ
    ChatRoomControlAckMessage() = delete;
    ChatRoomControlAckMessage(
        int content_type,
        const std::string& mt,
        MessageSendStatus send_status,
        const std::string& room_id,
        uint64_t send_message_id,
        const std::vector<std::string>& unsend_cid_list
    );
    //デストラクタ
    ~ChatRoomControlAckMessage();

    //各種アクセッサ
	MessageSendStatus get_send_status() const { return _send_status; }
    const std::string& get_room_id() const { return _room_id; }
    uint64_t get_send_message_id() const { return _send_message_id; }
    const std::vector<std::string>& get_unsend_cid_list() const { return _unsend_cid_list; }
    static json deserialize_message(const std::unique_ptr<ChatRoomControlAckMessage>& chatroom_control_ack_message);

};

#endif // !CHATROOMCONTROLACKMESSAGE_HPP

