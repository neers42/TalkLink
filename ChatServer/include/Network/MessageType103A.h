#ifndef MESSAGETYPE103A
#define MESSAGETYPE103A
#include "BaseMessage.h"
class MessageType103A :
    public BaseMessage
{
private:
	std::string _room_id;	//チャットルームID
	std::string _sender_id;	//送信者ID
	std::string _content;	//メッセージ内容
	std::string _send_time;	//送信時間
	


public:
	MessageType103A() = delete;
	MessageType103A(int c_type,
		const std::string& mt,
		const uint64_t m_id,
		const std::string& room_id,
		const std::string& sender_id,
		const std::string& send_time,
		const std::string& content);
	~MessageType103A();

	static json desirialize_message(const std::unique_ptr<MessageType103A>& mt103A);
	
};
#endif // !MESSAGETYPE103A



