#include "BaseMessage.h"

BaseMessage::BaseMessage()
	:_content_type{ ContentType::TEXT }, _message_type{""}, _dst_client_id{""}, _src_client_id{""}, _content{""}, _time_stamp{ServerTime::get_current_time()}, _message_id{""}
{

}

BaseMessage::BaseMessage(ContentType c_type, const std::string& m_type, const std::string& dst_id, std::string& src_id, const std::string& content, const std::string& id)
	:_content_type{ c_type }, _message_type{m_type},_dst_client_id {dst_id}, _src_client_id{ src_id }, _content{ content }, _time_stamp{ ServerTime::get_current_time()}, _message_id{id}
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
	default:
		break;
	}
}

std::string BaseMessage::get_dst_client_id() const
{
	return this->_dst_client_id;
}

void BaseMessage::set_dst_client_id(const std::string& dst_id)
{
	this->_dst_client_id = dst_id;
}

std::string BaseMessage::get_src_client_id() const
{
	return this->_src_client_id;
}

void BaseMessage::set_src_client_id(const std::string& src_id)
{
	this->_src_client_id = src_id;
}

std::string BaseMessage::get_content() const
{
	return this->_content;
}

void BaseMessage::set_content(const std::string& content)
{
	this->_content = content;
}

std::string BaseMessage::get_message_id() const
{
	return this->_message_id;
}

void BaseMessage::set_message_id(const std::string& m_id)
{
	this->_message_id = m_id;
}

bool BaseMessage::is_valid()
{
	if (is_valid_mesasge_type(this->_message_type) == true &&
		is_valid_dst_id(this->_dst_client_id) == true &&
		is_valid_src_id(this->_src_client_id) == true &&
		is_valid_conent(this->_content) == true &&
		is_valid_message_id(this->_message_id) == true) {

		LogHandler::get_instance().logging_debug(StringFormat("message(receive time stamp : %s) is valid.", this->_time_stamp));
		return true;
	}
	else {
		LogHandler::get_instance().logging_error(StringFormat("message(receive time stamp : %s) is not valid.", this->_time_stamp));
		return false;
	}
}

bool BaseMessage::is_valid_mesasge_type(const std::string& mt)
{
	//Messagge_type‚ª3byte‚Å‚Í‚È‚¢ê‡
	if (mt.size() != 3) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("Message Type is invalid. Message Type byte size : %d", mt.size()));
		return false;
	}
	return true;
}

bool BaseMessage::is_valid_dst_id(const std::string& dst_id)
{
	//dst_id‚ª28byte‚Å‚Í‚È‚¢ê‡
	if (dst_id.size() != 28) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("Dst_client_id is invalid. Dst_client_id byte size : %d", dst_id.size()));
		return false;
	}
	return true;
}

bool BaseMessage::is_valid_src_id(const std::string& src_id)
{
	if (src_id.size() != 28) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("Src_client_id is invalid. Src_client_id byte size : %d", src_id.size()));
		return false;
	}
	return true;
}

bool BaseMessage::is_valid_conent(const std::string& content)
{
	if (content.size() > 0) {
		return true;
	}
	LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("Content is invalid."));
	return false;
}

bool BaseMessage::is_valid_message_id(const std::string& m_id)
{

	return false;
}
