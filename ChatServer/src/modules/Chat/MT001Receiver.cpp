#include "MT001Receiver.h"

MT001Receiver::MT001Receiver()
{

}

MT001Receiver::~MT001Receiver()
{

}


void MT001Receiver::show_receive_info(const std::string& ipv4_addr, int& detect_num, int& receive_result)
{
	LogHandler::get_instance().logging_info(StringFormat("\033[33mMT001\033[m received from client[%d](IP address : %s) ⇒ SERVER).", detect_num, ipv4_addr, receive_result));
}

void MT001Receiver::show_receive_info(const json& receive_json, const std::string& ip, int receive_byte)
{
	LogHandler::get_instance().logging_info(StringFormat("%s, \033[33mMT001\033[m message receive, success, sever received %d bytes message from client.\nReceived JSON : %s"
		, ip, receive_byte, receive_json.dump(4)));
}

void MT001Receiver::show_mt001_data(const MT001Data& mt001)
{
//	LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("SIGN Type(0: sign up 1: sign in, 2: sign out) : %d", ));
//	LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("AUTH type : %d", this->m_mt001_data.get_auth_type()));
//	LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Client Name Byte Size : %d", this->m_mt001_data.get_client_name_byte_size()));
//	LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Client Name : %s", this->m_mt001_data.get_client_name()));
//	LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Client Mail Addr Byte Size : %d", this->m_mt001_data.get_client_mail_addr_byte_size()));
//	LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Client Mail Address : %s", this->m_mt001_data.get_client_mail_addr()));
//	LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Client Password Byte Size : %d", this->m_mt001_data.get_client_password_byte_size()));
//	LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Client Password : %s", this->m_mt001_data.get_client_password()));
}


MT001Receiver::MT001Data::MT001Data(): m_sign_type{SIGN_TYPE::sign_up}, m_auth_type{AUTH_TYPE::normal_auth},m_client_id("")
, m_client_name_byte_size{0}, m_client_name{""}, m_client_mail_byte_size{0}, m_client_mail_addr{""}, m_client_password_byte_size{0}, m_client_password{""}
{
	time_stamp = ServerTime::get_current_time();
}

MT001Receiver::MT001Data::~MT001Data()
{
	
}

MT001Receiver::MT001Data::MT001Data(const uint8_t& sign_type, const uint8_t& auth_type, const std::string& name, const std::string& mail_addr, const std::string& password)
	: m_client_name{name}, m_client_mail_addr{mail_addr}, m_client_password{password}
{
	switch (sign_type)
	{
	case 0:
		m_sign_type = SIGN_TYPE::sign_up;
		break;

	case 1:
		m_sign_type = SIGN_TYPE::sign_in;
		break;

	case 2:
		m_sign_type = SIGN_TYPE::sign_out;
		break;

	default:
		break;
	}

	switch (auth_type)
	{
	case 0:
		m_auth_type = AUTH_TYPE::normal_auth;
		break;

	case 1:
		m_auth_type = AUTH_TYPE::google_auth;
		break;

	case 2:
		m_auth_type = AUTH_TYPE::facebook_auth;
		break;

	case 3:
		m_auth_type = AUTH_TYPE::twitter_auth;
		break;

	case 4:
		m_auth_type = AUTH_TYPE::microsoft_auth;
		break;

	case 5:
		m_auth_type = AUTH_TYPE::yahoo_auth;
		break;

	default:
		break;
	}
	time_stamp = ServerTime::get_current_time();
}

