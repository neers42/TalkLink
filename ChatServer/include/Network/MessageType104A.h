#ifndef MESSAGETYPE104A
#define MESSAGETYPE104A
#include "BaseMessage.h"
class MessageType104A :
    public BaseMessage
{
private:
	std::string _room_id;	//チャットルームID
	std::string _sender_id;	//送信者ID
	uint64_t _send_message_id;	//送信メッセージID
	std::string _content;	//メッセージ内容
	std::string _send_time;	//送信時間
	


public:
	MessageType104A() = delete;
	MessageType104A(int c_type,
		const std::string& mt,
		const std::string& room_id,
		const std::string& sender_id,
		const uint64_t send_message_id,
		const std::string& send_time,
		const std::string& content);
	~MessageType104A();

	//各種アクセッサ
	std::string get_room_id() const;
	std::string get_sender_id() const;
	uint64_t get_send_message_id() const;
	std::string get_send_time() const;
	std::string get_content() const;

	void set_room_id(const std::string& room_id);
	void set_sender_id(const std::string& sender_id);
	void set_send_message_id(const uint64_t send_message_id);
	void set_send_time(const std::string& send_time);
	void content(const std::string& content);


	static json deserialize_message(const std::unique_ptr<MessageType104A>& mt104A);
	
};
#endif // !MESSAGETYPE104A



