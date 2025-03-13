#include "MT002Receiver.h"


MT002Receiver::MT002Receiver()
{

}

MT002Receiver::~MT002Receiver()
{
}



/**********************************************************************************
 * @fn	  extraction_data_type
 * @brief 受信データからデータタイプを抽出する
 *
 * @param [in] std::string& data
 *             受信データ
 *
 * @return [out]
 *
 **********************************************************************************/
int MT002Receiver::extraction_trans_type(const std::string& data)
{
	try
	{
		if (SPLIT_START_POS > data.size() || (SPLIT_START_POS + TRANS_TYPE_BYTE_SIZE) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}
		std::string trans_type_string = data.substr(SPLIT_START_POS, TRANS_TYPE_BYTE_SIZE);
		int trans_type_int = std::stoi(trans_type_string);
		return trans_type_int;
	}

	catch (const std::out_of_range& oor_ex) {
		THROW_APP_EXCEPTION(StringFormat("Out of range : %s", oor_ex.what()));
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return -1;
	}
}


std::string MT002Receiver::extraction_dst_client_id(const std::string& data)
{
	try
	{
		if (CLIENT_DST_ID_START_POS > data.size() || (CLIENT_DST_ID_START_POS + CLIENT_DST_ID_BYTE_SIZE) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}

		std::string dst_client_id = data.substr(CLIENT_DST_ID_START_POS, CLIENT_DST_ID_BYTE_SIZE);
		return dst_client_id;
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


std::string MT002Receiver::extraction_src_client_id(const std::string& data)
{
	try
	{
		if (CLIENT_SRC_ID_START_POS > data.size() || (CLIENT_SRC_ID_START_POS + CLIENT_SRC_ID_BYTE_SIZE) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}

		std::string src_client_id = data.substr(CLIENT_SRC_ID_START_POS, CLIENT_SRC_ID_BYTE_SIZE);
		return src_client_id;
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


/**********************************************************************************
 * @fn	  extraction_scr_ip
 * @brief
 *
 * @param [in]
 *
 * @return [out]
 *
 **********************************************************************************/

std::string MT002Receiver::extraction_dst_chatroom_id(const std::string& data)
{
	try
	{
		if (CHATROOM_ID_START_POS > data.size() || (CHATROOM_ID_START_POS + CHATROOM_ID_BYTE_SIZE) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}

		std::string chatroom_id = data.substr(CHATROOM_ID_START_POS, CHATROOM_ID_BYTE_SIZE);
		return chatroom_id;
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

/**********************************************************************************
 * @fn	  extraction_data_type
 * @brief 受信データからデータタイプを抽出する
 *
 * @param [in] std::string& data
 *             受信データ
 * 
 * @return [out]
 *
 **********************************************************************************/

int MT002Receiver::extraction_data_type(const std::string& data)
{
	try
	{
		if (DATA_TYPE_START_POS > data.size() || (DATA_TYPE_START_POS + DATA_TYPE_BYTE_SIZE) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}
		std::string data_type_string = data.substr(DATA_TYPE_START_POS, DATA_TYPE_BYTE_SIZE);
		int data_type_int = std::stoi(data_type_string);
		return data_type_int;
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


/**********************************************************************************
 * @fn	  extraction_main_data
 * @brief 受信データからメインのデータを抽出する。 
 *
 * @param [in]
 *
 * @return [out]
 *
 **********************************************************************************/

std::string MT002Receiver::extraction_main_content(const std::string& data)
{
	try
	{
		if (MAIN_DATA_START_POS > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}
		std::string main_content = data.substr(MAIN_DATA_START_POS);
		return main_content;
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return std::string("");
	}
}

bool MT002Receiver::check_validation_trans_type(const int& trans_type)
{
	if (trans_type < 0 || trans_type > 2) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("MT002 contents trans_type is invalid. trans_type : %d", trans_type));
		return false;
	}
	return true;
}

bool MT002Receiver::check_validation_dst_client_id(const std::string& d_id)
{
	if (d_id.size() != 28) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("MT002 contents dst_client_id is invalid. dst_client_id byte size : %d", d_id.size()));
		return false;
	}
	return true;
}

bool MT002Receiver::check_validation_src_client_id(const std::string& s_id)
{
	if (s_id.size() != 28) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("MT002 contents src_client_id is invalid. src_client_id byte size : %d", s_id.size()));
		return false;
	}
	return true;
}

bool MT002Receiver::check_validation_dst_chatroom_id(const std::string& d_chatroom_id)
{
	if (d_chatroom_id.size() != 36) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("MT002 contents dst_chatroom_id is invalid. dst_chatroom_id byte size : %d", d_chatroom_id.size()));
		return false;
	}
	return true;
}

bool MT002Receiver::check_validation_data_type(const int& data_type)
{
	if (data_type < 0 || data_type > 3) {
		LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("MT002 contents data_type is invalid. data_type : %d", data_type));
		return false;
	}
	return true;
}

