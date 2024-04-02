#include "MT001Receiver.h"

MT001Receiver::MT001Receiver()
{

}

MT001Receiver::~MT001Receiver()
{

}

bool MT001Receiver::data_extraction(const std::string& data)
{
	bool sign_type_ex_res = this->extraction_sign_type(data);
	bool auth_type_ex_res = this->extraction_auth_type(data);
	bool client_id_ex_res = this->extraction_client_id(data);
	bool client_name_byte_size_ex_res = this->extraction_client_name_byte_size(data);
	bool client_name_ex_res = this->extraction_client_name(data);
	bool client_mail_addr_byte_size_ex_res = this->extraction_client_mail_addr_byte_size(data);
	bool client_mail_addr_ex_res = this->extraction_client_mail_addr(data);
	bool client_password_byte_size_ex_res = this->extraction_client_password_byte_size(data);
	bool client_password_ex_res = this->extraction_client_password(data);

	if (sign_type_ex_res == false || auth_type_ex_res == false
		||client_id_ex_res == false ||client_name_byte_size_ex_res == false
		|| client_name_ex_res == false || client_mail_addr_byte_size_ex_res == false
		|| client_mail_addr_ex_res == false || client_password_byte_size_ex_res == false
		|| client_password_ex_res == false) {
		return false;
	}

#ifdef DEBUG_MT001_CONTAIN
	std::cout << StringFormat("%s[DEBUG]SIGN Type(0: sign up 1: sign in, 2: sign out) : %d", ServerTime::get_current_time(), this->m_mt001_data.get_sign_type()) << std::endl;
	std::cout << StringFormat("%s[DEBUG]AUTH type : %d", ServerTime::get_current_time(), this->m_mt001_data.get_auth_type()) << std::endl;
	std::cout << StringFormat("%s[DEBUG]Client ID : %s", ServerTime::get_current_time(), this->m_mt001_data.get_client_id()) << std::endl;
	std::cout << StringFormat("%s[DEBUG]Client Name Byte Size : %d", ServerTime::get_current_time(), this->m_mt001_data.get_client_name_byte_size()) << std::endl;
	std::cout << StringFormat("%s[DEBUG]Client Name : %s", ServerTime::get_current_time(), this->m_mt001_data.get_client_name()) << std::endl;
	std::cout << StringFormat("%s[DEBUG]Client Mail Addr Byte Size : %d", ServerTime::get_current_time(), this->m_mt001_data.get_client_mail_addr_byte_size()) << std::endl;
	std::cout << StringFormat("%s[DEBUG]Client Mail Address : %s", ServerTime::get_current_time(), this->m_mt001_data.get_client_mail_addr()) << std::endl;
	std::cout << StringFormat("%s[DEBUG]Client Password Byte Size : %d", ServerTime::get_current_time(), this->m_mt001_data.get_client_password_byte_size()) << std::endl;
	std::cout << StringFormat("%s[DEBUG]Client Password : %s", ServerTime::get_current_time(), this->m_mt001_data.get_client_password()) << std::endl;
#endif // DEBUG_MT001_CONTAIN

	return true;
}


MT001Receiver::MT001Data::MT001Data()
{

}

MT001Receiver::MT001Data::~MT001Data()
{
}



const int MT001Receiver::MT001Data::get_sign_type()
{
	switch (m_sign_type) {
		case SIGN_TYPE::sign_up:
			return 0;
			break;
		case SIGN_TYPE::sign_in:
			return 1;
			break;
		case SIGN_TYPE::sign_out:
			return 2;
			break;
		default:
			return -1;
			break;
		}

}

