#include "MessageType103B.h"

MessageType103B::MessageType103B()
	:BaseMessage(), _status{ 0 }, _recever_id{ "" }, _room_id{ "" }
{
}

MessageType103B::MessageType103B(int c_type,
	const std::string& mt,
	const uint64_t id,
	const int status,
	const std::string& recever_id,
	const std::string& room_id)
	:BaseMessage(exchange_content_type_from_int(c_type), mt, id), _status(status), _recever_id(recever_id), _room_id(room_id)
{
}

MessageType103B::~MessageType103B()
{
}

int MessageType103B::get_status() const
{
	return 0;
}

void MessageType103B::set_status(const int status)
{
}

std::string& MessageType103B::get_recever_id() const
{
	// TODO: return ステートメントをここに挿入します
	std::string a = "";
	return a;
}

void MessageType103B::set_recever_id(const std::string& recever_id)
{

}

std::string& MessageType103B::get_room_id() const
{
	// TODO: return ステートメントをここに挿入します
	std::string a = "";
	return a;
}

void MessageType103B::set_room_id(const std::string& room_id)
{
}
