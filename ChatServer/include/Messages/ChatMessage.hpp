#ifndef CHATMESSAGE_HPP
#define CHATMESSAGE_HPP
#include "Chat/MessageIDGenerator.hpp"
#include "MessageBase.hpp"
#include "IReceiveMessage.hpp"
class ChatMessage :
    public MessageBase, public IReceiveMessage
{
private:
	uint64_t _message_id;	// メッセージID
	std::string _room_id;	// チャットルームID
	std::vector<std::string> _mention_ids;	// メンションIDのリスト
	std::string _sender_cid;	// 送信者のクライアントID
	std::string _content;	// メッセージ内容(JSON形式)

public:
	//コンストラクタ
    ChatMessage();
	ChatMessage(
		const MessageBase::CommonField& common_fields,
		const uint64_t m_id,
		const std::string& room_id, const std::vector<std::string>& mention_ids, const std::string& sender_cid, 
		const std::string& content);
	
	//デストラクタ
	~ChatMessage();
	
	//各種アクセッサ(getter/setter)
	uint64_t get_message_id() const;

	const std::vector<std::string>& get_mention_ids() const;
	void set_mention_ids(const std::vector<std::string>& mention_ids);

	std::string get_sender_cid() const;
	void set_sender_cid(const std::string& sender_id);


	std::string get_content() const;
	void set_content(const std::string& content);

	//_dst_chatroom_id
	std::string get_room_id() const;
	void set_room_id(const std::string& id);

	//IReceiveMessageの実装
	std::unique_ptr<MessageBase> create_instance(const json& message) const override;
	bool validate_payload(const json& message) const override;
	bool validate_value() const override;


private:
	bool is_valid_room_id(const std::string& room_id) const;
	bool is_valid_mention_ids(const std::vector<std::string>& mention_ids) const;
	bool is_valid_sender_id(const std::string& sender_id) const;
	bool is_valid_content(const std::string& content) const;

	
};
#endif // !CHATMESSAGE_HPP



