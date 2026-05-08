#ifndef SESSION_RESPONSE_MESSAGE_HPP
#define SESSION_RESPONSE_MESSAGE_HPP
#include "MessageBase.hpp"
#include "ISendMessage.hpp"
class SessionResponseMessage :
    public MessageBase, public ISendMessage
{
private:
    std::string _uid;
	bool _verify_result;
	std::string _error_message;
	json _unsend_messages = json::array();

public:
	//コンストラクタ
	SessionResponseMessage() = delete;
	SessionResponseMessage(const MessageBase::CommonField& common_fields, const std::string& uid, bool verify_result, const std::string& error_message, const json& unsend_messages);
		//デストラクタ
	~SessionResponseMessage();
	//各種アクセッサ(getter/setter)
	std::string get_uid_by_socket() const {
		return this->_uid;
	}

	void set_uid(const std::string& uid) {
		this->_uid = uid;
	}

	//ISendMessageの実装
	json serialize_payload() const override;
	bool validate_payload() const override;
};
#endif // !SESSION_RESPONSE_MESSAGE_HPP

