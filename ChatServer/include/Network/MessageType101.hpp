#ifndef MESSAGETYPE101_HPP
#define MESSAGETYPE101_HPP
#include "BaseMessage.hpp"
class MessageType101 :
    public BaseMessage
{
private:
    std::string _client_id;
	bool _verify_result;

public:
	//コンストラクタ
	MessageType101() = delete;
	MessageType101(const std::string& client_id, bool verify_result);
		//デストラクタ
	~MessageType101();
	//各種アクセッサ(getter/setter)
	std::string get_client_id() const {
		return this->_client_id;
	}

	void set_client_id(const std::string& client_id) {
		this->_client_id = client_id;
	}

	static json deserialize_message(const std::unique_ptr<MessageType101>& mt101, const json& unsend_messages, const std::string& error_message);

};
#endif // !MESSAGETYPE101_HPP

