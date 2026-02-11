#ifndef CHATACKMESSAGE_HPP
#define CHATACKMESSAGE_HPP
#include "MessageBase.hpp"
#include "ISendMessage.hpp"
class ChatAckMessage :
    public MessageBase, public ISendMessage
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
	std::vector<std::string> _unsend_cid_list;



public:
    //コンストラクタ
    ChatAckMessage();
	ChatAckMessage(
		const MessageBase::CommonField& common_fields,
		const uint64_t _send_mid,
		const MessageSendStatus status,
		const std::string& room_id,
		const std::vector<std::string>& _unsend_cid_list);

    //デストラクタ
	~ChatAckMessage();

	//各種アクセッサ(getter/setter)
	int get_send_status() const;
	void set_send_status(const int status);
	std::string get_room_id() const;
	void set_room_id(const std::string& room_id);

	//ISendMessageの実装
	json serialize_payload() const override;
	bool validate_payload() const override;

};
#endif // !CHATACKMESSAGE_HPP

