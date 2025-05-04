#include "BaseMessage.h"

BaseMessage::BaseMessage()
	:_content_type{ ContentType::TEXT }, _message_type{""}, _created_at{ServerTime::get_current_time_to_milliseconds()}, _message_id{0}
{

}

BaseMessage::BaseMessage(ContentType c_type, const std::string& mt)
	:_content_type{c_type}, _message_type{mt}, _created_at{ServerTime::get_current_time_to_milliseconds()}, _message_id{0}
{
	if (mt.empty()) {
		throw std::invalid_argument("Message Type is empty.");
	}

	//if (mt.size() != 3 || mt.size() != 4) {
	//	throw std::invalid_argument("Message Type is not 3 byte.");
	//}
}

BaseMessage::BaseMessage(int c_type, const std::string& mt)
	:_content_type{ this->exchange_content_type_from_int(c_type)}, _message_type{mt}, _created_at{ServerTime::get_current_time_to_milliseconds()}, _message_id{0}
{
}


BaseMessage::BaseMessage(ContentType c_type, const std::string& m_type, const uint64_t id)
	:_content_type{ c_type }, _message_type{m_type}, _created_at{ ServerTime::get_current_time_to_milliseconds()}, _message_id{id}
{

}

BaseMessage::~BaseMessage()
{
}

BaseMessage::ContentType BaseMessage::get_content_type() const
{
	return this->_content_type;
}

void BaseMessage::set_content_type(ContentType type)
{
	this->_content_type = type;
}

void BaseMessage::set_content_type(int type_num)
{
	switch (type_num)
	{
	case 0:
		this->_content_type = ContentType::TEXT;
		break;
	case 1:
		this->_content_type = ContentType::IMAGE;
		break;
	case 2:
		this->_content_type = ContentType::VIDEO;
		break;
	case 3:
		this->_content_type = ContentType::NOTIFICATION;
		break;
	case 4:
		this->_content_type = ContentType::HEARTBEAT;
		break;
	default:
		break;
	}
}

std::string BaseMessage::get_created_at() const
{
	return this->_created_at;
}


uint64_t BaseMessage::get_message_id() const
{
	return this->_message_id;
}

void BaseMessage::set_message_id(const uint64_t m_id)
{
	this->_message_id = m_id;
}

bool BaseMessage::is_valid()
{
	if (is_valid_mesasge_type(this->_message_type) == true &&
		is_valid_message_id(this->_message_id) == true) {
		spdlog::info("message(receive time stamp : {:s}) is valid.", this->_created_at);
		return true;
	}
	else {
		LOG_ERROR("message(receive time stamp : {:s}) is not valid.", this->_created_at);
		return false;
	}
}



void BaseMessage::print_received_message_info(const json& recv_json, const std::string& ip_addr)
{
	std::string mt = recv_json["message_type"];
	spdlog::info("\033[33mMT{:s}\033[m receive, server received message from client(IP : {:s})", mt, ip_addr);
	spdlog::debug("Recevive JSON DATA (MT{:s}): {:s}", mt, recv_json.dump(4));
}


BaseMessage::ContentType BaseMessage::exchange_content_type_from_int(int value)
{
	switch (value)
	{
	case 0:
		return ContentType::TEXT;
		break;
	case 1:
		return ContentType::IMAGE;
		break;
	case 2:
		return ContentType::VIDEO;
		break;
	case 3:
		return ContentType::NOTIFICATION;
		break;
	case 4:
		return ContentType::HEARTBEAT;
		break;
	default:
		return ContentType::TEXT;
		break;
	}
}

bool BaseMessage::is_valid_mesasge_type(const std::string& mt)
{
	//Messagge_typeが3byteではない場合
	if (mt.size() != 3) {
		LOG_ERROR("Message Type is invalid. Message Type byte size : {:d}", mt.size());
		return false;
	}
	return true;
}


bool BaseMessage::is_valid_message_id(const uint64_t m_id)
{

	return true;
}