bool MT002Receiver::check_validation_main_content(const std::string& main_content)
{
	if (main_content.size() > 0) {
		return true;
	}
	LogHandler::logging(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("MT002 contents main_content is invalid. main_content_byte_size : %d", main_content.size()));
	return false;
}

bool MT002Receiver::check_validation_contents(const int& trans_type, const std::string& d_id, const std::string& s_id, const std::string& d_chatroom_id, const int& data_type, const std::string& main_content)
{
	if (check_validation_trans_type(trans_type) == false
		|| check_validation_dst_client_id(d_id) == false
		|| check_validation_src_client_id(s_id) == false
		|| check_validation_dst_chatroom_id(d_chatroom_id) == false
		|| check_validation_data_type(data_type) == false
		|| check_validation_main_content(main_content) == false) {
		return false;
	}
	return true;
}

/**********************************************************************************
 * @fn	  extraction_scr_ip
 * @brief
 *
 * @param [in]
 *
 * @return [out]
 *
 **********************************************************************************/
bool MT002Receiver::store_receive_data()
{
	return false;
}

//受信した暗号化されたメッセージを表示する。
void MT002Receiver::print_src_recv_data(const std::string& data, const std::string& dst_ipv4_addr, int& detection_num, int& receive_result)
{
	LogHandler::logging(LogHandler::LogType::RECEIVE, std::cout, StringFormat("%d byte message was received from client list[%d] (IP address：%s)", receive_result, detection_num, dst_ipv4_addr));
}


bool MT002Receiver::data_extraction(const std::string& data)
{
	return true;
}


std::string MT002Receiver::remove_zero_padding(std::string& ip_address)
{
	std::stringstream ss(ip_address);
	std::vector<std::string> segments;
	std::string segment;

	while (std::getline(ss, segment, '.')) {
		// ゼロパディングを削除してベクトルにセグメントを追加
		segments.push_back(std::to_string(std::stoi(segment)));
	}

	// ベクトルのセグメントを"."で連結して新しいIPアドレスを作成
	std::string new_ip_address = segments[0];
	for (size_t i = 1; i < segments.size(); ++i) {
		new_ip_address += "." + segments[i];
	}

	return new_ip_address;
}

MT002Receiver::MT002Data::MT002Data():m_trans_type{trans_type::unicast}, m_data_type{data_type::message}
, m_dst_ip_address{""}, m_src_ip_address{""}, m_dst_chatroom_id{""}
{
	this->dst_client_id = "";
	this->src_client_id = "";
	this->main_content = "";
}

MT002Receiver::MT002Data::MT002Data(const int& trans_type, const std::string& d_id, const std::string& s_id, const std::string& dst_chatroom_id, const int& data_type, const std::string& content)
	: m_dst_chatroom_id{dst_chatroom_id}
{
	switch (trans_type)
	{
	case 0:
		m_trans_type = trans_type::unicast;
		break;

	case 1:
		m_trans_type = trans_type::multicast;
		break;

	case 2:
		m_trans_type = trans_type::bloadcast;
		break;

	default:
		break;
	}

	this->dst_client_id = d_id;
	this->src_client_id = s_id;

	switch (data_type)
	{
	case 0:
		this->m_data_type = data_type::message;
		break;
	case 1:
		this->m_data_type = data_type::image;
		break;
	case 2:
		this->m_data_type = data_type::video;
		break;
	case 3:
		this->m_data_type = data_type::contact_details;
		break;

	default:
		break;
	}
	this->main_content = content;
}

MT002Receiver::MT002Data::~MT002Data()
{
}

void MT002Receiver::MT002Data::set_trans_type(const int& trans_type)
{
	switch (trans_type)
	{
	case 0:
		this->m_trans_type = trans_type::unicast;
		break;

	case 1:
		this->m_trans_type = trans_type::multicast;
		break;

	case 2:
		this->m_trans_type = trans_type::bloadcast;
		break;

	default:
		break;
	}
}

void MT002Receiver::MT002Data::set_data_type(const int& data_type)
{
	switch (data_type)
	{
	case 0:
		this->m_data_type = data_type::message;
		break;

	case 1:
		this->m_data_type = data_type::image;
		break;
	case 2:
		this->m_data_type = data_type::video;
		break;
	case 3:
		this->m_data_type = data_type::contact_details;
		break;
	default:
		break;
	}
}

void MT002Receiver::MT002Data::set_dst_chatroom_id(const std::string& dst_chatroom_id)
{
	this->m_dst_chatroom_id = dst_chatroom_id;
}


const int MT002Receiver::MT002Data::get_trans_type()
{
     switch (m_trans_type)
     {
	 case MT002Receiver::MT002Data::trans_type::unicast:
		 break;
	 case MT002Receiver::MT002Data::trans_type::multicast:
		 return 1;
		 break;
	 case MT002Receiver::MT002Data::trans_type::bloadcast:
		 return 2;
		 break;
	 default:
		 return -1;
		 break;
	 }
	return 0;
}

