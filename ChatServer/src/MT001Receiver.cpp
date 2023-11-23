#include "MT001Receiver.h"

MT001Receiver::MT001Receiver()
{

}

MT001Receiver::~MT001Receiver()
{

}

MT001Receiver::MT001Data::MT001Data()
{
}

MT001Receiver::MT001Data::~MT001Data()
{
}

void MT001Receiver::MT001Data::set_sign_type(const uint8_t& sign_type)
{
	switch (sign_type)
	{
	case 0:
		this->m_sign_type = SIGN_TYPE::sign_up;
		break;

	case 1:
		this->m_sign_type = SIGN_TYPE::sign_in;
		break;

	case 2:
		this->m_sign_type = SIGN_TYPE::sign_out;
		break;
	default:
		break;
	}
}

void MT001Receiver::MT001Data::set_client_name_byte_size(const uint8_t& name_byte_size)
{
	this->m_client_gmail_byte_size = name_byte_size;
}

void MT001Receiver::MT001Data::set_client_name(const std::string& name)
{
	this->m_client_name = name;
}

void MT001Receiver::MT001Data::set_client_gmail_byte_size(const uint8_t& gmail_byte_size)
{
	this->m_client_gmail_byte_size = gmail_byte_size;
}

void MT001Receiver::MT001Data::set_client_gmail(const std::string& gmail)
{
	this->m_client_gmail = gmail;
}

void MT001Receiver::MT001Data::set_client_password_byte_size(const uint8_t& password_byte_size)
{
	this->m_client_password_byte_size = password_byte_size;
}

void MT001Receiver::MT001Data::set_client_password(const std::string& password)
{
	this->m_client_password = password;
}
