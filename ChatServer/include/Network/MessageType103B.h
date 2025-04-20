#ifndef MESSAGETYPE103_H_
#define MESSAGETYPE103_H_
#include "BaseMessage.h"
class MessageType103B :
    public BaseMessage
{
private:
	bool _send_status;
	std::string _room_id;


public:
    //コンストラクタ
    MessageType103B() = delete;
	MessageType103B(int c_type,
		const std::string& mt,
		const uint64_t message_id,
		const bool status,
		const std::string& room_id);

    //デストラクタ
	~MessageType103B();

	//各種アクセッサ(getter/setter)
	int get_status() const;
	void set_status(const bool status);
	std::string get_room_id() const;
	void set_room_id(const std::string& room_id);
	static json desirialize_message(const std::unique_ptr<MessageType103B>& mt103B);

};
#endif // !MESSAGETYPE103_H_

