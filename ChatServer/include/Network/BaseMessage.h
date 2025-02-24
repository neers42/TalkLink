#ifndef BASEMESSAGE
#define BASEMESSAGE
#include "common.h"

class BaseMessage
{
public:
	enum class ContentType {
		TEXT,			//テキスト
		IMAGE,			//画像
		VIDEO,			//動画
		NOTIFICATION	//通知
	};

protected:
	ContentType _content_type;	//メッセージコンテンツの内容
	std::string _message_type;	//メッセージ番号
	std::string _dst_client_id;	//送信先のクライアントID
	std::string _src_client_id;	//送信元のクライアントID
	std::string _content;		//メッセージ本体
	std::string _time_stamp;	//タイムスタンプ
	std::string _message_id;	//メッセージID


public:

	//コンストラクタ
	BaseMessage();
	BaseMessage(ContentType c_type, const std::string& mt);
	BaseMessage(int c_type, const std::string& mt);
	BaseMessage(ContentType c_type, const std::string& m_type,
		const std::string& dst_id, const std::string& src_id,
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
	void print_message(const json& recv_json, const std::string& ip_addr);

	//Content Typeをint型から変換
	ContentType exchange_content_type_from_int(int value);

protected:
	//メッセージ内容検証
	bool is_valid_mesasge_type(const std::string& mt);
	bool is_valid_dst_id(const std::string& dst_id);
	bool is_valid_src_id(const std::string& src_id);
	bool is_valid_conent(const std::string& content);
	bool is_valid_message_id(const std::string& m_id);
	
};


#endif // !BASEMESSAGE


