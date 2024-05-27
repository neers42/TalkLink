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
bool MT002Receiver::extraction_trans_type(const std::string& data)
{
	try
	{
		int start_pos_split = 3, end_pos_split = 1;
		if (start_pos_split > data.size() || (start_pos_split + end_pos_split) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}
		std::string trans_type_string = data.substr(start_pos_split, end_pos_split);
		int trans_type_int = std::stoi(trans_type_string);
		if (trans_type_int < 0 || trans_type_int > 2) {
			THROW_APP_EXCEPTION(StringFormat("The value of Trans type is invalid.(\033[31mtrans_type_int contains : %d\033[m)", trans_type_int));
		}
		m_mt002_data.set_trans_type(trans_type_int);
	}
	
	catch(const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return false;
	}
	
	return true;
}


bool MT002Receiver::extraction_dst_client_id(const std::string& data)
{
	try
	{
		if (CLIENT_DST_ID_START_POS > data.size() || (CLIENT_DST_ID_START_POS+ CLIENT_DST_ID_BYTE_SIZE) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}

		std::string dst_client_id = data.substr(CLIENT_DST_ID_START_POS, CLIENT_DST_ID_BYTE_SIZE);
		m_mt002_data.set_dst_client_id(dst_client_id);

	}
	catch(const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return false;
	}
	return true;
}

bool MT002Receiver::extraction_src_clinet_id(const std::string& data)
{
	try
	{
		if (CLIENT_SRC_ID_START_POS > data.size() || (CLIENT_SRC_ID_START_POS + CLIENT_SRC_ID_BYTE_SIZE) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}

		std::string src_client_id = data.substr(CLIENT_SRC_ID_START_POS, CLIENT_SRC_ID_BYTE_SIZE);
		m_mt002_data.set_src_client_id(src_client_id);
	}
	catch(const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
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
bool MT002Receiver::extraction_dst_chatroom_id(const std::string& data)
{
	try
	{
		//int start_pos_split = 76, end_pos_split = 36;
		if (CHATROOM_ID_START_POS > data.size() || (CHATROOM_ID_START_POS + CHATROOM_ID_BYTE_SIZE) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}

		std::string chatroom_id = data.substr(CHATROOM_ID_START_POS, CHATROOM_ID_BYTE_SIZE);
		m_mt002_data.set_dst_chatroom_id(chatroom_id);

	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return false;
	}
		return true;
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
bool MT002Receiver::extraction_data_type(const std::string& data)
{
	try
	{
		//int start_pos_split = 112, end_pos_split = 1;
		if (DATA_TYPE_START_POS > data.size() || (DATA_TYPE_START_POS + DATA_TYPE_BYTE_SIZE) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}

		std::string data_type_string = data.substr(DATA_TYPE_START_POS, DATA_TYPE_BYTE_SIZE);
		int data_type_int = std::stoi(data_type_string);
		if (data_type_int < 0 || data_type_int > 3) {
			THROW_APP_EXCEPTION(StringFormat("The value of Data type is invalid.(\033[31mdata_type_int contains : %d\033[m", data_type_int));
		}
		m_mt002_data.set_data_type(data_type_int);

	}
	catch(const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return false;
	}

	return true;
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
bool MT002Receiver::extraction_main_data(const std::string& data)
{
	try
	{
		//int start_pos_split = 113;
		if (MAIN_DATA_START_POS > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}

		std::string main_data = data.substr(MAIN_DATA_START_POS);
		m_mt002_data.set_main_data(main_data);
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
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
	LogHandler::logging_info(LogHandler::LogType::RECEIVE, std::cout, StringFormat("%d byte message was received from client list[%d] (IP address：%s)", receive_result, detection_num, dst_ipv4_addr));
}


bool MT002Receiver::data_extraction(const std::string& data)
{

	bool trans_type_ex_res = this->extraction_trans_type(data);
	bool dst_client_id_res = this->extraction_dst_client_id(data);
	bool src_client_id_res = this->extraction_src_clinet_id(data);
	bool chatroom_id_ex_res = this->extraction_dst_chatroom_id(data);
	bool data_type_ex_res = this->extraction_data_type(data);
	bool main_data_ex_res = this->extraction_main_data(data);

	if (trans_type_ex_res == false || data_type_ex_res == false
		|| dst_client_id_res == false || src_client_id_res == false
		|| chatroom_id_ex_res == false || main_data_ex_res == false) {
		return false;
	}
#ifdef DEBUG_MT002_CONTAIN
	show_mt002_data();
#endif // DEBUG

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
, m_dst_ip_address{""}, m_src_ip_address{""}, m_dst_client_id{""}, m_src_client_id{""}, m_dst_chatroom_id{""}, m_main_data{""}
{

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


void MT002Receiver::MT002Data::set_dst_client_id(const std::string& dst_client_id)
{
	this->m_dst_client_id = dst_client_id;
}

void MT002Receiver::MT002Data::set_src_client_id(const std::string& src_client_id)
{
	this->m_src_client_id = src_client_id;
}

void MT002Receiver::MT002Data::set_dst_chatroom_id(const std::string& dst_chatroom_id)
{
	this->m_dst_chatroom_id = dst_chatroom_id;
}

void MT002Receiver::MT002Data::set_main_data(const std::string& main_data)
{
	this->m_main_data = main_data;
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


std::string& MT002Receiver::MT002Data::get_dst_client_id()
{
	return this->m_dst_client_id;
}

std::string& MT002Receiver::MT002Data::get_src_client_id()
{
	return this->m_src_client_id;
}

std::string& MT002Receiver::MT002Data::get_dst_chatroom_id()
{
	return this->m_dst_chatroom_id;
}

std::string& MT002Receiver::MT002Data::get_main_data()
{
	return this->m_main_data;
}

const MT002Receiver::MT002Data& MT002Receiver::get_mt002_data()
{
	return this->m_mt002_data;
}


const std::string& MT002Receiver::get_dst_id()
{
	return this->m_mt002_data.get_dst_client_id();

}

const std::string& MT002Receiver::get_src_id()
{
	return this->m_mt002_data.get_src_client_id();
}

const int MT002Receiver::get_trans_type()
{
	return this->m_mt002_data.get_trans_type();
}

const std::string& MT002Receiver::get_dst_chatroom_id()
{
	return this->m_mt002_data.get_dst_chatroom_id();
}

void MT002Receiver::show_receive_info(const std::string& ipv4_addr, int& detect_num, int& receive_result)
{
	LogHandler::logging_info(LogHandler::LogType::RECEIVE, std::cout, StringFormat("\033[33mMT002\033[m received from client list [%d] (CLIENT(IP:%s) ⇒ SERVER). (BYTE size of the message: %d)", detect_num, ipv4_addr, receive_result));
}

void MT002Receiver::show_mt002_data()
{
	LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Transport Type(0: unicast 1: multicast, 2:bloadcast) : %d", this->m_mt002_data.get_trans_type()));
	LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Dist Client ID : %s", this->m_mt002_data.get_dst_client_id()));
	LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Source Client ID : %s", this->m_mt002_data.get_src_client_id()));
	LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Dist Chatroom ID : %s", this->m_mt002_data.get_dst_chatroom_id()));
	LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Data Type(0: message 1: image 2: video 3: contact_details) : %d", this->m_mt002_data.get_data_type()));
	LogHandler::logging_info(LogHandler::LogType::DEBUG_INFO, std::cout, StringFormat("Main Data : %s", this->m_mt002_data.get_main_data()));
}



