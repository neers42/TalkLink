#ifndef MESSAGETYPE103A_H
#define MESSAGETYPE103A_H
#include "BaseMessage.h"
class MessageType103A :
    public BaseMessage
{
public:
	struct ClientInfo {
		std::string cid;	//クライアントID
		std::string name;	//クライアント名
		std::string icon_url;
	};
private:
	std::string _client_id;	//クライアントID
	std::vector<ClientInfo> _cid_list;	//クライアントリスト
	std::string _error_message;	//エラーメッセージ

public:
	MessageType103A();
	~MessageType103A();
};
#endif // !MESSAGETYPE103A_H

