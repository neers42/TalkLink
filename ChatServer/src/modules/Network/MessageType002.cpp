#include "MessageType002.h"

MessageType002::MessageType002()
	:BaseMessage(), _chat_type{ChatType::Group}, _room_id{""}
{
}

MessageType002::MessageType002(BaseMessage::ContentType c_type, const std::string& mt,
	const std::string& receiver_id, const std::string& sender_id, const std::string& content,
	const uint64_t m_id, ChatType tt, const std::string& room_id)
	//BaseMessage::ContentType c_type, const std::string& mt, const std::string& dst_id, const std::string& src_id, const std::string& content, const uint64_t m_id, ChatType tt, const std::string& dst_chatroom_id)
	:BaseMessage::BaseMessage(c_type, mt, m_id),_chat_type{tt}, _room_id{room_id}
{
}

MessageType002::MessageType002(int c_type, const std::string& mt,
	const std::string& receiver_id, const std::string& sender_id, const std::string& content,
	const uint64_t m_id, int chat_type, const std::string& room_id)
	//int c_type, const std::string& mt, const std::string& dst_id, const std::string& src_id, const std::string& content, const uint64_t m_id, int trans_type, const std::string& dst_chatroom_id)
	:BaseMessage::BaseMessage(exchange_content_type_from_int(c_type), mt, m_id),
	_chat_type{exchange_trans_type_from_int(chat_type)},
	_receiver_id{receiver_id},
	_sender_id{sender_id},
	_content{content},
	_room_id{room_id}
{
}


MessageType002::~MessageType002()
{
}

MessageType002::ChatType MessageType002::get_chat_type()
{
	return this->_chat_type;
}

int MessageType002::convert_chat_type_to_int()
{
	switch (this->get_chat_type())
	{
	case ChatType::Group:
		return 0;
		break;
	case ChatType::P2P:
		return 1;
		break;
	default:
		return -1;
		break;
	}

}

void MessageType002::set_chat_type(ChatType tt)
{
	this->_chat_type = tt;
}

void MessageType002::set_chat_type_from_int(const int value)
{
	switch (value)
	{
	case 0:
		this->_chat_type = ChatType::Group;
		break;
	case 1:
		this->_chat_type = ChatType::P2P;
		break;
	default:
		break;
	}
}

std::string MessageType002::get_receiver_id() const
{
	return this->_receiver_id;
}

void MessageType002::set_receiver_id(const std::string& dst_id)
{
	this->_receiver_id = dst_id;
}

std::string MessageType002::get_sender_id() const
{
	return this->_sender_id;
}

void MessageType002::set_sender_id(const std::string& src_id)
{
	this->_sender_id = src_id;
}

std::string MessageType002::get_content() const
{
	return this->_content;
}

void MessageType002::set_content(const std::string& content)
{
	this->_content = content;
}

std::string MessageType002::get_room_id()
{
	return this->_room_id;
}

void MessageType002::set_room_id(const std::string& id)
{
	this->_room_id = id;
}

MessageType002::ChatType MessageType002::exchange_trans_type_from_int(int value)
{
	switch (value)
	{
	case 0:
		return ChatType::Group;
		break;
	case 1:
		return ChatType::P2P;
		break;
	default:
		break;
	}
}

bool MessageType002::is_valid()
{
	//グループチャットの場合
	if (this->get_chat_type() == MessageType002::ChatType::Group) {
		if (is_valid_mesasge_type(this->_message_type) == true &&
			is_valid_message_id(this->_message_id) == true &&
			is_valid_room_id(this->_room_id) == true &&
			is_valid_sender_id(this->_sender_id) == true &&
			is_valid_conent(this->_content) == true)
		{
			return true;
		}
	//P2Pの場合
	}else if (this->get_chat_type() == MessageType002::ChatType::P2P) {
		if (is_valid_mesasge_type(this->_message_type) == true &&
			is_valid_message_id(this->_message_id) == true &&
			is_valid_room_id(this->_room_id) == true &&
			is_valid_receiver_id(this->_receiver_id) == true &&
			is_valid_sender_id(this->_sender_id) == true &&
			is_valid_conent(this->_content) == true)
		{
			return true;
		}
	}
	return false;	
	
}

bool MessageType002::is_valid_room_id(const std::string& room_id)
{
	//dst_idが28byteではない場合
	if (room_id.size() != 36) {
		spdlog::error("dst chatroom id is invalid. dst chatroom id byte size : {:d}", room_id.size());
		return false;
	}
	return true;
}

bool MessageType002::is_valid_receiver_id(const std::string& receiver)
{
	//dst_idが28byteではない場合
	if (receiver.size() != 28) {
		spdlog::error("receiver id is invalid. dst client id byte size : {:d}", receiver.size());
		return false;
	}
	return true;
}

bool MessageType002::is_valid_sender_id(const std::string& sender_id)
{
	//dst_idが28byteではない場合
	if (sender_id.size() != 28) {
		spdlog::error("sender id is invalid. src client id byte size : {:d}", sender_id.size());
		return false;
	}
	return true;
}

bool MessageType002::is_valid_conent(const std::string& content)
{
	//dst_idが28byteではない場合
	if (content.size() <= 0) {
		spdlog::error("content is invalid. content byte size : {:d}", content.size());
		return false;
	}
	return true;
}

