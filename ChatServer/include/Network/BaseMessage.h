#ifndef BASEMESSAGE
#define BASEMESSAGE
#include "Utils/common.h"
#include "spdlog/spdlog.h"
#include "LogHandler.h"


class BaseMessage
{
public:
	enum class ContentType {
		TEXT,			//テキスト
		IMAGE,			//画像
		VIDEO,			//動画
		NOTIFICATION,	//通知
		HEARTBEAT,		//ハートビート
	};

protected:
	ContentType _content_type;	//メッセージコンテンツの内容
	std::string _message_type;	//メッセージタイプ
	std::string _created_at;	//タイムスタンプ
	uint64_t _message_id;	//メッセージID


public:

	//コンストラクタ
	BaseMessage();
	BaseMessage(ContentType c_type, const std::string& mt);
	BaseMessage(int c_type, const std::string& mt);
	BaseMessage(ContentType c_type, const std::string& m_type,const uint64_t id);

	//デストラクタ
	virtual ~BaseMessage();

	//各種アクセッサ(getter/setter)
	ContentType get_content_type() const;
	void set_content_type(ContentType type);
	void set_content_type(int type_num);
	std::string get_message_type() const;
	std::string get_created_at() const;

	uint64_t get_message_id() const;
	void set_message_id(const std::uint64_t m_id);

	//メッセージ検証
	virtual bool is_valid();

	//メッセージ表示
	static void print_received_message_info(const json& recv_json, const std::string& ip_addr);



	//Content Typeをint型から変換
	ContentType exchange_content_type_from_int(int value);

protected:
	//メッセージ内容検証
	bool is_valid_mesasge_type(const std::string& mt);
	bool is_valid_message_id(const uint64_t m_id);
	
};

#endif // !BASEMESSAGE


