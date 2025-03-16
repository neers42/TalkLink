#ifndef __MESSAGETYPE101_H__
#define __MESSAGETYPE101_H__
#include "BaseMessage.h"
class MessageType101 :
    public BaseMessage
{
private:
    std::string _client_id;

public:
	//コンストラクタ
	MessageType101();
	MessageType101(const std::string& client_id);
	MessageType101(ContentType c_type, const std::string& mt, const std::string& client_id);
	MessageType101(int c_type, const std::string& mt, const std::string& client_id);
	//デストラクタ
	~MessageType101();
	//各種アクセッサ(getter/setter)
	std::string get_client_id() const {
		return this->_client_id;
	}

	void set_client_id(const std::string& client_id) {
		this->_client_id = client_id;
	}

};
#endif // !__MESSAGETYPE101_H__

