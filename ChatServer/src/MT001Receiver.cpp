#include "MT001Receiver.h"


MT001Receiver::MT001Receiver()
{

}

MT001Receiver::~MT001Receiver()
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
int MT001Receiver::data_receive(const ClientHandler& client_handler, int detection_num)
{
	try
	{
		//暗号化されたデータを受信
		struct Receiver::trans_data_info trans_di;
		char temp_recv_data[MAX_RECEIVE_MESSAGE_LEN];
		memset(temp_recv_data, '\0', sizeof(temp_recv_data));
		int receive_result = recv(client_handler.connected_client_list[detection_num].get_client_socket(), temp_recv_data, MAX_RECEIVE_MESSAGE_LEN, 0);

		//ERROR検証用←デバック完了したら以下を消す
		//receive_result = SOCKET_ERROR;

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
bool MT001Receiver::extraction_trans_type(const std::string& data)
{
	//TODO エラー処理を実装
	std::string trans_type_string = data.substr(3, 1);
	int trans_type_int = std::stoi(trans_type_string);
	mt001_data.set_trans_type(trans_type_int);
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
bool MT001Receiver::extraction_data_type(const std::string& data)
{
	//TODO：エラー処理を実装
	std::string data_type_string = data.substr(4, 1);
	int data_type_int = std::stoi(data_type_string);
	mt001_data.set_data_type(data_type_int);
	return true;
}


/**********************************************************************************
 * @fn	  extraction_dst_ip
 * @brief
 *
 * @param [in]
 *
 * @return [out]
 *
 **********************************************************************************/
bool MT001Receiver::extraction_dst_ip(const std::string& data)
{
	//TODO：エラー処理を実装
	std::string dst_ip = data.substr(5, 15);
	std::string corrected_dst_ip = remove_zero_padding(dst_ip);
	mt001_data.set_dst_ip_address(corrected_dst_ip);
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
bool MT001Receiver::extraction_src_ip(const std::string& data)
{
	//TODO：エラー処理を実装
	std::string src_ip = data.substr(20, 15);
	std::string corrected_src_ip = remove_zero_padding(src_ip);
	mt001_data.set_src_ip_address(corrected_src_ip);
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
bool MT001Receiver::extraction_dst_chatroom_id(const std::string& data)
{
	//TODO：エラー処理を実装
	std::string chatroom_id = data.substr(35, 19);
	mt001_data.set_dst_chatroom_id(chatroom_id);
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
bool MT001Receiver::extraction_main_data(const std::string& data)
{
	//エラー処理を実装する
	std::string main_data = data.substr(54);
	mt001_data.set_main_data(main_data);
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
bool MT001Receiver::store_receive_data()
{
	return false;
}

//受信した暗号化されたメッセージを表示する。
void MT001Receiver::print_src_recv_data(const std::string& data, const ClientHandler& client_handler, int& detection_num, int& receive_result)
{
	std::cout << ServerTime::get_current_time() << "[RECEIVE]" << receive_result<< "byte message was received from client list["  << detection_num << "]"
		<< "(IP address：" << client_handler.connected_client_list[detection_num].get_client_ipv4_addr() << std::endl;

}


bool MT001Receiver::data_extraction(const std::string& data)
{
	this->extraction_trans_type(data);
	this->extraction_data_type(data);
	this->extraction_src_ip(data);
	this->extraction_dst_ip(data);
	this->extraction_dst_chatroom_id(data);
	this->extraction_main_data(data);
#ifdef DEBUG
	std::cout << StringFormat("%s[DEBUG]Trans Type(0: unicast 1: multicast, 2:bloadcast) : %d", ServerTime::get_current_time(), this->mt001_data.get_trans_type()) << std::endl;
	std::cout << StringFormat("%s[DEBUG]Data_type(0: message 1: image 2: video 3: contact_details) : %d", ServerTime::get_current_time(), this->mt001_data.get_data_type()) << std::endl;
	std::cout << StringFormat("%s[DEBUG]Dst IP Adress : %s", ServerTime::get_current_time(), this->mt001_data.get_dst_ip()) << std::endl;
	std::cout << StringFormat("%s[DEBUG]Src IP Adress : %s", ServerTime::get_current_time(), this->mt001_data.get_src_ip()) << std::endl;
	std::cout << StringFormat("%s[DEBUG]Dst Chatroom ID : %s", ServerTime::get_current_time(), this->mt001_data.get_dst_chatroom_id()) << std::endl;
	std::cout << StringFormat("%s[DEBUG]Main Data : %s", ServerTime::get_current_time(), this->mt001_data.get_main_data()) << std::endl;
#endif // DEBUG

	return true;
}

std::string MT001Receiver::remove_zero_padding(std::string& ip_address)
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

MT001Receiver::MT001Data::MT001Data()
{

}

MT001Receiver::MT001Data::~MT001Data()
{
}

void MT001Receiver::MT001Data::set_trans_type(const int& trans_type)
{
	switch (trans_type)
	{
	case 0:
		this->trans_type = trans_type::unicast;
		break;

	case 1:
		this->trans_type = trans_type::multicast;
		break;

	case 2:
		this->trans_type = trans_type::bloadcast;
		break;

	default:
		break;
	}
}

void MT001Receiver::MT001Data::set_data_type(const int& data_type)
{
	switch (data_type)
	{
	case 0:
		this->data_type = data_type::message;
		break;

	case 1:
		this->data_type = data_type::image;
		break;
	case 2:
		this->data_type = data_type::video;
		break;
	case 3:
		this->data_type = data_type::contact_details;
		break;
	default:
		break;
	}
}

void MT001Receiver::MT001Data::set_dst_ip_address(const std::string& dst_ip)
{
	this->dst_ip_address = dst_ip;
}

void MT001Receiver::MT001Data::set_src_ip_address(const std::string& src_ip)
{
	this->src_ip_address = src_ip;
}

void MT001Receiver::MT001Data::set_dst_chatroom_id(const std::string& dst_chatroom_id)
{
	this->dst_chatroom_id = dst_chatroom_id;
}

void MT001Receiver::MT001Data::set_main_data(const std::string& main_data)
{
	this->main_data = main_data;
}
