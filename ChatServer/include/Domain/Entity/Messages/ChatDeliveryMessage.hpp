#ifndef CHATDELIVERYMESSAGE_HPP
#define CHATDELIVERYMESSAGE_HPP
#include "MessageBase.hpp"
#include "ISendMessage.hpp"
class ChatDeliveryMessage :
    public MessageBase, public ISendMessage
{
private:
	uint64_t _send_message_id; //送信メッセージID
	std::string _room_id;	//チャットルームID
	std::string _sender_uid;	//送信者ID
	std::string _content;	//メッセージ内容
	


public:
	ChatDeliveryMessage();
	ChatDeliveryMessage(
		const MessageBase::CommonField& common_fields,
		const uint64_t send_message_id,
		const std::string& room_id,
		const std::string& sender_id,
		const std::string& content);
	~ChatDeliveryMessage();

	//各種アクセッサ
	uint64_t get_send_message_id() const;
	std::string get_room_id() const;
	std::string get_sender_uid() const;
	std::string get_content() const;

	void set_room_id(const std::string& room_id);
	void set_sender_uid(const std::string& sender_id);
	void set_content(const std::string& content);

	//ISendMessageの実装
	json serialize_payload() const override;
	bool validate_payload() const override;
	
};
#endif // !CHATDELIVERYMESSAGE_HPP



