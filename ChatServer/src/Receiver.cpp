#include "Receiver.h"


Receiver::Receiver()
{
}

Receiver::~Receiver()
{
}

std::tuple<int32_t, std::string> Receiver::receive_data(const ClientHandler& ch, int client_list_num)
{

	try
	{
		//暗号化されたデータを受信
		char temp_recv_data[MAX_RECEIVE_MESSAGE_LEN];
		memset(temp_recv_data, '\0', sizeof(temp_recv_data));
		int receive_result = recv(ch.connected_client_list[client_list_num].get_client_socket(), temp_recv_data, MAX_RECEIVE_MESSAGE_LEN, 0);

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
		else if (receive_result == 0) {
			return std::make_tuple(receive_result, "");
		}
		//データ受信が正常に実施された際の処理
		else
		{
			//trans_di.src_recv_data = std::string(temp_recv_data);
			show_receive_info(temp_recv_data, ch, client_list_num, receive_result);
		}
		return std::make_tuple(receive_result, temp_recv_data);

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
		return std::make_tuple(SOCKET_ERROR, "");

	}

}
void Receiver::show_receive_info(const std::string& data, const ClientHandler& client_handler, int& detection_num, int& receive_result)
{
	
	std::cout << ServerTime::get_current_time() << "[RECEIVE]" << receive_result<< "byte message was received from client list["  << detection_num << "]"
		<< "(IP address：" << client_handler.connected_client_list[detection_num].get_client_ipv4_addr()<< ")" << std::endl;

#ifdef DEBUG
	//std::cout << ServerTime::get_current_time() << "[DEBUG]The message contains：" << data << std::endl;
#endif // DEBUG
}


std::string Receiver::extraction_mt(const std::string& data)
{
	std::string mt = data.substr(0, 3);
	return mt;
}

std::string Receiver::extraction_trans_type(const std::string& data)
{
	std::string trans_type = data.substr(53, 1);
	return trans_type;
}

