#include "MT002Receiver.h"


MT002Receiver::MT002Receiver()
{

}

MT002Receiver::~MT002Receiver()
{
}

/**********************************************************************************
 * @fn	  data_receive
 * @brief データ受信
 *
 * @param [in] int detection_num
 *             受信データ
 *
 * @return [out]
 *
 **********************************************************************************/
int MT002Receiver::data_receive(const ClientHandler& client_handler, int detection_num)
{
	try
	{
		//暗号化されたデータを受信
		struct Receiver::trans_data_info trans_di;
		char temp_recv_data[MAX_RECEIVE_MESSAGE_LEN];
		memset(temp_recv_data, '\0', sizeof(temp_recv_data));
		int receive_result = recv(client_handler.connected_client_list[detection_num].get_client_socket(), temp_recv_data, MAX_RECEIVE_MESSAGE_LEN, 0);


#ifdef DEBUG
		std::cout << ServerTime::get_current_time() << "[DEBUG]receive_result = " << receive_result << std::endl;
#endif // DEBUG


		//recv結果がSOCKET_ERRORの場合
		if (receive_result == SOCKET_ERROR) {
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSANOTINITIALISED(この関数を使用する前に、 WSAStartup 呼び出しが正常に行われる必要があります。)");
				break;
			case WSAENETDOWN:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAENETDOWN(ネットワーク サブシステムが失敗しました。)");
				break;
			case WSAEFAULT:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAEFAULT(buf パラメーターは、ユーザー・アドレス・スペースの有効な部分に完全には含まれていません。)");
				break;
			case WSAENOTCONN:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAENOTCONN(ソケットは接続されていません。)");
				break;
			case WSAEINTR:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAEINTR((ブロッキング) 呼び出しは WSACancelBlockingCall を介して取り消されました。)");
				break;
			case WSAEINPROGRESS:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAEINPROGRESS(ブロックしている Windows ソケット 1.1 呼び出しが進行中であるか、サービス プロバイダーがコールバック関数を処理しています。)");
				break;
			case WSAENETRESET:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAENETRESET(接続指向ソケットの場合、このエラーは、操作の進行中に障害を検出した キープアライブ アクティビティが原因で接続が切断されたことを示します。 データグラム ソケットに関して、このエラーは有効期限が切れたことを示します。)");
				break;
			case WSAENOTSOCK:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAENOTSOCK(記述子はソケットではありません。)");
				break;
			case WSAEOPNOTSUPP:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAEOPNOTSUPP(MSG_OOB指定されましたが、ソケットは、型SOCK_STREAM、OOB データがこのソケットに関連付けられている通信ドメインでサポートされていない、またはソケットが一方向であり、送信操作のみをサポートするなど、ストリーム スタイルではありません。)");
				break;
			case WSAESHUTDOWN:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAESHUTDOWN(ソケットがシャットダウンされました。SD_RECEIVEまたはSD_BOTHに設定された方法でシャットダウンが呼び出された後、ソケットでを受信することはできません。)");
				break;
			case WSAEWOULDBLOCK:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAEWOULDBLOCK(ソケットは非ブロッキングとしてマークされ、受信操作はブロックされます。)");
				break;
			case WSAEMSGSIZE:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAEMSGSIZE(メッセージは大きすぎて指定されたバッファーに収まらず、切り捨てられました。)");
				break;
			case WSAEINVAL:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAEINVAL(ソケットが bind でバインドされていないか、不明なフラグが指定されたか、SO_OOBINLINEが有効になっているソケットに対して指定されたMSG_OOB、または (バイト ストリーム ソケットの場合のみ) len が 0 または負の値でした。)");
				break;
			case WSAECONNABORTED:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAECONNABORTED(	仮想回線はタイムアウトまたはその他の障害のために切断されました。 ソケットが使用できないため、アプリケーションはソケットを閉じる必要があります。)");
				break;
			case WSAETIMEDOUT:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAETIMEDOUT(ネットワーク障害が発生したか、ピア システムが応答できなかったため、接続は切断されました。)");
				break;
			case WSAECONNRESET:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAECONNRESET(強制終了または中止になる閉じる操作を実行するリモート側によって仮想回線がリセットされました。 ソケットが使用できないため、アプリケーションはソケットを閉じる必要があります。 UDP データグラム ソケットでは、このエラーは、以前の送信操作で ICMP 'ポートに到達できません' というメッセージが発生したことを示します。)");
				break;
			default:
				THROW_APP_EXCEPTION("クライアントからのメッセージ受信時に何らかのエラーが発生しました。");
				break;
			}

		}
		//接続が閉じられた際の処理
		else if (receive_result == 0) {
			return receive_result;
		}
		//データ受信が正常に実施された際の処理
		else
		{

			trans_di.src_recv_data = std::string(temp_recv_data);

			print_src_recv_data(trans_di.src_recv_data, client_handler, detection_num, receive_result);

			//データの復号を実施する(未実装)


			//データタイプの抽出
			extraction_data_type(trans_di.src_recv_data);
		}
		return receive_result;

	}
	//catch (const std::exception& ex)
	catch (const exception::ExceptionBase& ex)
	{
#ifdef DEBUG
		//std::string temp_str = std::string(ex.what());
		//std::cout << temp_str << std::endl;
#endif // DEBUG


		//std::cerr << StringFormat("[ERROR] %s", ex.what()) << std::endl;
		std::cerr << StringFormat("%s[ERROR]%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
		return SOCKET_ERROR;

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
			//throw std::invalid_argument(StringFormat("The value of Trans type is invalid.(trans_type_int contains : %d)", trans_type_int));
		}
		m_mt002_data.set_trans_type(trans_type_int);
	}
	
	catch(const exception::ExceptionBase& ex)
	{
		std::cout << StringFormat("%s\033[31m[ERORR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
		return false;
	}
	
	return true;
}


