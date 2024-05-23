#include "MT000Receiver.h"

MT000Receiver::MT000Data::MT000Data() : client_id("")
{

}

MT000Receiver::MT000Data::MT000Data(const std::string& id) : client_id{id}
{
	
}

MT000Receiver::MT000Data::~MT000Data()
{

}

std::string MT000Receiver::MT000Data::get_mt000_client_id()
{
	return this->client_id;
}

void MT000Receiver::MT000Data::set_mt000_client_id(const std::string& id)
{
	this->client_id = id;
}


bool MT000Receiver::extract_client_id(const std::string& data)
{
	try
	{
		//int start_pos_split = 3, end_pos_split = 1;
		if (CLIENT_ID_START_POS > data.size() || (CLIENT_ID_START_POS + CLIENT_ID_BYTE_SIZE) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}
		std::string client_id_str = data.substr(CLIENT_ID_START_POS, CLIENT_ID_BYTE_SIZE);
		this->set_mt000_data_client_id(client_id_str);
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return false;
	}
	return true;
}

MT000Receiver::MT000Receiver()
{
}

MT000Receiver::~MT000Receiver()
{
}

MT000Receiver::MT000Data MT000Receiver::get_mt000_data()
{
	return this->mt000_data;
}

std::string MT000Receiver::get_mt000_client_id()
{
	return this->mt000_data.get_mt000_client_id();
}

void MT000Receiver::set_mt000_data_client_id(const std::string& id)
{
	this->mt000_data.set_mt000_client_id(id);
}


bool MT000Receiver::data_extraction(const std::string& data)
{
	bool client_id_ex_res = this->extract_client_id(data);
	if (client_id_ex_res == false) {
		return false;
	}
	return true;
}

void MT000Receiver::show_receive_info(const std::string& ipv4_addr, int& detect_num, int& receive_result)
{
}