const int MT001Receiver::MT001Data::get_auth_type()
{
	switch (m_auth_type)
	{
	case MT001Receiver::MT001Data::AUTH_TYPE::normal_auth:
		return 0;
		break;

	case MT001Receiver::MT001Data::AUTH_TYPE::google_auth:
		return 1;
		break;

	case MT001Receiver::MT001Data::AUTH_TYPE::facebook_auth:
		return 2;
		break;

	case MT001Receiver::MT001Data::AUTH_TYPE::twitter_auth:
		return 3;
		break;

	case MT001Receiver::MT001Data::AUTH_TYPE::microsoft_auth:
		return 4;
		break;

	case MT001Receiver::MT001Data::AUTH_TYPE::yahoo_auth:
		return 5;
		break;

	default:
		return -1;
		break;
	}
}

const std::string MT001Receiver::MT001Data::get_client_id()
{
	return this->m_client_id;
}

const uint8_t MT001Receiver::MT001Data::get_client_name_byte_size()
{
	return this->m_client_name_byte_size;
}

const std::string& MT001Receiver::MT001Data::get_client_name()
{
	return this->m_client_name;
}

const uint8_t MT001Receiver::MT001Data::get_client_mail_addr_byte_size()
{
	return this->m_client_mail_byte_size;
}

const std::string& MT001Receiver::MT001Data::get_client_mail_addr()
{
	return this->m_client_mail_addr;
}

const uint8_t MT001Receiver::MT001Data::get_client_password_byte_size()
{
	return this->m_client_password_byte_size;
}

