#include "SocketHandler.h"


/**********************************************************************************
 * @fn	  SocketHandlerコンストラクタ
 * @brief 設定ファイル読み込み(int)
 *
 * @param [in] なし
 * @return [out]なし
 *
 **********************************************************************************/
SocketHandler::SocketHandler()
{
	ZeroMemory(&this->param, sizeof(this->param));

	try
	{
		int init_wsa_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
		if (!init_wsa_result) {
			switch (init_wsa_result) {
			case WSASYSNOTREADY:
				THROW_APP_EXCEPTION("CASE:WSASYSNOTREADY(ネットワークサブシステムがネットワークへの接続を準備出来ていません)");
				break;

			case WSAVERNOTSUPPORTED:
				THROW_APP_EXCEPTION("CASE:WSAVERNOTSUPPORTED:(要求された Windows ソケットのサポートのバージョンは、この特定の Windows ソケットの実装では提供されません)");
				break;

			case WSAEINPROGRESS:
				THROW_APP_EXCEPTION("CASE:WSAEINPROGRESS(Windows ソケット 1.1 のブロック操作が進行中です)");
				break;

			case WSAEPROCLIM:
				THROW_APP_EXCEPTION("CASE:WSAEPROCLIM(Windows ソケットの実装でサポートされるタスクの数の制限に達しました)");
				break;

			case WSAEFAULT:
				THROW_APP_EXCEPTION("CASE:WSAEFAULT(lpWSAData パラメーターが有効なポインターではありません)");
				break;
			}
		}
		std::cout << "param_ai_flags = " << param.ai_flags << std::endl;
	}
	catch (const std::exception& ex)
	{
		std::cerr << StringFormat("[ERROR] %s", ex.what()) << std::endl;
	}

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
	client_socket_list.reserve(MAX_CONNECT_NUM);
	client_socket_list.push_back(INVALID_SOCKET);

	init_sockaddr_in.sin_family = AF_INET;
	init_sockaddr_in.sin_port = 50000;
	init_sockaddr_in.sin_addr.S_un.S_addr = INADDR_ANY;

	client_net_info init_net_info(INVALID_SOCKET, init_sockaddr_in);
	client_net_info_list.reserve(MAX_CONNECT_NUM);
	client_net_info_list.push_back(init_net_info);



}

/**********************************************************************************
 * @fn	  SocketHandlerコンストラクタ
 * @brief 設定ファイル読み込み(int)
 *
 * @param [in] なし
 * @return [out]なし
 *
 **********************************************************************************/
