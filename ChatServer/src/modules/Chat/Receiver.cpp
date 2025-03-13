#include "Receiver.h"


Receiver::Receiver()
{
}

Receiver::~Receiver()
{
}

Receiver::Message::Message() {

}

Receiver::Message::Message(std::string& s_id, std::string& d_id, std::string& data, std::string& time_stamp, std::string& id)
	: src_client_id{s_id}, dst_client_id{d_id}, main_content{data}, time_stamp{time_stamp}, message_id{id}
{
}


Receiver::Message::~Message() {

}


std::tuple<int32_t, json> Receiver::receive_data(const SOCKET& receive_socket, int client_num)
{
	char buffer[MAX_RECEIVE_MESSAGE_LEN] = { 0 };
	int bytes_received;
	json received_json;
	try
	{
		bytes_received = recv(receive_socket, buffer, sizeof(buffer), 0);
		std::string message_time_stamp = ServerTime::get_current_time();
		//recv結果がSOCKET_ERRORの場合
		if (bytes_received == SOCKET_ERROR) {
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
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAETIMEDOUT(ネットワーク障害が発生したか、ピアシステムが応答できなかったため、接続は切断されました。)");
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
		else if (bytes_received == 0) {
			return std::make_tuple(bytes_received, json());
		}
		//データ受信が正常に実施された際の処理
		else {
			std::vector<uint8_t> received_data(buffer, buffer + bytes_received);
			json received_json = json::from_msgpack(received_data);
			return std::make_tuple(bytes_received, received_json);
		}

	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::get_instance().logging_error(StringFormat("%s", ex.what_for_str()));
		return std::make_tuple(bytes_received, json());

	}
}
bool Receiver::data_extraction(const std::string& data)
{
	return false;
}
void Receiver::show_receive_info(const std::string& ipv4_addr, int& detection_num, int& receive_result)
{
	LogHandler::logging(LogHandler::LogType::RECEIVE, std::cout, StringFormat("%d byte message was received from client list[%d](IP Address : %s)", receive_result, detection_num, ipv4_addr));
}

void Receiver::show_receive_info(const json& receive_json, const std::string& ip, int receive_byte)
{
	LogHandler::get_instance().logging_info(StringFormat("%s, message receive, success, sever received %d bytes message from client.\nReceived JSON : %s"
	,ip, receive_byte, receive_json.dump(4)));
}

std::string Receiver::extraction_src_client_id(const std::string& data)
{
	return std::string();
}

std::string Receiver::extraction_dst_client_id(const std::string& data)
{
	return std::string();
}

std::string Receiver::extraction_main_content(const std::string& data)
{
	return std::string();
}

bool Receiver::check_validation_src_client_id(const std::string& s_id)
{
	return true;
}

bool Receiver::check_validation_dst_client_id(const std::string& d_id)
{
	return true;
}

bool Receiver::check_validation_main_content(const std::string& main_content)
{
	return true;
}




std::string Receiver::extraction_mt(const std::string& data)
{
	std::string mt = data.substr(MT_START_POS, MT_BYTE_SIZE);
	return mt;
}

std::string Receiver::extraction_trans_type(const std::string& data)
{
	std::string trans_type = data.substr(TRANS_TYPE_START_POS, TRANS_TYPE_BYTE_SIZE);
	return trans_type;
}

bool Receiver::check_validation(const std::string& data)
{
	if (data.size() < 4) {
		return false;
	}
	return true;
}