MT001Receiver::MT001Data::MT001Data(const uint8_t& sign_type
	, const uint8_t& auth_type, const uint8_t name_byte_size
	, const std::string& name, const uint8_t mail_addr_byte_size
	, const std::string& mail_addr, const uint8_t password_byte_size
	, const std::string& password) 
	: m_client_id{ "" }, m_client_name_byte_size{ name_byte_size }, m_client_name{ name }
	, m_client_mail_byte_size{mail_addr_byte_size}, m_client_mail_addr{mail_addr}
	, m_client_password_byte_size{password_byte_size}, m_client_password{password}

	
{
	switch (sign_type)
	{
	case 0:
		m_sign_type = SIGN_TYPE::sign_up;
		break;

	case 1:
		m_sign_type = SIGN_TYPE::sign_in;
		break;

	case 2:
		m_sign_type = SIGN_TYPE::sign_out;
		break;

	default:
		break;
	}

	switch (auth_type)
	{
	case 0:
		m_auth_type = AUTH_TYPE::normal_auth;
		break;

	case 1:
		m_auth_type = AUTH_TYPE::google_auth;
		break;

	case 2:
		m_auth_type = AUTH_TYPE::facebook_auth;
		break;

	case 3:
		m_auth_type = AUTH_TYPE::twitter_auth;
		break;

	case 4:
		m_auth_type = AUTH_TYPE::microsoft_auth;
		break;

	case 5:
		m_auth_type = AUTH_TYPE::yahoo_auth;
		break;

	default:
		break;
	}
	time_stamp = ServerTime::get_current_time();
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

void MT001Receiver::MT001Data::print_message_contents()
{

}


uint8_t MT001Receiver::extraction_sign_type_(const std::string& data)
{
	try
	{
		if (SIGN_TYPE_START_POS > data.size() || (SIGN_TYPE_START_POS + SIGN_TYPE_BYTE_SIZE) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}
		std::string sign_type_str = data.substr(SIGN_TYPE_START_POS, SIGN_TYPE_BYTE_SIZE);
		int sign_type_int = std::stoi(sign_type_str);
		return sign_type_int;
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return -1;
	}
}


uint8_t MT001Receiver::extraction_auth_type_(const std::string& data)
{
	try
	{
		if (AUTH_TYPE_START_POS > data.size() || (AUTH_TYPE_START_POS + AUTH_TYPE_BYTE_SIZE) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}
		std::string auth_type_str = data.substr(AUTH_TYPE_START_POS, AUTH_TYPE_BYTE_SIZE);
		uint8_t auth_type_int = std::stoi(auth_type_str);
		return auth_type_int;
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return -1;
	}
}




uint8_t MT001Receiver::extraction_client_name_byte_size_(const std::string& data)
{
	try
	{
		if (CLIENT_NAME_BYTE_SIZE_START_POS > data.size() || (CLIENT_NAME_BYTE_SIZE_START_POS + CLIENT_NAME_BYTE_SIZE) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}
		std::string client_namebs_str = data.substr(CLIENT_NAME_BYTE_SIZE_START_POS, CLIENT_NAME_BYTE_SIZE);
		uint8_t client_name_byte_size = std::stoi(data.substr(CLIENT_NAME_BYTE_SIZE_START_POS, CLIENT_NAME_BYTE_SIZE));
		return client_name_byte_size;
	}
	catch (const std::invalid_argument& ia_ex) {
		THROW_APP_EXCEPTION(StringFormat("Invalid argument : %s", ia_ex.what()));
	}
	catch (const std::out_of_range& oor_ex) {
		THROW_APP_EXCEPTION(StringFormat("Out of range：%s", oor_ex.what()));
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return -1;
	}
}


std::string MT001Receiver::extraction_client_name_(const std::string& data, uint8_t client_name_byte_size)
{
	try
	{
		if (CLIENT_NAME_START_POS > data.size() || (CLIENT_NAME_START_POS + client_name_byte_size) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}
		std::string client_name = data.substr(CLIENT_NAME_START_POS, client_name_byte_size);
		return client_name;
	}
	catch (const std::out_of_range& oor_ex) {
		THROW_APP_EXCEPTION(StringFormat("Out of range：%s", oor_ex.what()));
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return std::string("");
	}
}


uint8_t MT001Receiver::extraction_client_mail_addr_byte_size_(const std::string& data, uint8_t name_byte_size)
{
	try
	{
		uint8_t start_pos_split = CLIENT_NAME_START_POS + name_byte_size;
		uint8_t client_mail_byte_size = std::stoi(data.substr(start_pos_split, CLIENT_MAIL_ADDRESS_BYTE_SIZE));
		return client_mail_byte_size;
	}
	catch (const std::invalid_argument& ia_ex) {
		THROW_APP_EXCEPTION(StringFormat("Invalid argument : %s", ia_ex.what()));
	}
	catch (const std::out_of_range& oor_ex) {
		THROW_APP_EXCEPTION(StringFormat("Out of range：%s", oor_ex.what()));
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return -1;
	}
}


std::string MT001Receiver::extraction_client_mail_addr_(const std::string& data, uint8_t name_byte_size, uint8_t mail_addr_byte_size)
{
	try
	{
		uint8_t start_pos_split = CLIENT_NAME_START_POS
			+ name_byte_size
			+ CLIENT_MAIL_ADDRESS_BYTE_SIZE;
		uint8_t end_pos_split = mail_addr_byte_size;
		std::string client_mail_addr = data.substr(start_pos_split, end_pos_split);
		return client_mail_addr;
	}
	catch (const std::out_of_range& oor_ex) {
		THROW_APP_EXCEPTION(StringFormat("Out of range：%s", oor_ex.what()));
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return std::string("");
	}
}


uint8_t MT001Receiver::extraction_client_password_byte_size_(const std::string& data, uint8_t name_byte_size, uint8_t mail_addr_byte_size)
{
	try
	{
			uint8_t start_pos_split = CLIENT_NAME_START_POS
			+ name_byte_size
			+ CLIENT_MAIL_ADDRESS_BYTE_SIZE
			+ mail_addr_byte_size;
		uint8_t client_password_byte_size = std::stoi(data.substr(start_pos_split, CLIENT_PASSWORD_BYTE_SIZE));
		return client_password_byte_size;
	}
	catch (const std::invalid_argument& ia_ex) {
		THROW_APP_EXCEPTION(StringFormat("Invalid argument : %s", ia_ex.what()));
	}
	catch (const std::out_of_range& oor_ex) {
		THROW_APP_EXCEPTION(StringFormat("Out of range：%s", oor_ex.what()));
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return -1;
	}
}


std::string MT001Receiver::extraction_client_password_(const std::string& data, uint8_t name_byte_size, uint8_t mail_addr_byte_size, uint8_t password_byte_size)
{
	try
	{
		uint8_t start_pos_split = CLIENT_NAME_START_POS
			+ name_byte_size
			+ CLIENT_MAIL_ADDRESS_BYTE_SIZE
			+ mail_addr_byte_size
			+ CLIENT_PASSWORD_BYTE_SIZE;
		uint8_t end_pos_split = password_byte_size;
		std::string client_password = data.substr(start_pos_split, end_pos_split);
		return client_password;
	}
	catch (const std::out_of_range& oor_ex) {
		THROW_APP_EXCEPTION(StringFormat("Out of range：%s", oor_ex.what()));
	}

	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return std::string("");

	}
}

