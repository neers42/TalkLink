#ifndef BASEMESSAGE
#define BASEMESSAGE
#include "common.h"

class BaseMessage
{
public:
	enum class ContentType {
		TEXT,
		IMAGE,
		VIDEO,
		NOTIFICATION
	};

private:
	ContentType _content_type;
	std::string _message_type;
	std::string _dst_client_id;
	std::string _src_client_id;
	std::string _content;
	std::string _time_stamp;
	std::string _message_id;


public:

	//コンストラクタ
	BaseMessage();
	BaseMessage(ContentType c_type, const std::string& m_type,
		const std::string& dst_id, std::string& src_id,
		const std::string& content, const std::string& id);

	//デストラクタ
	virtual ~BaseMessage();

	//各種アクセッサ(getter/setter)
	ContentType get_content_type() const;
	void set_content_type(ContentType type);
	void set_content_type(int type_num);

	std::string get_dst_client_id() const;
	void set_dst_client_id(const std::string& dst_id);

	std::string get_src_client_id() const;
	void set_src_client_id(const std::string& src_id);

	std::string get_content() const;
	void set_content(const std::string& content);

	std::string get_message_id() const;
	void set_message_id(const std::string& m_id);

	//メッセージ検証
	virtual bool is_valid();

	//メッセージ表示
	template<typename MT>
	void print_message(MT message) {
		
	}

protected:
	bool is_valid_mesasge_type(const std::string& mt);
	bool is_valid_dst_id(const std::string& dst_id);
	bool is_valid_src_id(const std::string& src_id);
	bool is_valid_conent(const std::string& content);
	bool is_valid_message_id(const std::string& m_id);
	
};


#endif // !BASEMESSAGE


