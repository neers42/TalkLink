#ifndef CHATMESSAGE_HPP
#define CHATMESSAGE_HPP
#include "Chat/MessageIDGenerator.hpp"
#include "MessageBase.hpp"
#include "IReceiveMessage.hpp"
class ChatMessage :
    public MessageBase, public IReceiveMessage
{
private:
	std::string _room_id;	// チャットルームID
	std::vector<std::string> _mention_ids;	// メンションIDのリスト
	std::string _sender_id;	// 送信者のクライアントID
	std::string _content;	// メッセージ内容(JSON形式)

public:
	//コンストラクタ
    ChatMessage();
	ChatMessage(MessageBase::ContentType c_type, const std::string& mt, const uint64_t m_id,
		const std::string& room_id, const std::vector<std::string>& mention_ids, const std::string& sender_id, 
		const std::string& content);
	ChatMessage(int c_type, 
		const std::string& mt, 
		const uint64_t m_id,
		const std::string& room_id,
		const std::vector<std::string>& mention_ids, 
		const std::string& sender_id,
		const std::string& content);

	ChatMessage(int c_type, 
		const std::string& mt, 
		const uint64_t m_id,
		const std::string& time_stamp,
		const std::string& room_id,
		const std::vector<std::string>& mention_ids, 
		const std::string& sender_id,
		const std::string& content);
	
	//デストラクタ
	~ChatMessage();
	
	//各種アクセッサ(getter/setter)
	const std::vector<std::string>& get_mention_ids() const;
	void set_mention_ids(const std::vector<std::string>& mention_ids);

	std::string get_sender_cid() const;
	void set_sender_id(const std::string& src_id);


	std::string get_content() const;
	void set_content(const std::string& content);

	//_dst_chatroom_id
	std::string get_room_id();
	void set_room_id(const std::string& id);

	//IReceiveMessageの実装
	std::unique_ptr<MessageBase> create_instance(const json& message) const override;
	bool validate_specific_fields(const json& message) const override;
	bool validate_value() const override;


private:
	bool is_valid_room_id(const std::string& room_id) const;
	bool is_valid_mention_ids(const std::vector<std::string>& mention_ids) const;
	bool is_valid_sender_id(const std::string& sender_id) const;
	bool is_valid_content(const std::string& content) const;

	
};
#endif // !CHATMESSAGE_HPP



