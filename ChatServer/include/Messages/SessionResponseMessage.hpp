#ifndef CLIENTSESSIONRESPONSEMESSAGE_HPP
#define CLIENTSESSIONRESPONSEMESSAGE_HPP
#include "MessageBase.hpp"
#include "ISendMessage.hpp"
class SessionResponseMessage :
    public MessageBase, public ISendMessage
{
private:
    std::string _client_id;
	bool _verify_result;
	std::string _error_message;
	json _unsend_messages = json::array();

public:
	//コンストラクタ
	SessionResponseMessage() = delete;
	SessionResponseMessage(const MessageBase::CommonField& common_fields, const std::string& client_id, bool verify_result, const std::string& error_message, const json& unsend_messages);
		//デストラクタ
	~SessionResponseMessage();
	//各種アクセッサ(getter/setter)
	std::string get_client_id() const {
		return this->_client_id;
	}

	void set_client_id(const std::string& client_id) {
		this->_client_id = client_id;
	}

	//ISendMessageの実装
	json serialize_payload() const override;
	bool validate_payload() const override;
};
#endif // !CLIENTSESSIONRESPONSEMESSAGE_HPP

