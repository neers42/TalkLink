#ifndef MESSAGETYPE104_H_
#define MESSAGETYPE104_H_
#include "BaseMessage.h"
class MessageType104B :
    public BaseMessage
{
public:
	enum class MessageSendStatus
	{
		SEND_SUCCESS = 0,
		SEND_PARTIAL_SUCCESS = 1,
		SEND_FAILED = 2
	};
private:
	MessageSendStatus _send_status;
	std::string _room_id;
	uint64_t _send_message_id;
	std::vector<std::string> _unsend_cid_list;



public:
    //コンストラクタ
    MessageType104B() = delete;
	MessageType104B(int c_type,
		const std::string& mt,
		const MessageSendStatus status,
		const std::string& room_id,
		const uint64_t _send_mid,
		const std::vector<std::string>& _unsend_cid_list);

    //デストラクタ
	~MessageType104B();

	//各種アクセッサ(getter/setter)
	int get_status() const;
	void set_status(const int status);
	std::string get_room_id() const;
	void set_room_id(const std::string& room_id);
	uint64_t get_send_message_id() const;
	void set_send_message_id(const uint64_t send_mid);
	static json deserialize_message(const std::unique_ptr<MessageType104B>& mt104B);

};
#endif // !MESSAGETYPE104_H_

