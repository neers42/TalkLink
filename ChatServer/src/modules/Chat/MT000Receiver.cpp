#include "MT000Receiver.h"

MT000Receiver::MT000Data::MT000Data()
{
	src_client_id = "";
}

MT000Receiver::MT000Data::MT000Data(const std::string& id)
{
	src_client_id = id;
}

MT000Receiver::MT000Data::~MT000Data()
{

}

void MT000Receiver::MT000Data::print_message_contents()
{
	LogHandler::get_instance().logging_debug(StringFormat("%s", this->main_content));
}




std::string MT000Receiver::extraction_src_client_id(const std::string& data)
{
	try
	{
		if (CLIENT_ID_START_POS > data.size() || (CLIENT_ID_START_POS + CLIENT_ID_BYTE_SIZE) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}
		std::string client_id_str = data.substr(CLIENT_ID_START_POS, CLIENT_ID_BYTE_SIZE);
		return client_id_str;
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return std::string("");
	}
}

bool MT000Receiver::check_validation_src_client_id(const std::string& id)
{
	if (id.size() != 28) {
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







void MT000Receiver::show_receive_info(const std::string& ipv4_addr, int& detect_num, int& receive_result)
{
}

std::unique_ptr<MT000Receiver::MT000Data> MT000Receiver::create_mt000_instance()
{
	return std::unique_ptr<MT000Data>();
}

std::unique_ptr<MT000Receiver::MT000Data> MT000Receiver::create_mt000_instance(const std::string& id)
{
	std::unique_ptr<MT000Data> new_mt000;
	new_mt000 = std::make_unique<MT000Data>(id);
	return new_mt000;
}

std::string MT000Receiver::data_extraction_(const std::string& data)
{
	std::string src_client_id = this->extraction_src_client_id(data);
	if (check_validation_src_client_id(src_client_id) == false) {
		return std::string("");
	}
	return src_client_id;
}
