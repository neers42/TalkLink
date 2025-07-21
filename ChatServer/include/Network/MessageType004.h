#ifndef MESSAGETYPE004_H_
#define MESSAGETYPE004_H_
#include "BaseMessage.h"
#include "Chat/MessageIDGenerator.h"
class MessageType004 :
    public BaseMessage
{
private:
	std::string _room_id;	// チャットルームID
	std::vector<std::string> _mention_ids;	// メンションIDのリスト
	std::string _sender_id;	// 送信者のクライアントID
	std::string _send_time;	// 送信時間
	std::string _content;	// メッセージ内容

public:
	//コンストラクタ
    MessageType004() = delete;
	MessageType004(BaseMessage::ContentType c_type, const std::string& mt, const uint64_t m_id,
		const std::string& room_id, const std::vector<std::string>& mention_ids, const std::string& sender_id, 
		const std::string& send_time, const std::string& content);
	MessageType004(int c_type, const std::string& mt, const uint64_t m_id,
		const std::string& room_id,const std::vector<std::string>& mention_ids, const std::string& sender_id,
		const std::string& send_time ,const std::string& content);
	
	//デストラクタ
	~MessageType004();
	
	//各種アクセッサ(getter/setter)
	const std::vector<std::string>& get_mention_ids() const;
	void set_mention_ids(const std::vector<std::string>& mention_ids);

	std::string get_sender_id() const;
	void set_sender_id(const std::string& src_id);

	std::string get_send_time() const;

	std::string get_content() const;
	void set_content(const std::string& content);

	//_dst_chatroom_id
	std::string get_room_id();
	void set_room_id(const std::string& id);

	//メッセージ検証
	bool is_valid() override;
	static std::unique_ptr<MessageType004> serialize_message(const json& json_message);


private:
	bool is_valid_room_id(const std::string& room_id);
	bool is_valid_mention_ids(const std::vector<std::string>& mention_ids);
	bool is_valid_sender_id(const std::string& sender_id);
	bool is_valid_conent(const std::string& content);
	

	
};
#endif // !MESSAGETYPE004_H_



