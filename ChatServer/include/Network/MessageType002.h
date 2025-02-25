#ifndef MESSAGETYPE002
#define MESSAGETYPE002
#include "BaseMessage.h"
class MessageType002 :
    public BaseMessage
{
public:
	enum class TransType
	{
		UNICAST,
		MULTICAST,
		BLOADCAST
	};
private:
	TransType _trans_type;
	std::string _dst_client_id;
	std::string _src_client_id;
	std::string _content;
	std::string _dst_chatroom_id;

public:
	//コンストラクタ
    MessageType002();
	MessageType002(BaseMessage::ContentType c_type, const std::string& mt, 
		const std::string& dst_id, const std::string& src_id, const std::string& content,
		const std::string& m_id,TransType tt, const std::string& dst_chatroom_id);
	MessageType002(int c_type, const std::string& mt,
		const std::string& dst_id, const std::string& src_id, const std::string& content,
		const std::string& m_id, int trans_type, const std::string& dst_chatroom_id);
	
	//デストラクタ
	~MessageType002();
	
	//各種アクセッサ(getter/setter)
	//_trans_type
	TransType get_trans_type();
	void set_trans_type(TransType tt);
	void set_trans_type(const int value);

	std::string get_dst_client_id() const;
	void set_dst_client_id(const std::string& dst_id);

	std::string get_src_client_id() const;
	void set_src_client_id(const std::string& src_id);

	std::string get_content() const;
	void set_content(const std::string& content);

	//_dst_chatroom_id
	std::string get_dst_chatroom_id();
	void set_dst_chatroom_id(const std::string& id);


	TransType exchange_trans_type_from_int(int value);

	//メッセージ検証
	bool is_valid() override;

private:
	bool is_valid_dst_chatroom(const std::string& id);
	bool is_valid_dst_id(const std::string& dst_id);
	bool is_valid_src_id(const std::string& src_id);
	bool is_valid_conent(const std::string& content);
	

	
};
#endif // !MESSAGETYPE002



