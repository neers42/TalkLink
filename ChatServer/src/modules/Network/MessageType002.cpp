#include "MessageType002.h"

MessageType002::MessageType002()
	:BaseMessage(), _trans_type{TransType::UNICAST}, _dst_chatroom_id{""}
{
}

MessageType002::MessageType002(BaseMessage::ContentType c_type, const std::string& mt, const std::string& dst_id, const std::string& src_id, const std::string& content, const std::string& m_id, TransType tt, const std::string& dst_chatroom_id)
	:BaseMessage::BaseMessage(c_type, mt, m_id),_trans_type{tt}, 
	_dst_chatroom_id{dst_chatroom_id}
{
}

MessageType002::MessageType002(int c_type, const std::string& mt, const std::string& dst_id, const std::string& src_id, const std::string& content, const std::string& m_id, int trans_type, const std::string& dst_chatroom_id)
	:BaseMessage::BaseMessage(exchange_content_type_from_int(c_type), mt, m_id),
	_trans_type{exchange_trans_type_from_int(trans_type)},
	_dst_client_id{dst_id},
	_src_client_id{src_id},
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

std::string MessageType002::get_dst_client_id() const
{
	return this->_dst_client_id;
}

void MessageType002::set_dst_client_id(const std::string& dst_id)
{
	this->_dst_client_id = dst_id;
}

std::string MessageType002::get_src_client_id() const
{
	return this->_src_client_id;
}

void MessageType002::set_src_client_id(const std::string& src_id)
{
	this->_src_client_id = src_id;
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
	if (is_valid_dst_id(this->_dst_client_id) == true && 
		is_valid_dst_chatroom(this->_dst_chatroom_id) == false &&
		is_valid_mesasge_type(this->_message_type) == true &&
		is_valid_src_id(this->_src_client_id) == true &&
		is_valid_conent(this->_content) == true &&
		is_valid_message_id(this->_message_id) == true)
	{
		return true;
	}
	//チャットルームの通信
	else if (is_valid_dst_id(this->_dst_client_id) == false &&
		is_valid_dst_chatroom(this->_dst_chatroom_id) == true &&
		is_valid_mesasge_type(this->_message_type) == true &&
		is_valid_src_id(this->_src_client_id) == true &&
		is_valid_conent(this->_content) == true &&
		is_valid_message_id(this->_message_id) == true) 
	{
		return true;
	}
	else {
		return false;
	}
}

bool MessageType002::is_valid_dst_chatroom(const std::string& id)
{
	//dst_idが28byteではない場合
	if (id.size() != 36) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("Dst chatroom id is invalid. Dst chatroom id byte size : %d", id.size()));
		return false;
	}
	return true;
}