bool MT002Receiver::extraction_dst_client_id(const std::string& data)
{
	try
	{
		//int start_pos_split = 4, end_pos_split = 36;
		if (CLIENT_DST_ID_START_POS > data.size() || (CLIENT_DST_ID_START_POS+ CLIENT_DST_ID_BYTE_SIZE) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}

		std::string dst_client_id = data.substr(CLIENT_DST_ID_START_POS, CLIENT_DST_ID_BYTE_SIZE);
		m_mt002_data.set_dst_client_id(dst_client_id);

	}
	catch(const exception::ExceptionBase& ex)
	{
		std::cout << StringFormat("%s\033[31m[ERORR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
		return false;
	}
	return true;
}

bool MT002Receiver::extraction_src_clinet_id(const std::string& data)
{
	try
	{
		//int start_pos_split = 40, end_pos_split = 36;
		if (CLIENT_SRC_ID_START_POS > data.size() || (CLIENT_SRC_ID_START_POS + CLIENT_SRC_ID_BYTE_SIZE) > data.size()) {
			THROW_APP_EXCEPTION(StringFormat("The location to be split exceeds the data size."));
		}

		std::string src_client_id = data.substr(CLIENT_SRC_ID_START_POS, CLIENT_SRC_ID_BYTE_SIZE);
		m_mt002_data.set_src_client_id(src_client_id);
	}
	catch(const exception::ExceptionBase& ex)
	{
		std::cout << StringFormat("%s\033[31m[ERORR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
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
		std::cout << StringFormat("%s\033[31m[ERORR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
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
		std::cout << StringFormat("%s\033[31m[ERORR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
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
		std::cout << StringFormat("%s\033[31m[ERORR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
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
void MT002Receiver::print_src_recv_data(const std::string& data, const ClientHandler& client_handler, int& detection_num, int& receive_result)
{
	std::cout << ServerTime::get_current_time() << "[RECEIVE]" << receive_result<< "byte message was received from client list["  << detection_num << "]"
		<< "(IP address：" << client_handler.connected_client_list[detection_num].get_client_ipv4_addr() << std::endl;

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
	std::cout << StringFormat("%s[DEBUG]Transport Type(0: unicast 1: multicast, 2:bloadcast) : %d", ServerTime::get_current_time(), this->m_mt002_data.get_trans_type()) << std::endl;
	std::cout << StringFormat("%s[DEBUG]Dist Client ID : %s", ServerTime::get_current_time(), this->m_mt002_data.get_dst_client_id()) << std::endl;
	std::cout << StringFormat("%s[DEBUG]Source Client ID : %s", ServerTime::get_current_time(), this->m_mt002_data.get_src_client_id()) << std::endl;
	std::cout << StringFormat("%s[DEBUG]Dist Chatroom ID : %s", ServerTime::get_current_time(), this->m_mt002_data.get_dst_chatroom_id()) << std::endl;
	std::cout << StringFormat("%s[DEBUG]Data Type(0: message 1: image 2: video 3: contact_details) : %d", ServerTime::get_current_time(), this->m_mt002_data.get_data_type()) << std::endl;
	std::cout << StringFormat("%s[DEBUG]Main Data : %s", ServerTime::get_current_time(), this->m_mt002_data.get_main_data()) << std::endl;
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

MT002Receiver::MT002Data::MT002Data()
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

const int MT002Receiver::get_trans_type()
{
	return this->m_mt002_data.get_trans_type();
}

const std::string& MT002Receiver::get_dst_chatroom_id()
{
	return this->m_mt002_data.get_dst_chatroom_id();
}



