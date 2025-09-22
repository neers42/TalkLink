#ifndef CHATDELIVERYMESSAGE_HPP
#define CHATDELIVERYMESSAGE_HPP
#include "BaseMessage.hpp"
class ChatDeliveryMessage :
    public BaseMessage
{
private:
	std::string _room_id;	//チャットルームID
	std::string _sender_cid;	//送信者ID
	uint64_t _send_message_id;	//送信メッセージID
	std::string _content;	//メッセージ内容
	std::string _send_time;	//送信時間
	


public:
	ChatDeliveryMessage() = delete;
	ChatDeliveryMessage(int c_type,
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
	uint64_t get_send_message_id() const;
	std::string get_send_time() const;
	std::string get_content() const;

	void set_room_id(const std::string& room_id);
	void set_sender_cid(const std::string& sender_id);
	void set_send_message_id(const uint64_t send_message_id);
	void set_send_time(const std::string& send_time);
	void content(const std::string& content);


	static json deserialize_message(const std::unique_ptr<ChatDeliveryMessage>& chat_delivery_message);
	
};
#endif // !CHATDELIVERYMESSAGE_HPP



