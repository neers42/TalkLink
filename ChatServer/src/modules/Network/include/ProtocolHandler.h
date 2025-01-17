#ifndef PROTOCOLHANDLER
#define PROTOCOLHANDLER
#include "common.h"
#include "wolfssl/ssl.h"
#include "MessageType001.h"
#include "MessageType002.h"
#include "MessageType003.h"
class ProtocolHandler
{
private:
	


public:

	ProtocolHandler();
	~ProtocolHandler();

	template <typename T>
	T serialize_message(const std::vector<uint8_t> recv_data);

	const std::string generate_message_id();


private:
	//各メッセージデータをjson形式にシリアライズする(オブジェクト⇒文字系列式)
	MessageType001 serialize_mt001(const json& recv_json);
	MessageType002 serialize_mt002(const json& recv_json);
	//MessageType003 serialize_mt003(const std::vector<uint8_t> recv_data);
	

};


#endif // !PROTOCOLHANDLER



