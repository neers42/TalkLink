#include "SocketHandler.h"


/**********************************************************************************
 * @fn	  SocketHandlerデフォルトコンストラクタ
 * @brief 設定ファイル読み込み(int)
 *
 * @param [in] なし
 * @return [out]なし
 *
 **********************************************************************************/
SocketHandler::SocketHandler()
{
	ZeroMemory(&param, sizeof(param));

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
	//デフォルトコンストラクタでは500マイクロ秒 * 1000　⇒ 500ミリ秒に設定
	timeout.tv_sec = 0;
	timeout.tv_usec = 500 * 1000;

	//リッスンソケットの初期化
	listen_socket = INVALID_SOCKET;

	//クライアントのリスト初期化
	client_socket_list.reserve(MAX_CONNECT_NUM);
	client_socket_list.push_back(INVALID_SOCKET);

	init_sockaddr_in.sin_family = AF_INET;
	init_sockaddr_in.sin_port = 0;
	init_sockaddr_in.sin_addr.S_un.S_addr = INADDR_ANY;

	client_net_info init_net_info(INVALID_SOCKET, init_sockaddr_in);
	client_net_info_list.reserve(MAX_CONNECT_NUM);
	for (int i = 0; i < MAX_CONNECT_NUM; i++) {
		client_net_info_list.push_back(init_net_info);

	}

}

