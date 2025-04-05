#ifndef MESSAGETYPE103_H_
#define MESSAGETYPE103_H_
#include "BaseMessage.h"
class MessageType103B :
    public BaseMessage
{
private:
	int _status;
	std::string _recever_id;
	std::string _room_id;


public:
    //コンストラクタ
    MessageType103B();
	MessageType103B(int c_type,
		const std::string& mt,
		const uint64_t id,
		const int status,
		const std::string& recever_id,
		const std::string& room_id);

    //デストラクタ
	~MessageType103B();

	//各種アクセッサ(getter/setter)
	int get_status() const;
	void set_status(const int status);
	std::string& get_recever_id() const;
	void set_recever_id(const std::string& recever_id);
	std::string& get_room_id() const;
	void set_room_id(const std::string& room_id);

};
#endif // !MESSAGETYPE103_H_

