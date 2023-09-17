#include "Receiver.h"

Receiver::Receiver()
{

}

Receiver::~Receiver()
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
int Receiver::extraction_data_type(const std::string& data)
{
	return 0;
}

__int16 Receiver::extraction_dst_ip_num(const std::string& data)
{
	return 0;
}

std::string Receiver::extraction_scr_ip(const std::string& data)
{
	return std::string();
}

std::string Receiver::extraction_dst_ip(const std::string& data)
{
	return std::string();
}

std::string Receiver::extraction_group_id(const std::string& data)
{
	return std::string();
}

std::vector<std::string> Receiver::extraction_dst_ip_list(const std::string& group_id)
{
	return std::vector<std::string>();
}

std::string Receiver::extraction_scr_payload(const std::string& data)
{
	return std::string();
}

bool Receiver::store_receive_data()
{
	return false;
}