/**********************************************************************************
 * @fn	  SocketHandlerコピーコンストラクタ
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
	std::cout << "[INFO] iniファイルの読み込みに成功しました。" << std::endl;
	
	//フラッグの設定
	std::string temp_flag = socket_setting_ini["SocketSetting"]["flags"].as<std::string>();
	

	if (std::equal(temp_flag.begin(), temp_flag.end(), "AI_PASSIVE")) {
		param.ai_flags = AI_PASSIVE;
		std::cout << "[INFO] param：ai_flagsはAI_PASSIVEに設定されました" << std::endl;
	}
	else if (std::equal(temp_flag.begin(), temp_flag.end(), "AI_CANONNAME")) {
		param.ai_flags = AI_CANONNAME;
		std::cout << "[INFO] param : ai_flagsはAI_CANONNAMEに設定されました" << std::endl;
	}
	else if (std::equal(temp_flag.begin(), temp_flag.end(), "AI_NUMERICHOST")) {
		param.ai_flags = AI_NUMERICHOST;
		std::cout << "[INFO] param : ai_flagsはAI_NUMERICHOSTに設定されました" << std::endl;
	}
	else if (std::equal(temp_flag.begin(), temp_flag.end(), "AI_ALL")) {
		param.ai_flags = AI_ALL;
		std::cout << "[INFO] param : ai_flagsはAI_ALLに設定されました" << std::endl;
	}
	else if (std::equal(temp_flag.begin(), temp_flag.end(), "AI_ADDRCONFIG")) {
		param.ai_flags = AI_ADDRCONFIG;
		std::cout << "[INFO] param : ai_flagsはAI_ADDRCONFIGに設定されました" << std::endl;
	}
	else if (std::equal(temp_flag.begin(), temp_flag.end(), "AI_V4MAPPED")) {
		param.ai_flags = AI_V4MAPPED;
		std::cout << "[INFO] param : ai_flagsはAI_V4MAPPEDに設定されました" << std::endl;
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
		std::cout << "[INFO] param : ai_familyはAF_INET(アドレスファミリはipv4に設定されました)" << std::endl;
	}
	//通信プロトコルをipv6に設定する場合
	else if (std::equal(temp_family.begin(), temp_family.end(), "AF_INET6")) {
		param.ai_family = AF_INET6;
		std::cout << "[INFO] param : ai_familyはAF_INET6(アドレスファミリはipv6に設定されました)" << std::endl;
	}
	//通信プロトコルがaf_unspecに設定された場合(ipv4とipv6に対応)
	//通信プロトコルがどちらにも設定していない場合及び読み取れなかった場合(af_unspecに設定)
	else if(std::equal(temp_family.begin(), temp_family.end(), "AF_UNSPEC")) {
		param.ai_family = AF_UNSPEC;
		std::cout << "[INFO] param : ai_familyはAF_UNSPEC(アドレスファミリはAF=UNSPECに設定されました)" << std::endl;
	}
	else {

	}


	//ソケット型の設定
	std::string temp_socket_type = socket_setting_ini["SocketSetting"]["socktype"].as<std::string>();
	if (std::equal(temp_socket_type.begin(), temp_socket_type.end(), "SOCK_STREAM")) {
		param.ai_socktype = SOCK_STREAM;
		std::cout << "[INFO] param : ai_socktypeはSOCK_STREAM(TCPプロトコルを使用)に設定されました" << std::endl;
	}
	else if(std::equal(temp_socket_type.begin(), temp_socket_type.end(), "SOCK_DGRAM")) {
		param.ai_socktype = SOCK_DGRAM;
		std::cout << "[INFO] param : ai_socktypeはsock_dgram(UDPプロトコルを使用)に設定されました" << std::endl;
	}
	else if (std::equal(temp_socket_type.begin(), temp_socket_type.end(), "SOCK_RDM")) {
		param.ai_socktype = SOCK_RDM;
		std::cout << "[INFO] param : ai_socktypeはSOCK_RDM(RDMプロトコルを使用)に設定されました" << std::endl;
	}
	else{
		std::cout << "[INFO] param : ai_socktypeは設定されませんでした" << std::endl;
		return false;
	}

	//プロトコルの設定
	std::string temp_protocol = socket_setting_ini["SocketSetting"]["protocol"].as<std::string>();
	//std::equal(temp_family.begin(), temp_family.end(), "AF_INET")
	//プロトコルをtcpに設定する場合
	if ((std::equal(temp_protocol.begin(), temp_protocol.end(), "IPPROTO_TCP")) && ((param.ai_family == AF_INET || param.ai_family == AF_INET6)&& param.ai_socktype == SOCK_STREAM)) {
		param.ai_protocol = IPPROTO_TCP;
		std::cout << "[INFO] param : ai_protocolはIPPROTO_TCP(tcpプロトコルを使用)に設定されました" << std::endl;
	}

	//プロトコルをudpに設定する場合
	else if ((std::equal(temp_protocol.begin(), temp_protocol.end(), "IPPROTO_UDP")) && ((param.ai_family == AF_INET || param.ai_family == AF_INET6)&& param.ai_socktype == SOCK_DGRAM)) {
		param.ai_protocol = IPPROTO_UDP;
		std::cout << "[INFO] param : ai_protocolはipproto_tcp(tcpプロトコルを使用)に設定されました" << std::endl;
	}
	//プロトコルをpgmに設定する場合
	else if ((std::equal(temp_protocol.begin(), temp_protocol.end(), "IPPROTO_PGM")) && param.ai_family == AF_INET && param.ai_socktype == SOCK_RDM) {
		param.ai_protocol = IPPROTO_PGM;
		std::cout << "[INFO] param : ai_protocolはipproto_pgm(pgmプロトコルを使用)に設定されました" << std::endl;
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
		std::cout << "[INFO] param : アプリケーションポートは" << port<<"が割り当てられました。" << std::endl;

	}
	catch (const std::exception& ex)
	{

		std::cerr << StringFormat("[ERROR] %s", ex.what()) << std::endl;
		return false;
	}
	return true;
}

/**********************************************************************************
 * @fn	  get_client_ip
 * 接続要求があった際のクライアントソケットに対してaccept処理を実行する
 * @brief accept処理
 *
 * @param [in] client_num  IPアドレス変換をする際のクライアントリストの番号
 * @return 処理結果(true:成功/false:エラー)
 *
 * @detail
 **********************************************************************************/
