#ifndef BASEMESSAGE_H_
#define BASEMESSAGE_H_


#include "BaseMessage.h"
class MessageType000 :
	public BaseMessage
{
private:
	std::string _client_id;

public:
	explicit MessageType000();
	explicit MessageType000(int content_type, std::string& mt, const std::string& client_id);
	~MessageType000();
};
#endif // !BASEMESSAGE_H_



