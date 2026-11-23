#ifndef SYNCRESPONSEMESSAGE_HPP
#define SYNCRESPONSEMESSAGE_HPP
#include "Utils/common.hpp"
#include "MessageBase.hpp"
#include "ISendMessage.hpp"

class SyncResponseMessage : public MessageBase, public ISendMessage
{
private:
    std::string _target_room_id;
    std::string _target_uid;
    json _gap_messages;

public:
    //コンストラクタ
    SyncResponseMessage() = delete;
    SyncResponseMessage(
        const MessageBase::CommonField& common_fields,
        const std::string& target_room_id,
        const std::string& target_uid,
        const json& gap_messages
    );
    //デストラクタ
    ~SyncResponseMessage();
    //各種アクセッサ
    std::string get_target_room_id() const;
    std::string get_target_uid() const;
    json get_gap_messages() const;

    //ISendMessageの実装
    json serialize_payload() const override;
    bool validate_payload() const override;

};

#endif  // !SYNCRESPONSEMESSAGE_HPP