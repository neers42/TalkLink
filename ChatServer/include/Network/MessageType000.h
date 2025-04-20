#ifndef BASEMESSAGE_H_
#define BASEMESSAGE_H_


#include "BaseMessage.h"
class MessageType000 :
	public BaseMessage
{
private:
	std::string _client_id;

public:
	//コンストラクタ
	explicit MessageType000() = delete;
	explicit MessageType000(const std::string& client_id);
	//デストラクタ
	~MessageType000();
	//各種アクセッサ(getter/setter)
	std::string get_client_id() const { return this->_client_id; }

	static std::unique_ptr<MessageType000> serialize_message(const json& json_message);
	
};
#endif // !BASEMESSAGE_H_