const std::string& MT001Receiver::MT001Data::get_client_password()
{
	return this->m_client_password;
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

void MT001Receiver::MT001Data::set_auth_type(const uint8_t& auth_type)
{
	switch (auth_type)
	{
	case 0:
		this->m_auth_type = AUTH_TYPE::normal_auth;
		break;

	case 1:
		this->m_auth_type = AUTH_TYPE::google_auth;
		break;

	case 2:
		this->m_auth_type = AUTH_TYPE::facebook_auth;
		break;

	case 3:
		this->m_auth_type = AUTH_TYPE::twitter_auth;
		break;

	case 4:
		this->m_auth_type = AUTH_TYPE::microsoft_auth;
		break;

	case 5:
		this->m_auth_type = AUTH_TYPE::yahoo_auth;
		break;

	default:
		break;
	}
}

void MT001Receiver::MT001Data::set_client_id(const std::string& id)
{
	this->m_client_id = id;
}

void MT001Receiver::MT001Data::set_client_name_byte_size(const uint8_t& name_byte_size)
{
	this->m_client_name_byte_size = name_byte_size;
}

void MT001Receiver::MT001Data::set_client_name(const std::string& name)
{
	this->m_client_name = name;
}

void MT001Receiver::MT001Data::set_client_mail_addr_byte_size(const uint8_t& mail_addr_byte_size)
{
	this->m_client_mail_byte_size = mail_addr_byte_size;
}

void MT001Receiver::MT001Data::set_client_mail_addr(const std::string& mail_addr)
{
	this->m_client_mail_addr = mail_addr;
}

void MT001Receiver::MT001Data::set_client_password_byte_size(const uint8_t& password_byte_size)
{
	this->m_client_password_byte_size = password_byte_size;
}

void MT001Receiver::MT001Data::set_client_password(const std::string& password)
{
	this->m_client_password = password;
}

bool MT001Receiver::extraction_sign_type(const std::string& data)
{
	try
	{
		int start_pos_split = 3, end_pos_split = 1;
		if (start_pos_split > data.size() || (start_pos_split + end_pos_split) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}
		std::string sign_type_str = data.substr(start_pos_split, end_pos_split);
		int sign_type_int = std::stoi(sign_type_str);
		if (sign_type_int < 0 || sign_type_int > 2) {
			THROW_APP_EXCEPTION(StringFormat("The value of Trans type is invalid.(\033[31msign_type_int contains : %d\033[m)", sign_type_int));
		}
		m_mt001_data.set_sign_type(sign_type_int);
	}
	catch (const exception::ExceptionBase& ex)
	{
		std::cerr << StringFormat("%s\033[31m[ERORR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
		return false;
	}
	return true;
}

bool MT001Receiver::extraction_auth_type(const std::string& data)
{
	try
	{
		int start_pos_split = 4, end_pos_split = 1;
		if (start_pos_split > data.size() || (start_pos_split + end_pos_split) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}
		std::string auth_type_str = data.substr(start_pos_split, end_pos_split);
		int auth_type_int = std::stoi(auth_type_str);
		if (auth_type_int < 0 || auth_type_int > 5) {
			THROW_APP_EXCEPTION(StringFormat("The value of Trans type is invalid.(\033[31msign_type_int contains : %d\033[m)", auth_type_int));
		}
		m_mt001_data.set_auth_type(auth_type_int);
	}
	catch (const exception::ExceptionBase& ex)
	{
		std::cerr << StringFormat("%s\033[31m[ERORR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
		return false;
	}
	return true;
}

bool MT001Receiver::extraction_client_id(const std::string& data)
{
	try
	{
		uint8_t end_pos_split = 36;
		if (CLIENT_ID_START_POS > data.size() || (CLIENT_ID_START_POS + end_pos_split) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}
		std::string new_client_id = data.substr(CLIENT_ID_START_POS, end_pos_split);
		this->m_mt001_data.set_client_id(new_client_id);

	}
	catch (const std::out_of_range& oor_ex) {
		THROW_APP_EXCEPTION(StringFormat("Out of range：%s", oor_ex.what()));
	}
	catch (const exception::ExceptionBase& ex)
	{
		std::cerr << StringFormat("%s\033[31m[ERORR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
		return false;
	}
	return true;
}

bool MT001Receiver::extraction_client_name_byte_size(const std::string& data)
{
	try
	{
		int start_pos_split = 41, end_pos_split = 2;
		if (start_pos_split > data.size() || (start_pos_split + end_pos_split) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}
		std::string client_namebs_str = data.substr(start_pos_split, end_pos_split);
		uint8_t client_name_byte_size = std::stoi(data.substr(start_pos_split, end_pos_split));
		this->m_mt001_data.set_client_name_byte_size(client_name_byte_size);
	}
	catch (const std::invalid_argument& ia_ex) {
		THROW_APP_EXCEPTION(StringFormat("Invalid argument : %s", ia_ex.what()));
	}
	catch (const std::out_of_range& oor_ex) {
		THROW_APP_EXCEPTION(StringFormat("Out of range：%s", oor_ex.what()));
	}
	catch (const exception::ExceptionBase& ex)
	{
		std::cerr << StringFormat("%s\033[31m[ERORR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
		return false;
	}
	return true;
}

bool MT001Receiver::extraction_client_name(const std::string& data)
{
	try
	{
		uint8_t  end_pos_split = this->m_mt001_data.get_client_name_byte_size();
		if (CLIENT_NAME_START_POS > data.size() || (CLIENT_NAME_START_POS + end_pos_split) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}
		std::string new_client_name = data.substr(CLIENT_NAME_START_POS, end_pos_split);
		this->m_mt001_data.set_client_name(new_client_name);

	}
	catch (const std::out_of_range& oor_ex) {
		THROW_APP_EXCEPTION(StringFormat("Out of range：%s", oor_ex.what()));
	}
	catch (const exception::ExceptionBase& ex)
	{
		std::cerr << StringFormat("%s\033[31m[ERORR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
		return false;
	}
	return true;
}

bool MT001Receiver::extraction_client_mail_addr_byte_size(const std::string& data)
{
	try
	{
		if (this->m_mt001_data.get_client_name_byte_size() == 0) {
			return false;
		}
		uint8_t start_pos_split = CLIENT_NAME_START_POS + this->m_mt001_data.get_client_name_byte_size();
		uint8_t client_mail_byte_size = std::stoi(data.substr(start_pos_split, CLIENT_MAIL_ADDRESS_BYTE_SIZE));
		this->m_mt001_data.set_client_mail_addr_byte_size(client_mail_byte_size);
	}
	catch (const std::invalid_argument& ia_ex) {
		THROW_APP_EXCEPTION(StringFormat("Invalid argument : %s", ia_ex.what()));
	}
	catch (const std::out_of_range& oor_ex) {
		THROW_APP_EXCEPTION(StringFormat("Out of range：%s", oor_ex.what()));
	}
	catch (const exception::ExceptionBase& ex)
	{
		std::cerr << StringFormat("%s\033[31m[ERORR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
		return false;
	}

	return true;
}

bool MT001Receiver::extraction_client_mail_addr(const std::string& data)
{
	try
	{
		uint8_t start_pos_split = CLIENT_NAME_START_POS
								+ this->m_mt001_data.get_client_name_byte_size() 
								+ CLIENT_MAIL_ADDRESS_BYTE_SIZE;
		uint8_t end_pos_split = this->m_mt001_data.get_client_mail_addr_byte_size();
		std::string new_client_gmail = data.substr(start_pos_split, end_pos_split);
		this->m_mt001_data.set_client_mail_addr(new_client_gmail);
	}
	catch (const std::out_of_range& oor_ex) {
		THROW_APP_EXCEPTION(StringFormat("Out of range：%s", oor_ex.what()));
	}
	catch (const exception::ExceptionBase& ex)
	{
		std::cerr << StringFormat("%s\033[31m[ERORR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
		return false;
	}
	return true;
}

bool MT001Receiver::extraction_client_password_byte_size(const std::string& data)
{
	try
	{
		if (this->m_mt001_data.get_client_name_byte_size() == 0 || this->m_mt001_data.get_client_mail_addr_byte_size() == 0) {
			return false;
		}
		uint8_t start_pos_split = CLIENT_NAME_START_POS 
								+ this->m_mt001_data.get_client_name_byte_size()
								+ CLIENT_MAIL_ADDRESS_BYTE_SIZE 
								+ this->m_mt001_data.get_client_mail_addr_byte_size();
		uint8_t client_password_byte_size = std::stoi(data.substr(start_pos_split, CLIENT_PASSWORD_BYTE_SIZE));
		this->m_mt001_data.set_client_password_byte_size(client_password_byte_size);
	}
	catch (const std::invalid_argument& ia_ex) {
		THROW_APP_EXCEPTION(StringFormat("Invalid argument : %s", ia_ex.what()));
	}
	catch (const std::out_of_range& oor_ex) {
		THROW_APP_EXCEPTION(StringFormat("Out of range：%s", oor_ex.what()));
	}
	catch (const exception::ExceptionBase& ex)
	{
		std::cerr << StringFormat("%s\033[31m[ERORR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
		return false;
	}

	return true;
}

bool MT001Receiver::extraction_client_password(const std::string& data)
{
	try
	{
		uint8_t start_pos_split = CLIENT_NAME_START_POS
			+ this->m_mt001_data.get_client_name_byte_size()
			+ CLIENT_MAIL_ADDRESS_BYTE_SIZE
			+ this->m_mt001_data.get_client_mail_addr_byte_size()
			+ CLIENT_PASSWORD_BYTE_SIZE;
		uint8_t end_pos_split = this->m_mt001_data.get_client_password_byte_size();
		std::string new_client_password = data.substr(start_pos_split, end_pos_split);
		this->m_mt001_data.set_client_password(new_client_password);
	}
	catch (const std::out_of_range& oor_ex) {
		THROW_APP_EXCEPTION(StringFormat("Out of range：%s", oor_ex.what()));
	}

	catch (const exception::ExceptionBase& ex)
	{
		std::cerr << StringFormat("%s\033[31m[ERORR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
		return false;

	}
	return true;
}

MT001Receiver::MT001Data& MT001Receiver::get_mt001_data()
{
	return this->m_mt001_data;
}

