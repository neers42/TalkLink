#pragma once
#include "BaseMessage.h"
class MessageType103B :
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
    MessageType103B() = delete;
    MessageType103B(
        int content_type,
        const std::string& mt,
        MessageSendStatus send_status,
        const std::string& room_id,
        uint64_t send_message_id,
        const std::vector<std::string>& unsend_cid_list
    );
    //デストラクタ
    ~MessageType103B();

    //各種アクセッサ
	MessageSendStatus get_send_status() const { return _send_status; }
    const std::string& get_room_id() const { return _room_id; }
    uint64_t get_send_message_id() const { return _send_message_id; }
    const std::vector<std::string>& get_unsend_cid_list() const { return _unsend_cid_list; }
    static json deserialize(const std::unique_ptr<MessageType103B>& mt103B);

};

