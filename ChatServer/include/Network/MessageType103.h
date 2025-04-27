#ifndef MESSAGETYPE103_H
#define MESSAGETYPE103_H
#include "BaseMessage.h"
class MessageType103 :
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
	std::vector<ClientInfo> _client_list;	//クライアントリスト
	std::string _error_message;	//エラーメッセージ

public:
	MessageType103();
	~MessageType103();
};
#endif // !MESSAGETYPE103_H

