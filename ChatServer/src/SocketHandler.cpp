#include "SocketHandler.h"

//リッスンソケット生成用のパラメータ
//struct addrinfo* param, *result;

//WSADATA wsa_data;

//fd_set fds, fd_read;

//timeval timeout;

//リッスンソケット
//SOCKET listen_socket;

//接続中のクライアントソケットリスト
//std::vector<SOCKET>* client_socket_list;


//デフォルトコンストラクタ
SocketHandler::SocketHandler()
{
	ZeroMemory(&param, sizeof(param));

	//wsa_dataの初期化
	int init_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);

	//ファイルディスクリプタキャッチ用のfdsを初期化
	FD_ZERO(&fds);
	FD_ZERO(&fd_read);

	//select関数のタイムアウト初期設定(tv_sec=秒の指定　tv_usec=マイクロ秒の指定)
	//デフォルトコンストラクタでは500マイクロ秒 * 1000　⇒ 500ミリ秒に設定
	timeout.tv_sec = 0;
	timeout.tv_usec = 500 * 1000;

	//リッスンソケットの初期化
	listen_socket = INVALID_SOCKET;

	//クライアントのリスト初期化
	client_socket_list->reserve(10000);
	client_socket_list->push_back(INVALID_SOCKET);


}

//コンストラクタ
SocketHandler::SocketHandler(int tv_sec, int tv_usec)
{
	ZeroMemory(&param, sizeof(param));
	//wsa_dataの初期化
	int init_wsa_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);

	if (!init_wsa_result) {
		switch(init_wsa_result) {
			case WSAVERNOTSUPPORTED:
				throw "ネットワークサブシステムがネットワークへの接続を準備出来ていない";
				printf("WSAVERNOTSUPPORTED\n");
				break;
			case WSAEINPROGRESS:
				printf("WSAEINPROGRESS\n");
				break;
			case WSAEPROCLIM:
				printf("WSAEPROCLIM\n");
				break;
			case WSAEFAULT:
				printf("WSAEFAULT\n");
				break;
		}
	}

	//ファイルディスクリプタキャッチ用のfdsを初期化
	FD_ZERO(&fds);
	FD_ZERO(&fd_read);

	//select関数のタイムアウト初期設定(tv_sec=秒の指定　tv_usec=マイクロ秒の指定)
	timeout.tv_sec = tv_sec;
	timeout.tv_usec = tv_usec;
	
	//リッスンソケットの初期化
	listen_socket = INVALID_SOCKET;

	//クライアントのリスト初期化
	client_socket_list->reserve(10000);
	client_socket_list->push_back(INVALID_SOCKET);

}

//デストラクタ
SocketHandler::~SocketHandler()
{
	WSACleanup();
}

//設定値の読み込み
void SocketHandler::load_socket_setting_ini_file(const std::string& file_pass)
{

	//iniファイルの読み込み
	ini::IniFile socket_setting_ini;
	socket_setting_ini.load(file_pass);
	
	//iniファイルの各セクションからキーを読み込む
	//アドレスファミリの設定(IPv4 or IPv6)
	std::string temp_family = socket_setting_ini["SocketSetting"]["family"].as<std::string>();

	//通信プロトコル
	if (temp_family.compare("AF_INET")) {
		param.ai_family = AF_INET;
		std::cout << "param : ai_familyはAF_INET(アドレスファミリはIPv4に設定されました)" << std::endl;
	}
	//通信プロトコルをIPv6に設定する場合
	else if (temp_family.compare("AF_INET6")) {
		std::cout << "param : ai_familyはAF_INET6(アドレスファミリはIPv6に設定されました)" << std::endl;
		param.ai_family = AF_INET6;
	}
	//通信プロトコルがAF_UNSPECに設定された場合(IPv4とIPv6に対応)
	//通信プロトコルがどちらにも設定していない場合及び読み取れなかった場合(AF_UNSPECに設定)
	else {
		std::cout << "param : ai_familyはAF_UNSPEC(アドレスファミリは設定されませんでした)" << std::endl;
		param.ai_family = AF_UNSPEC;
	}


	//ソケット型の設定
	std::string temp_socket_type = socket_setting_ini["SocketSetting"]["socktype"].as<std::string>();
	if (temp_socket_type.compare("SOCK_STREAM")) {
		param.ai_socktype = SOCK_STREAM;
		std::cout << "param : ai_socktypeはSOCK_STREAM(TCPプロトコルを使用)に設定されました" << std::endl;
	}
	else if(temp_socket_type.compare("SOCK_DGRAM")) {
		param.ai_socktype = SOCK_DGRAM;
		std::cout << "param : ai_socktypeはSOCK_DGRAM(UDPプロトコルを使用)に設定されました" << std::endl;
	}
	else if (temp_socket_type.compare("SOCK_RDM")) {
		param.ai_socktype = SOCK_RDM;
		std::cout << "param : ai_socktypeはSOCK_DGRAM(UDPプロトコルを使用)に設定されました" << std::endl;
	}
	else {

	}

	//プロトコルの設定
	std::string temp_protocol = socket_setting_ini["SocketSetting"]["protocol"].as<std::string>();

	//プロトコルをTCPに設定する場合
	if (temp_protocol.compare("IPPROTO_TCP") && ((param.ai_family == AF_INET || param.ai_family == AF_INET6)&& param.ai_socktype == SOCK_STREAM)) {
		param.ai_protocol = IPPROTO_TCP;
		std::cout << "param : ai_protocolはIPPROTO_TCP(TCPプロトコルを使用)に設定されました" << std::endl;
	}

	//プロトコルをUDPに設定する場合
	else if (temp_protocol.compare("IPPROTO_UDP") && ((param.ai_family == AF_INET || param.ai_family == AF_INET6)&& param.ai_socktype == SOCK_DGRAM)) {
		param.ai_protocol = IPPROTO_UDP;
		std::cout << "param : ai_protocolはIPPROTO_TCP(TCPプロトコルを使用)に設定されました" << std::endl;
	}
	//プロトコルをPGMに設定する場合
	else if (temp_protocol.compare("IPPROTO_RM") && param.ai_family == AF_INET && param.ai_socktype == SOCK_RDM) {
		param.ai_protocol = IPPROTO_PGM;
	}
	else {

	}

	//フラッグの設定
	std::string temp_flag = socket_setting_ini["SocketSetting"]["flgas"].as<std::string>();
	

}

void SocketHandler::bind_listen_socket()
{
	
}

void SocketHandler::accept_client_socket()
{
}

void SocketHandler::push_client_socket_list(const SOCKET new_client_socket)
{

}

int SocketHandler::cal_connect_client_socket_num(const SOCKET serach_socket)
{
	return 0;
}

void SocketHandler::close_socket(SOCKET request_close_socket)
{

}

void SocketHandler::check_valid_socket(SOCKET request_socket)
{

}

SOCKET SocketHandler::genarated_listen_socket()
{


	return SOCKET();
}

int SocketHandler::monitor_socket()
{
	return 0;
}
