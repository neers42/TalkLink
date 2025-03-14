#include "MessageType002.h"

MessageType002::MessageType002()
	:BaseMessage(), _trans_type{TransType::UNICAST}, _dst_chatroom_id{""}
{
}

MessageType002::MessageType002(BaseMessage::ContentType c_type, const std::string& mt, const std::string& dst_id, const std::string& src_id, const std::string& content, const uint64_t m_id, TransType tt, const std::string& dst_chatroom_id)
	:BaseMessage::BaseMessage(c_type, mt, m_id),_trans_type{tt}, _dst_chatroom_id{dst_chatroom_id}
{
}

MessageType002::MessageType002(int c_type, const std::string& mt, const std::string& dst_id, const std::string& src_id, const std::string& content, const uint64_t m_id, int trans_type, const std::string& dst_chatroom_id)
	:BaseMessage::BaseMessage(exchange_content_type_from_int(c_type), mt, m_id),
	_trans_type{exchange_trans_type_from_int(trans_type)},
	_receiver_id{dst_id},
	_sender_id{src_id},
	_content{content},
	_dst_chatroom_id{dst_chatroom_id}
{
}


MessageType002::~MessageType002()
{
}

MessageType002::TransType MessageType002::get_trans_type()
{
	return this->_trans_type;
}

int MessageType002::get_trans_type_int()
{
	switch (this->get_trans_type())
	{
	case TransType::UNICAST:
		return 0;
		break;
	case TransType::MULTICAST:
		return 1;
		break;
	case TransType::BLOADCAST:
		return 2;
		break;
	default:
		return -1;
		break;
	}

}

void MessageType002::set_trans_type(TransType tt)
{
	this->_trans_type = tt;
}

void MessageType002::set_trans_type(const int value)
{
	switch (value)
	{
	case 0:
		this->_trans_type = TransType::UNICAST;
		break;
	case 1:
		this->_trans_type = TransType::MULTICAST;
		break;
	case 2:
		this->_trans_type = TransType::BLOADCAST;
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

std::string MessageType002::get_dst_chatroom_id()
{
	return this->_dst_chatroom_id;
}

void MessageType002::set_dst_chatroom_id(const std::string& id)
{
	this->_dst_chatroom_id = id;
}

MessageType002::TransType MessageType002::exchange_trans_type_from_int(int value)
{
	switch (value)
	{
	case 0:
		return TransType::UNICAST;
		break;
	case 1:
		return TransType::MULTICAST;
		break;
	case 2:
		return TransType::BLOADCAST;
		break;
	default:
		break;
	}
}

bool MessageType002::is_valid()
{
	//クライアント間の通信
	if (this->get_trans_type() == MessageType002::TransType::UNICAST) {
		if (is_valid_receiver_id(this->_receiver_id) == true &&
			is_valid_mesasge_type(this->_message_type) == true &&
			is_valid_sender_id(this->_sender_id) == true &&
			is_valid_conent(this->_content) == true &&
			is_valid_message_id(this->_message_id) == true)
		{
			return true;
		}
	}
	//チャットルームの通信
	else {
		if (is_valid_dst_chatroom(this->_dst_chatroom_id) == true &&
			is_valid_mesasge_type(this->_message_type) == true &&
			is_valid_sender_id(this->_sender_id) == true &&
			is_valid_conent(this->_content) == true &&
			is_valid_message_id(this->_message_id) == true)
		{
			return true;
		}
	}
	
	}

bool MessageType002::is_valid_dst_chatroom(const std::string& cr_id)
{
	//dst_idが28byteではない場合
	if (cr_id.size() != 36) {
		spdlog::error("dst chatroom id is invalid. dst chatroom id byte size : {:d}", cr_id.size());
		return false;
	}
	return true;
}

bool MessageType002::is_valid_receiver_id(const std::string& dst_id)
{
	//dst_idが28byteではない場合
	if (dst_id.size() != 28) {
		spdlog::error("receiver id is invalid. dst client id byte size : {:d}", dst_id.size());
		return false;
	}
	return true;
}

bool MessageType002::is_valid_sender_id(const std::string& src_id)
{
	//dst_idが28byteではない場合
	if (src_id.size() != 28) {
		spdlog::error("sender id is invalid. src client id byte size : {:d}", src_id.size());
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

