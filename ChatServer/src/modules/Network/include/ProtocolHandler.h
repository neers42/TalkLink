#ifndef PROTOCOLHANDLER
#define PROTOCOLHANDLER
#include "common.h"
#include "wolfssl/ssl.h"
class ProtocolHandler
{
private:
	


public:

	ProtocolHandler();
	~ProtocolHandler();


	//各メッセージデータをjson形式にシリアライズする(オブジェクト⇒文字系列式)
	json serialize_mt001(const std::vector<uint8_t> recv_data);
	json serialize_mt002(const std::vector<uint8_t> recv_data);
	json serialize_mt003(const std::vector<uint8_t> recv_data);
	

};


#endif // !PROTOCOLHANDLER