const int MT002Receiver::MT002Data::get_data_type()
{
	switch (m_data_type)
	{
	case MT002Receiver::MT002Data::data_type::message:
		return 0;
		break;
	case MT002Receiver::MT002Data::data_type::image:
		return 1;
		break;
	case MT002Receiver::MT002Data::data_type::video:
		return 2;
		break;
	case MT002Receiver::MT002Data::data_type::contact_details:
		return 3;
		break;
	default:
		return -1;
		break;
	}
	return 0;
}


std::string& MT002Receiver::MT002Data::get_dst_chatroom_id()
{
	return this->m_dst_chatroom_id;
}


void MT002Receiver::MT002Data::print_message_contents()
{
	LogHandler::get_instance().logging_debug(StringFormat("MT002 Contents"));
	LogHandler::get_instance().logging_debug(StringFormat("Transport Type(0: unicast 1: multicast, 2:bloadcast) : %d", this->get_trans_type()));
	LogHandler::get_instance().logging_debug(StringFormat("Dst client id : %s", this->get_src_client_id()));
	LogHandler::get_instance().logging_debug(StringFormat("Source Client ID : %s", this->get_src_client_id()));
	LogHandler::get_instance().logging_debug(StringFormat("Dist Chatroom ID : %s", this->get_dst_chatroom_id()));
	LogHandler::get_instance().logging_debug(StringFormat("Data Type(0: message 1: image 2: video 3: contact_details) : %d", this->get_data_type()));
	LogHandler::get_instance().logging_debug(StringFormat("Main Content : %s", this->get_main_content()));
}




void MT002Receiver::show_receive_info(const std::string& ipv4_addr, int& detect_num, int& receive_result)
{
	LogHandler::logging(LogHandler::LogType::RECEIVE, std::cout, StringFormat("\033[33mMT002\033[m received from client list [%d] (CLIENT(IP:%s) ⇒ SERVER). (BYTE size of the message: %d)", detect_num, ipv4_addr, receive_result));
}

void MT002Receiver::show_receive_info(const json& receive_json, const std::string& ip, int receive_byte)
{
	LogHandler::get_instance().logging_info(StringFormat("%s, \033[33mMT002\033[m message receive, success, sever received %d bytes message from client.\nReceived JSON : %s"
		, ip, receive_byte, receive_json.dump(4)));
}

void MT002Receiver::show_mt002_data()
{
	//LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Transport Type(0: unicast 1: multicast, 2:bloadcast) : %d", this->m_mt002_data.get_trans_type()));
	//LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Dist Client ID : %s", this->m_mt002_data.get_dst_client_id()));
	//LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Source Client ID : %s", this->m_mt002_data.get_src_client_id()));
	//LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Dist Chatroom ID : %s", this->m_mt002_data.get_dst_chatroom_id()));
	//LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Data Type(0: message 1: image 2: video 3: contact_details) : %d", this->m_mt002_data.get_data_type()));
	//LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Main Data : %s", this->m_mt002_data.get_main_data()));
}

std::unique_ptr<MT002Receiver::MT002Data> MT002Receiver::create_mt002data_instance()
{
	return std::unique_ptr<MT002Data>();
}

std::unique_ptr<MT002Receiver::MT002Data> MT002Receiver::create_mt002data_instance(const int& trans_type, const std::string& d_id, const std::string& s_id, const std::string& d_chatroom_id, const int& data_type, const std::string& main_content)
{
	std::unique_ptr<MT002Receiver::MT002Data> new_mt002;
	new_mt002 = std::make_unique<MT002Data>(trans_type, d_id, s_id, d_chatroom_id, data_type, main_content);
	return new_mt002;
}

std::tuple<int, std::string, std::string, std::string, int, std::string> MT002Receiver::data_extraction_(const json& receive_json)
{
	int trans_type = receive_json["trans_type"];
	std::string dst_client_id = receive_json["dst_client_id"];
	std::string src_client_id = receive_json["src_client_id"];
	std::string dst_chatroom_id = receive_json["dst_chatroom_id"];
	int data_type = receive_json["content_type"];
	std::string main_content = receive_json["main_content"];


	//int trans_type = this->extraction_trans_type(data);
	//std::string dst_client_id = extraction_dst_client_id(data);
	//std::string src_client_id = extraction_src_client_id(data);
	//std::string dst_chatroom_id = extraction_dst_chatroom_id(data);
	//int data_type = extraction_data_type(data);
	//std::string main_content = extraction_main_content(data);

	if (check_validation_contents(trans_type, dst_client_id, src_client_id, dst_chatroom_id, data_type, main_content) == false){
		return std::make_tuple(3, "", "", "", 4, "");
	}
	return std::tuple<int, std::string, std::string, std::string, int, std::string>(trans_type, dst_client_id, src_client_id, dst_chatroom_id, data_type, main_content);
}