void SocketHandler::extraction_client_ip(int client_num)
{
	try
	{
		char temp_ip_addr_char[100];
		PCSTR iresult;
		iresult = inet_ntop(client_net_info_list[client_num].client_sockaadr_in.sin_family, &client_net_info_list[client_num].client_sockaadr_in.sin_addr,
			temp_ip_addr_char, sizeof(client_net_info_list[client_num].client_ip_addr));
		client_net_info_list[client_num].client_ip_addr = temp_ip_addr_char;
		if (iresult == NULL) {
			switch (WSAGetLastError())
			{
			case WSAEAFNOSUPPORT:
				THROW_APP_EXCEPTION("CASE:WSAEAFNOSUPPORT(Family パラメーターで指定されたアドレス ファミリ はサポートされていません。 このエラーは、指定された Family パラメーターが AF_INET または AF_INET6されていない場合に返されます。)");
				break;

			case ERROR_INVALID_PARAMETER:
				THROW_APP_EXCEPTION("CASE:ERROR_INVALID_PARAMETER(無効なパラメーターが関数に渡されました。 このエラーは、pStringBuf で NULL ポインターが渡された場合、または StringBufSize パラメーターが 0 の場合に返されます。 このエラーは、 pStringBuf パラメーターが指すバッファーの長さが IP アドレスの文字列表現を受け取るのに十分な大きさでない場合にも返されます。)");
				break;

			default:
				break;
			}
		}
	}
	catch (const std::exception& ex)
	{
		std::cerr << StringFormat("[ERROR] %s", ex.what()) << std::endl;
	}
}


void SocketHandler::init_client_net_info(int client_num)
{
	//ソケットをINVALID_SOCKETに初期化する
	client_net_info_list[client_num].client_socket = INVALID_SOCKET;

	//sockaddr_inを初期化する
	client_net_info_list[client_num].client_sockaadr_in = init_sockaddr_in;
}


void SocketHandler::print_client_net_info(int client_num)
{
	std::cout << "[INFO] client_net番号[" << client_num << "] IPアドレス(" << client_net_info_list[client_num].client_ip_addr <<")" << std::endl;
}