bool MT001Receiver::check_validation_sign_type(const uint8_t& sign_type)
{
	if (sign_type > MAX_SIGN_TYPE || sign_type < MIN_SIGN_TYPE) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("MT001 contents sign_type is invalid. sign_type : %d (sign_type : 0 - 2)", sign_type));
		return false;
	}
	return true;
}

bool MT001Receiver::check_validation_auth_type(const uint8_t& auth_type)
{
	if (auth_type > MAX_AUTH_TYPE || auth_type < MIN_AUTH_TYPE) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr
			, StringFormat("MT001 contents auth_type is invalid. auth_type : %d (auth_type : 0 - 5)", auth_type));
		return false;
	}
	return true;
}

bool MT001Receiver::check_validation_client_name_byte_size(const uint8_t& name_byte_size)
{
	if (name_byte_size < MIN_NAME_BYTE_SIZE || MAX_NAME_BYTE_SIZE > 12) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr
			, StringFormat("MT001 contents name_byte_size ERROR. name_byte_size : %d (name_byte_size : 6-12)", name_byte_size));
		return false;
	}
	return true;
}

bool MT001Receiver::check_validation_client_name(const std::string& name)
{
	if (name.size() < MIN_NAME_BYTE_SIZE || name.size() > MAX_NAME_BYTE_SIZE) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr
			, StringFormat("MT001 contents name size ERROR. name size : %d (name size : 6 -12)", name.size()));
		return false;
	}
	return true;
}

