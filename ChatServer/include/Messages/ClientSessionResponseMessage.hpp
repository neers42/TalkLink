#ifndef CLIENTSESSIONRESPONSEMESSAGE_HPP
#define CLIENTSESSIONRESPONSEMESSAGE_HPP
#include "BaseMessage.hpp"
class ClientSessionResponseMessage :
    public BaseMessage
{
private:
    std::string _client_id;
	bool _verify_result;

public:
	//コンストラクタ
	ClientSessionResponseMessage() = delete;
	ClientSessionResponseMessage(const std::string& client_id, bool verify_result);
		//デストラクタ
	~ClientSessionResponseMessage();
	//各種アクセッサ(getter/setter)
	std::string get_client_id() const {
		return this->_client_id;
	}

	void set_client_id(const std::string& client_id) {
		this->_client_id = client_id;
	}

	static json deserialize_message(const std::unique_ptr<ClientSessionResponseMessage>& session_response, const json& unsend_messages, const std::string& error_message);

};
#endif // !CLIENTSESSIONRESPONSEMESSAGE_HPP

