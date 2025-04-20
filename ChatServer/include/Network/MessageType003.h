#ifndef MESSAGETYPE003_H_
#define MESSAGETYPE003_H_
#include "BaseMessage.h"
#include "MessageIDGenerator.h"
class MessageType003 :
    public BaseMessage
{
public:
	enum class ChatType
	{
		Group,
		P2P
	};
private:
	ChatType _chat_type;
	std::string _room_id;
	std::string _receiver_id;
	std::string _sender_id;
	std::string _send_time;
	std::string _content;

public:
	//コンストラクタ
    MessageType003() = delete;
	MessageType003(BaseMessage::ContentType c_type, const std::string& mt, const uint64_t m_id, ChatType tt,
		const std::string& room_id, const std::string& receiver_id, const std::string& sender_id, 
		const std::string& send_time, const std::string& content);
	MessageType003(int c_type, const std::string& mt, const uint64_t m_id, int chat_type,
		const std::string& room_id,const std::string& receiver_id, const std::string& sender_id,
		const std::string& send_time ,const std::string& content);
	
	//デストラクタ
	~MessageType003();
	
	//各種アクセッサ(getter/setter)
	//_trans_type
	ChatType get_chat_type();
	int convert_chat_type_to_int();
	void set_chat_type(ChatType tt);
	void set_chat_type_from_int(const int value);

	std::string get_receiver_id() const;
	void set_receiver_id(const std::string& dst_id);

	std::string get_sender_id() const;
	void set_sender_id(const std::string& src_id);

	std::string get_send_time() const;

	std::string get_content() const;
	void set_content(const std::string& content);

	//_dst_chatroom_id
	std::string get_room_id();
	void set_room_id(const std::string& id);


	ChatType exchange_trans_type_from_int(int value);

	//メッセージ検証
	bool is_valid() override;
	static std::unique_ptr<MessageType003> serialize_message(const json& json_message);


private:
	bool is_valid_room_id(const std::string& room_id);
	bool is_valid_receiver_id(const std::string& receiver_id);
	bool is_valid_sender_id(const std::string& sender_id);
	bool is_valid_conent(const std::string& content);
	

	
};
#endif // !MESSAGETYPE003_H_