bool MT001Receiver::check_validation_client_mail_addr_byte_size(const uint8_t& mail_addr_byte_size)
{
	if (mail_addr_byte_size > MAX_MAIL_ADDR_SIZE || mail_addr_byte_size < MIN_MAIL_ADDR_SIZE) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr
			, StringFormat("MT001 contents mail addr byte size ERROR. mail addr byte size : %d (mail addr byte size : 3 - 254)", mail_addr_byte_size));
		return false;
	}
	return true;
}

bool MT001Receiver::check_validation_client_mail_addr(const std::string& mail_addr)
{
	if (mail_addr.size() < MIN_MAIL_ADDR_SIZE || mail_addr.size() > MAX_MAIL_ADDR_SIZE) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr
			, StringFormat("MT001 contents mail addr size ERROR. mail addr size : %d (mail addr size : 3 - 254)", mail_addr.size()));
		return false;
	}
	return true;
}

bool MT001Receiver::check_validation_client_password_byte_size(const uint8_t& password_byte_size)
{
	if (password_byte_size < MIN_PASSWORD_SIZE || password_byte_size > MAX_PASSWORD_SIZE) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr
			, StringFormat("MT001 contents password byte size ERROR. password byte size : %d (password byte size 10 - 127)"));
		return false;
	}
	return true;
}

bool MT001Receiver::check_validation_client_password(const std::string& password)
{
	if (password.size() < MIN_PASSWORD_SIZE || password.size() > MAX_PASSWORD_SIZE) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr
			, StringFormat("MT001 contents password size ERROR. password size : %d (password size 10 - 127)"));
		return false;
	}
	return true;
}

bool MT001Receiver::check_validation_contents(const uint8_t& sign_type, const uint8_t& auth_type, const uint8_t name_byte_size, const std::string& name, const uint8_t mail_addr_byte_size, const std::string& mail_addr, const uint8_t password_byte_size, const std::string& password)
{
	if (check_validation_sign_type(sign_type) == false
		|| check_validation_auth_type(auth_type) == false
		|| check_validation_client_name_byte_size(name_byte_size) == false
		|| check_validation_client_name(name) == false
		|| check_validation_client_mail_addr_byte_size(mail_addr_byte_size) == false
		|| check_validation_client_mail_addr(mail_addr) == false
		|| check_validation_client_password_byte_size(password_byte_size) == false
		|| check_validation_client_password(password) == false) {
		return false;
	}
	return true;
}



std::unique_ptr<MT001Receiver::MT001Data> MT001Receiver::create_mt001data_instance()
{
	return std::unique_ptr<MT001Data>();
}

std::unique_ptr<MT001Receiver::MT001Data> MT001Receiver::create_mt001data_instance(const uint8_t& sign_type, const uint8_t& auth_type, std::string& name, const std::string& mail_addr, const std::string& password)
{
	std::unique_ptr<MT001Data> new_mt001;
	new_mt001 = std::make_unique<MT001Data>(sign_type, auth_type, name, mail_addr, password);
	return new_mt001;
}

