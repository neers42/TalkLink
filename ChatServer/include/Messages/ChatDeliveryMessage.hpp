#ifndef CHATDELIVERYMESSAGE_HPP
#define CHATDELIVERYMESSAGE_HPP
#include "MessageBase.hpp"
#include "ISendMessage.hpp"
class ChatDeliveryMessage :
    public MessageBase, public ISendMessage
{
private:
	std::string _room_id;	//チャットルームID
	std::string _sender_cid;	//送信者ID
	std::string _content;	//メッセージ内容
	


public:
	ChatDeliveryMessage();
	ChatDeliveryMessage(int c_type,
		const std::string& mt,
		const std::string& room_id,
		const std::string& sender_id,
		const uint64_t send_message_id,
		const std::string& send_time,
		const std::string& content);
	ChatDeliveryMessage(MessageBase::ContentType c_type,
		const std::string& mt,
		const std::string& room_id,
		const std::string& sender_id,
		const uint64_t send_message_id,
		const std::string& send_time,
		const std::string& content);
	~ChatDeliveryMessage();

	//各種アクセッサ
	std::string get_room_id() const;
	std::string get_sender_cid() const;
	std::string get_content() const;

	void set_room_id(const std::string& room_id);
	void set_sender_cid(const std::string& sender_id);
	void set_content(const std::string& content);

	//ISendMessageの実装
	json serialize_specific_message_fields() const override;
	bool validate_specific_message_fields() const override;
	
};
#endif // !CHATDELIVERYMESSAGE_HPP



