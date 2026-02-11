#ifndef HEART_BEAT_MESSAGE_HPP
#define HEART_BEAT_MESSAGE_HPP
#include "MessageBase.hpp"
#include "IReceiveMessage.hpp"


class HeartBeatMessage :
	public MessageBase, public IReceiveMessage
{
private:
	std::string _client_id;

public:
	//コンストラクタ
	explicit HeartBeatMessage();
	explicit HeartBeatMessage(const MessageBase::CommonField& common_fields, const std::string& client_id);
	//デストラクタ
	~HeartBeatMessage();
	//各種アクセッサ(getter/setter)
	std::string get_client_id() const { return this->_client_id; }

	//IReceiveMessageの実装
	std::unique_ptr<MessageBase> create_instance(const json& message) const override;
	bool validate_payload(const json& message) const override;
	bool validate_value() const override;
	
};
#endif // !HEART_BEAT_MESSAGE_HPP