std::unique_ptr<MT001Receiver::MT001Data> MT001Receiver::create_mt001data_instance(const uint8_t& sign_type, const uint8_t& auth_type, const uint8_t& name_byte_size, std::string& name, const uint8_t& mail_addr_byte_size, const std::string& mail_addr, const uint8_t& password_byte_size, const std::string& password)
{
	std::unique_ptr<MT001Data> new_mt001;
	new_mt001 = std::make_unique<MT001Data>(sign_type, auth_type, name_byte_size, name, mail_addr_byte_size, mail_addr, password_byte_size, password);
	return new_mt001;
}

bool MT001Receiver::data_extraction(const std::string& data, MT001Data& mt001)
{
	uint8_t sign_type = this->extraction_auth_type_(data);
	uint8_t auth_type = this->extraction_auth_type_(data);
	uint8_t client_name_byte_size = this->extraction_client_name_byte_size_(data);
	std::string client_name = this->extraction_client_name_(data, client_name_byte_size);
	uint8_t client_mail_addr_byte_size = this->extraction_client_mail_addr_byte_size_(data, client_name_byte_size);
	std::string client_mail_addr = this->extraction_client_mail_addr_(data, client_name_byte_size, client_mail_addr_byte_size);
	uint8_t client_password_byte_size = this->extraction_client_password_byte_size_(data, client_name_byte_size, client_mail_addr_byte_size);
	std::string client_password = this->extraction_client_password_(data, client_name_byte_size, client_mail_addr_byte_size, client_password_byte_size);

	if (check_validation_contents(sign_type, auth_type, client_name_byte_size, client_name,
		client_mail_addr_byte_size, client_mail_addr, client_password_byte_size, client_password) == false) {
		return false;
	}
	
	mt001.set_sign_type(sign_type);
	mt001.set_auth_type(auth_type);
	mt001.set_client_name_byte_size(client_name_byte_size);
	mt001.set_client_name(client_name);
	mt001.set_client_mail_addr_byte_size(client_mail_addr_byte_size);
	mt001.set_client_mail_addr(client_mail_addr);
	mt001.set_client_password_byte_size(client_password_byte_size);
	mt001.set_client_password(client_password);

	return true;

}

std::tuple<uint8_t, uint8_t, std::string, std::string, std::string> MT001Receiver::data_extraction_(const json& receive_json)
{
	//auto receive_json = json::parse(receive_json);
	uint8_t sign_type = receive_json["sign_type"];
	uint8_t auth_type = receive_json["auth_type"];
	std::string client_name = receive_json["name"];
	std::string client_mail_addr = receive_json["mail_address"];
	std::string client_password = receive_json["password"];


	//uint8_t sign_type = this->extraction_sign_type_(data);
	//uint8_t auth_type = this->extraction_auth_type_(data);
	//uint8_t client_name_byte_size = this->extraction_client_name_byte_size_(data);
	//std::string client_name = this->extraction_client_name_(data, client_name_byte_size);
	//uint8_t client_mail_addr_byte_size = this->extraction_client_mail_addr_byte_size_(data, client_name_byte_size);
	//std::string client_mail_addr = this->extraction_client_mail_addr_(data, client_name_byte_size, client_mail_addr_byte_size);
	//uint8_t client_password_byte_size = this->extraction_client_password_byte_size_(data, client_name_byte_size, client_mail_addr_byte_size);
	//std::string client_password = this->extraction_client_password_(data, client_name_byte_size, client_mail_addr_byte_size, client_password_byte_size);

	//if (check_validation_contents(sign_type, auth_type, client_name_byte_size, client_name,
	//	client_mail_addr_byte_size, client_mail_addr, client_password_byte_size, client_password) == false) {
	//	return std::make_tuple(3, 6, 0, "", 0, "", 0, "");
	//}
	return std::make_tuple(sign_type, auth_type, client_name, client_mail_addr, client_password);
}