void SocketHandler::set_select_info(SOCKET* socket, fd_set* fd, fd_set* fds)
{
	FD_ZERO(fd);
	FD_SET(*socket, fd);

	for (int i = 0; i < MAX_CONNECT_NUM; i++) {
		
		//有効なソケットにFDSETを実行する
		if (client_net_info_list[i].client_socket != INVALID_SOCKET) {
			FD_SET(client_net_info_list[i].client_socket, fd);
		}
			}

	memcpy(fds, fd, sizeof(fd_set));

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
	std::cout << "[INFO] ソケットバインドに成功しました。" << std::endl;

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
			if (client_net_info_list[check_n_invalid_socket_result].client_socket == INVALID_SOCKET) {
				std::cout << "[ERROR] Accept要求のあったSocketが無効です(INVALID SOCKET)。"<< std::endl;

				switch (WSAGetLastError())
				{
				case WSANOTINITIALISED:
					THROW_APP_EXCEPTION("WSA ERROR CASE:WSANOTINITIALISED(この関数を使用する前に 、WSAStartup 呼び出しが正常に行われる必要があります。)");
					break;

				case WSAECONNRESET:
					THROW_APP_EXCEPTION("WSA ERROR CASE:WSAECONNRESET(着信接続が示されましたが、その後、呼び出しを受け入れる前にリモート ピアによって終了されました。)");
					break;

				case WSAEFAULT:
					THROW_APP_EXCEPTION("WSA ERROR CASE:WSAEFAULT(addrlen パラメーターが小さすぎるか、addr がユーザー アドレス空間の有効な部分ではありません。)");
					break;

				case WSAEINTR:
					THROW_APP_EXCEPTION("WSA ERROR CASE:WSAEINTR(WSACancelBlockingCall を使用して、ブロックしている Windows ソケット 1.1 呼び出しが取り消されました。)");
					break;

				case WSAEINVAL:
					THROW_APP_EXCEPTION("WSA ERROR CASE:WSAEINVAL(リッスン関数は、受け入れる前に呼び出されませんでした。)");
					break;
					
				case WSAEINPROGRESS:
					THROW_APP_EXCEPTION("WSA ERROR CASE:WSAEINPROGRESS(ブロックしている Windows Sockets 1.1 呼び出しが進行中であるか、サービス プロバイダーがコールバック関数を処理しています。)");
					break;

				case EMFILE:
					THROW_APP_EXCEPTION("WSA ERROR CASE:EMFILE(キューは、 受け入れる エントリに空きがなく、使用可能な記述子がありません。)");
					break;

				case WSAENETDOWN:
					THROW_APP_EXCEPTION("WSA ERROR CASE:WSAENETDOWN(ネットワーク サブシステムが失敗しました。)");
					break;

				case WSAENOBUFS:
					THROW_APP_EXCEPTION("WSA ERROR CASE:WSAENOBUFS(バッファーに空き領域がありません。)");
					break;

				case WSAENOTSOCK:
					THROW_APP_EXCEPTION("WSA ERROR CASE:WSAENOTSOCK(記述子はソケットではありません。)");
					break;

				case WSAEOPNOTSUPP:
					THROW_APP_EXCEPTION("CASE:WSAEOPNOTSUPP(参照されるソケットは、接続指向サービスをサポートする型ではありません。)");
					break;

				case WSAEWOULDBLOCK:
					THROW_APP_EXCEPTION("CASE:WSAEWOULDBLOCK(ソケットは非ブロッキングとしてマークされ、受け入れられる接続はありません。)");
					break;

				default:
					
					break;
				}
			}
			else {
				//acceptが成功した場合
				extraction_client_ip(check_n_invalid_socket_result);

				std::cout << "[INFO] クライアントリスト[ " << check_n_invalid_socket_result
					<< " ]にIPアドレス(" << client_net_info_list[check_n_invalid_socket_result].client_ip_addr << ")が登録されました。"<<std::endl;
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

/**********************************************************************************
 * @fn	  search_connect_client_socket_num
 * SEARCH要求があったクライアントのソケット番号を返す
 * @brief accept処理
 *
 * @param [in] なし
 * @return 処理結果(接続中のクライアントにSEARCH要求があったIPアドレスがあれば、リストの番号を返す)
 * 　　　　IPアドレスがない場合は、
 *
 * @detail
 **********************************************************************************/
int SocketHandler::search_connect_client_socket_num(const std::string search_ip_addr)
{
	for (int search_result = 0; search_result < MAX_CONNECT_NUM; search_result++) {

		if (client_net_info_list[search_result].client_ip_addr == search_ip_addr) {
			return search_result;
		}
	}
	return NOT_EXIST_IP_ADDRESS;
}

void SocketHandler::disconnect_socket(int socket_num)
{
	try
	{
		int close_socket_result = closesocket(client_net_info_list[socket_num].client_socket);
		if (close_socket_result == SOCKET_ERROR) {
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSANOTINITIALISED(この関数を使用する前に、 WSAStartup 呼び出しが正常に行われる必要があります。)");
				break;
			case WSAENETDOWN:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAENETDOWN(ネットワーク サブシステムが失敗しました。)");
				break;
			case WSAENOTSOCK:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAENOTSOCK(記述子はソケットではありません。)");
				break;
			case WSAEINPROGRESS:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAEINPROGRESS(ブロックしている Windows Sockets 1.1 呼び出しが進行中であるか、サービス プロバイダーがコールバック関数を処理しています。)");
				break;
			case WSAEINTR:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAEINTR((ブロッキング) Windows Socket 1.1 呼び出しは 、WSACancelBlockingCall を介して取り消されました。)");
				break;
			case WSAEWOULDBLOCK:
				THROW_APP_EXCEPTION("WSA ERROR CASE : WSAEWOULDBLOCK(ソケットは非ブロッキングとしてマークされていますが、残留構造のl_onoff メンバーは 0 以外に設定され、残留構造のl_linger メンバーは 0 以外のタイムアウト値に設定されます。)");
				break;

			default:
				break;
			}
		}
	}
	catch (const std::exception& ex)
	{
		std::cerr << StringFormat("[ERROR] %s", ex.what()) << std::endl;
	}

	init_client_net_info(socket_num);
	
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
		listen_socket = socket(result -> ai_family, result -> ai_socktype, result->ai_protocol);
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
	std::cout << "[INFO]ソケットが正常に生成されました" << std::endl;

	bind_listen_socket();

	return true;
}

int SocketHandler::monitor_socket()
{
	try
	{
		
		set_select_info(&listen_socket, &fd_read, &fds);

		int select_result = 0;
		select_result = select(0, &fds, NULL, NULL, &timeout);

		switch (select_result)
		{
			//fd_set構造体に含まれているソケット ハンドルの合計数、期限切れの場合
		case 0:
			//THROW_APP_EXCEPTION("CASE:SELECT FUNCTION ERROR(fd_set構造体に含まれているソケット ハンドルの合計数、期限切れです。)");
			return SELECT_TIME_OUT;
			break;

			//エラーが発生した場合
		case SOCKET_ERROR:
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				THROW_APP_EXCEPTION("WSA ERROR CASE:WSANOTINITIALISED(この関数を使用する前に 、WSAStartup 呼び出しが成功する必要があります。)");
				break;
			case WSAEFAULT:
				THROW_APP_EXCEPTION("WSA ERROR CASE:WSAEFAULT(Windows ソケットの実装では、内部操作に必要なリソースを割り当てることができませんでした。または、 readfds、 writefds、 exceptfds、または timeval パラメーターがユーザー アドレス空間の一部ではありません。)");
				break;
			case ENETDOWN:
				THROW_APP_EXCEPTION("WSA ERROR CASE:ENETDOWN(ネットワーク サブシステムが失敗しました。)");
				break;
			case WSAEINVAL:
				THROW_APP_EXCEPTION("WSA ERROR CASE:WSAEINVAL(タイムアウト値が無効であるか、3 つの記述子パラメーターがすべて null でした。)");
				break;
			case WSAEINTR:
				THROW_APP_EXCEPTION("WSA ERROR CASE:WSAEINTR(WSACancelBlockingCall を使用して、Windows ソケット 1.1 呼び出しのブロックが取り消されました。)");
				break;
			case WSAEINPROGRESS:
				THROW_APP_EXCEPTION("WSA ERROR CASE:WSAEINPROGRESS(ブロック中の Windows ソケット 1.1 呼び出しが進行中であるか、サービス プロバイダーがコールバック関数を処理しています。)");
				break;
			case WSAENOTSOCK:
				THROW_APP_EXCEPTION("WSA ERROR CASE:WSAENOTSOCK(記述子セットの 1 つに、ソケットではないエントリが含まれています。)");
				break;
			default:
				break;
			}
			break;



			//ディスクリプションに変化が起こった場合
		default:
			if (FD_ISSET(listen_socket, &fds)) {
				std::cout << "[INFO]新規クライアントから接続要求がありました。" << std::endl;
				return REQUEST_ACCEPT_CLIENT_DETECTION;
			}
			else {
				//接続中のクライアントリストのどのクライアントからの通知かを判定する
				for (int detection_client_num = 0; detection_client_num < MAX_CONNECT_NUM; detection_client_num++) {
					if (client_net_info_list[detection_client_num].client_socket != INVALID_SOCKET && FD_ISSET(client_net_info_list[detection_client_num].client_socket, &fds)) {
						std::cout << "[INFO]接続中のクライアント(client_net_info_list[" << detection_client_num << "]から要求がありました" << std::endl;
						return detection_client_num;
					}
				}
			}
			break;
		}
	}
	catch (const std::exception& ex)
	{
		std::cerr << StringFormat("[ERROR] %s", ex.what()) << std::endl;
		return SOCKET_ERROR;
	}

	return 0;
}

void SocketHandler::active_listen_socket()
{
	try
	{
		if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {

			freeaddrinfo(result);
			closesocket(listen_socket);
			WSACleanup();
			THROW_APP_EXCEPTION("ERROR");

		}
		else
		{
			std::cout << "[INFO] Listenソケットがアクティブ状態になりました。" << std::endl;
		}
	}
	catch (const std::exception& ex)
	{
		std::cerr << StringFormat("[ERROR] %s", ex.what()) << std::endl;
	}
	
}