SocketHandler::SocketHandler(int tv_sec, int tv_usec)
{
	ZeroMemory(&param, sizeof(param));
	//wsa_dataの初期化
	try
	{
		int init_wsa_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
		if (!init_wsa_result) {
			switch (init_wsa_result) {
			case WSASYSNOTREADY:
				THROW_APP_EXCEPTION("CASE:WSASYSNOTREADY(ネットワークサブシステムがネットワークへの接続を準備出来ていません)");
				break;

			case WSAVERNOTSUPPORTED:
				THROW_APP_EXCEPTION("CASE:WSAVERNOTSUPPORTED:(要求された Windows ソケットのサポートのバージョンは、この特定の Windows ソケットの実装では提供されません)");
				break;

			case WSAEINPROGRESS:
				THROW_APP_EXCEPTION("CASE:WSAEINPROGRESS(Windows ソケット 1.1 のブロック操作が進行中です)");
				break;

			case WSAEPROCLIM:
				THROW_APP_EXCEPTION("CASE:WSAEPROCLIM(Windows ソケットの実装でサポートされるタスクの数の制限に達しました)");
				break;

			case WSAEFAULT:
				THROW_APP_EXCEPTION("CASE:WSAEFAULT(lpWSAData パラメーターが有効なポインターではありません)");
				break;
			}
		}
	}
	catch (const std::exception& ex)
	{
		std::cerr << StringFormat("[ERROR] %s", ex.what()) << std::endl;
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
	client_socket_list.reserve(10000);
	client_socket_list.push_back(INVALID_SOCKET);

	init_sockaddr_in.sin_family = AF_INET;
	init_sockaddr_in.sin_port = 50000;
	init_sockaddr_in.sin_addr.S_un.S_addr = INADDR_ANY;

	client_net_info init_net_info(INVALID_SOCKET, init_sockaddr_in);
	client_net_info_list.reserve(MAX_CONNECT_NUM);
	client_net_info_list.push_back(init_net_info);
}

//デストラクタ
SocketHandler::~SocketHandler()
{
	freeaddrinfo(result);
	closesocket(get_listen_socket());
	WSACleanup();
}



//設定値の読み込み
bool SocketHandler::load_socket_setting_ini_file(const std::string& file_pass)
{

	//iniファイルの読み込み
	ini::IniFile socket_setting_ini;
	try
	{
		socket_setting_ini.load(file_pass);
	}
	catch (const std::exception& ex)
	{
		THROW_APP_EXCEPTION("iniファイルの読み込みに失敗しました");
	}
	
	//フラッグの設定
	std::string temp_flag = socket_setting_ini["SocketSetting"]["flags"].as<std::string>();
	std::cout << temp_flag << std::endl;

	if (std::equal(temp_flag.begin(), temp_flag.end(), "AI_PASSIVE")) {
		param.ai_flags = AI_PASSIVE;
		std::cout << "param：ai_flagsはAI_PASSIVEに設定されました" << std::endl;
	}
	else if (std::equal(temp_flag.begin(), temp_flag.end(), "AI_CANONNAME")) {
		param.ai_flags = AI_CANONNAME;
		std::cout << "param : ai_flagsはAI_CANONNAMEに設定されました" << std::endl;
	}
	else if (std::equal(temp_flag.begin(), temp_flag.end(), "AI_NUMERICHOST")) {
		param.ai_flags = AI_NUMERICHOST;
		std::cout << "param : ai_flagsはAI_NUMERICHOSTに設定されました" << std::endl;
	}
	else if (std::equal(temp_flag.begin(), temp_flag.end(), "AI_ALL")) {
		param.ai_flags = AI_ALL;
		std::cout << "param : ai_flagsはAI_ALLに設定されました" << std::endl;
	}
	else if (std::equal(temp_flag.begin(), temp_flag.end(), "AI_ADDRCONFIG")) {
		param.ai_flags = AI_ADDRCONFIG;
		std::cout << "param : ai_flagsはAI_ADDRCONFIGに設定されました" << std::endl;
	}
	else if (std::equal(temp_flag.begin(), temp_flag.end(), "AI_V4MAPPED")) {
		param.ai_flags = AI_V4MAPPED;
		std::cout << "param : ai_flagsはAI_V4MAPPEDに設定されました" << std::endl;
	}
	else {
		return false;
	}

	//iniファイルの各セクションからキーを読み込む
	//アドレスファミリの設定(ipv4 or ipv6)
	std::string temp_family = socket_setting_ini["SocketSetting"]["family"].as<std::string>();

	//通信プロトコル
	if (std::equal(temp_family.begin(), temp_family.end(), "AF_INET") ){
		param.ai_family = AF_INET;
		std::cout << "param : ai_familyはAF_INET(アドレスファミリはipv4に設定されました)" << std::endl;
	}
	//通信プロトコルをipv6に設定する場合
	else if (std::equal(temp_family.begin(), temp_family.end(), "AF_INET6")) {
		param.ai_family = AF_INET6;
		std::cout << "param : ai_familyはAF_INET6(アドレスファミリはipv6に設定されました)" << std::endl;
	}
	//通信プロトコルがaf_unspecに設定された場合(ipv4とipv6に対応)
	//通信プロトコルがどちらにも設定していない場合及び読み取れなかった場合(af_unspecに設定)
	else if(std::equal(temp_family.begin(), temp_family.end(), "AF_UNSPEC")) {
		param.ai_family = AF_UNSPEC;
		std::cout << "param : ai_familyはAF_INET6(アドレスファミリはAF=UNSPECに設定されました)" << std::endl;
	}
	else {

	}


	//ソケット型の設定
	std::string temp_socket_type = socket_setting_ini["SocketSetting"]["socktype"].as<std::string>();
	if (std::equal(temp_socket_type.begin(), temp_socket_type.end(), "SOCK_STREAM")) {
		param.ai_socktype = SOCK_STREAM;
		std::cout << "param : ai_socktypeはSOCK_STREAM(TCPプロトコルを使用)に設定されました" << std::endl;
	}
	else if(std::equal(temp_socket_type.begin(), temp_socket_type.end(), "SOCK_DGRAM")) {
		param.ai_socktype = SOCK_DGRAM;
		std::cout << "param : ai_socktypeはsock_dgram(UDPプロトコルを使用)に設定されました" << std::endl;
	}
	else if (std::equal(temp_socket_type.begin(), temp_socket_type.end(), "SOCK_RDM")) {
		param.ai_socktype = SOCK_RDM;
		std::cout << "param : ai_socktypeはSOCK_RDM(RDMプロトコルを使用)に設定されました" << std::endl;
	}
	else{
		std::cout << "param : ai_socktypeは設定されませんでした" << std::endl;
		return false;
	}

	//プロトコルの設定
	std::string temp_protocol = socket_setting_ini["SocketSetting"]["protocol"].as<std::string>();
	//std::equal(temp_family.begin(), temp_family.end(), "AF_INET")
	//プロトコルをtcpに設定する場合
	if ((std::equal(temp_protocol.begin(), temp_protocol.end(), "IPPROTO_TCP")) && ((param.ai_family == AF_INET || param.ai_family == AF_INET6)&& param.ai_socktype == SOCK_STREAM)) {
		param.ai_protocol = IPPROTO_TCP;
		std::cout << "param : ai_protocolはIPPROTO_TCP(tcpプロトコルを使用)に設定されました" << std::endl;
	}

	//プロトコルをudpに設定する場合
	else if ((std::equal(temp_protocol.begin(), temp_protocol.end(), "IPPROTO_UDP")) && ((param.ai_family == AF_INET || param.ai_family == AF_INET6)&& param.ai_socktype == SOCK_DGRAM)) {
		param.ai_protocol = IPPROTO_UDP;
		std::cout << "param : ai_protocolはipproto_tcp(tcpプロトコルを使用)に設定されました" << std::endl;
	}
	//プロトコルをpgmに設定する場合
	else if ((std::equal(temp_protocol.begin(), temp_protocol.end(), "IPPROTO_PGM")) && param.ai_family == AF_INET && param.ai_socktype == SOCK_RDM) {
		param.ai_protocol = IPPROTO_PGM;
		std::cout << "param : ai_protocolはipproto_pgm(pgmプロトコルを使用)に設定されました" << std::endl;
	}
	else {
		return false;
	}

	//ポートの設定
	try
	{
		int temp_port = socket_setting_ini["SocketSetting"]["port"].as<int>();

		if (temp_port < 0 || temp_port > 65535) {
			THROW_APP_EXCEPTION("aaa");
		}
		port = temp_port;
	}
	catch (const std::exception& ex)
	{

		std::cerr << StringFormat("[ERROR] %s", ex.what()) << std::endl;
		return false;
	}
	return true;
}

std::string SocketHandler::get_client_ip(int client_num)
{
	
	return std::string();
}

bool SocketHandler::bind_listen_socket()
{
	try
	{
		int bind_result = bind(listen_socket, result->ai_addr, (int)result->ai_addrlen);
		if (bind_result == SOCKET_ERROR) {
			std::stringstream socket_error_msg;
			socket_error_msg << "ソケットバインド中にエラーが発生しました。WSAGetLastError Code : " << std::to_string(WSAGetLastError());
			THROW_APP_EXCEPTION(socket_error_msg.str());
			
		}
	}
	catch (const std::exception& ex)
	{
		std::cerr << StringFormat("[ERROR] %s %s", ex.what()) << std::endl;
		return false;
	}
	std::cout << "ソケットバインドに成功しました。" << std::endl;

	return true;
	
}

/**********************************************************************************
 * @fn	  accept_client_socket
 * 接続要求があった際のクライアントソケットに対してaccept処理を実行する
 * @brief accept処理
 *
 * @param [in] なし
 * @return 処理結果(true:成功/false:エラー)
 *
 * @detail 
 **********************************************************************************/
bool SocketHandler::accept_client_socket()
{
	//接続リストの空き状況を確認する
	int check_n_invalid_socket_result = check_next_invalid_socket();

	if (check_n_invalid_socket_result != -1) {
		try
		{
			client_net_info_list[check_n_invalid_socket_result].client_socket = 
				accept(listen_socket, 
					(struct sockaddr*)&client_net_info_list[check_n_invalid_socket_result].client_sockaadr_in, 
					&client_net_info_list[check_n_invalid_socket_result].socket_addr_in_length);
			
			//acceptしたソケットがINVALID_SOCKETである場合
			if (client_socket_list[check_n_invalid_socket_result] == INVALID_SOCKET) {
				

			}
			else {
				//acceptが成功した場合
				std::cout << "[INFO] IP : " << std::endl;
				return true;
			}
		}
		catch (const std::exception& ex)
		{
			std::cerr << StringFormat("[ERROR] %s", ex.what()) << std::endl;
		}
		

	}
	
	return false;
}


int SocketHandler::check_next_invalid_socket()
{
	for (int ninvalid_socket = 0; ninvalid_socket < MAX_CONNECT_NUM ; ninvalid_socket++) {
		if (client_socket_list[ninvalid_socket] == INVALID_SOCKET)
			return ninvalid_socket;
	}
	return -1;
}

void SocketHandler::push_client_socket_list(const SOCKET new_client_socket)
{
	
}

int SocketHandler::search_connect_client_socket_num(const SOCKET serach_socket)
{
	
	return 0;
}

void SocketHandler::close_socket(SOCKET request_close_socket)
{
	
}

bool SocketHandler::check_valid_socket(SOCKET request_socket)
{
	if (request_socket == INVALID_SOCKET)
		return false;
	return true;
}

bool SocketHandler::generate_listen_socket()
{
	//パラメータ設定
	load_socket_setting_ini_file(INIT_FILE_PASS);

	//名前解決
	try
	{
		int get_addr_result = getaddrinfo(NULL, std::to_string(port).c_str(), &param, &result);
		if (get_addr_result != 0) {
			switch (get_addr_result)
			{
			case EAI_AGAIN:
				THROW_APP_EXCEPTION("名前解決の一時的なエラーが発生しました");
				return false;
				break;

			case EAI_BADFLAGS:
				THROW_APP_EXCEPTION("pHints パラメーターのai_flags メンバーに無効な値が指定されました");
				return false;
				break;
			
			case EAI_FAIL:
				THROW_APP_EXCEPTION("名前解決で回復不可能なエラーが発生しました");
				return false;
				break;

			case EAI_FAMILY:
				THROW_APP_EXCEPTION("pHints パラメーターのai_familyメンバーはサポートされていません");
				return false;
				break;

			case EAI_MEMORY:
				THROW_APP_EXCEPTION("メモリ割り当てエラーが発生しました");
				return false;
				break;

			case EAI_NONAME:
				THROW_APP_EXCEPTION("指定されたパラメーターの名前が解決されないか、 pNodeName パラメーターと pServiceName パラメーターが指定されませんでした");
				return false;

			case EAI_SERVICE:
				THROW_APP_EXCEPTION("pServiceName パラメーターは、pHints パラメーターの指定したai_socktype メンバーではサポートされていません");
				return false;

			case EAI_SOCKTYPE:
				THROW_APP_EXCEPTION("pHints パラメーターのai_socktype メンバーはサポートされていません");
				return false;
				break;

			default:
				break;
			}
		}

		//リッスンソケットの生成
		listen_socket = socket(param.ai_family, param.ai_socktype, param.ai_protocol);
		if (listen_socket == INVALID_SOCKET) {
			THROW_APP_EXCEPTION("生成されたLISTENソケットは有効ではありません");
		}
	}
	catch (const std::exception& ex)
	{
		std::cerr << StringFormat("[ERROR] %s", ex.what()) << std::endl;
		freeaddrinfo(result);
		WSACleanup();
	}
	std::cout << "ソケットが正常に生成されました" << std::endl;

	bind_listen_socket();

	return true;
}

int SocketHandler::monitor_socket()
{
	return 0;
}
