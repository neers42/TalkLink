#ifndef SYNCREQUESTMESSAGE_HPP
#define SYNCREQUESTMESSAGE_HPP
#include "Utils/common.hpp"
#include "MessageBase.hpp"
#include "IReceiveMessage.hpp"

class SyncRequestMessage : public MessageBase, public IReceiveMessage
{
private:
    std::string _room_id;
    std::string _target_uid;   //同期対象(リクエスト元)のユーザID
    std::uint64_t _from_sequence;   //ユーザが最後に連続で受信したルームシーケンス番号

public:
    //コンストラクタ・デストラクタ
    SyncRequestMessage();
    SyncRequestMessage(
        const MessageBase::CommonField& common_fields,
        const std::string& room_id,
        const std::string& target_uid,
        const std::uint64_t from_sequence
    );
    ~SyncRequestMessage();

    //各種アクセッサ
    std::string get_room_id() const;
    std::string get_target_uid() const;
    std::uint64_t get_from_sequence() const;

    //IReceiveMessageの実装
	bool validate_payload(const json& message) const override;
	std::unique_ptr<MessageBase> create_instance(const json& message) const override;
	bool validate_value() const override;

private:
	bool is_valid_room_id(const std::string& room_id) const;
	bool is_valid_target_uid(const std::string& target_uid) const;

};


#endif  // !SYNCREQUESTMESSAGE_HPP