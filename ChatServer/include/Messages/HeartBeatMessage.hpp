#ifndef HEART_BEAT_MESSAGE_HPP
#define HEART_BEAT_MESSAGE_HPP
#include "BaseMessage.hpp"


class HeartBeatMessage :
	public BaseMessage
{
private:
	std::string _client_id;

public:
	//コンストラクタ
	explicit HeartBeatMessage() = delete;
	explicit HeartBeatMessage(const std::string& client_id);
	//デストラクタ
	~HeartBeatMessage();
	//各種アクセッサ(getter/setter)
	std::string get_client_id() const { return this->_client_id; }

	static std::unique_ptr<HeartBeatMessage> serialize_message(const json& json_message);
	
};
#endif // !MESSAGE_TYPE000_HPP



