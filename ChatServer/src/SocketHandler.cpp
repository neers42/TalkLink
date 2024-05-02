#include "SocketHandler.h"

//#define DEBUG

/*
* 【未実装リスト】
* マルチキャスト用ソケット及びブロードキャスト用のソケットに対応する
*/

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
	ZeroMemory(&m_param, sizeof(m_param));

	try
	{
		int init_wsa_result = WSAStartup(MAKEWORD(2, 2), &m_wsa_data);
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
	catch (const exception::ExceptionBase& ex)
	{
		std::cerr << StringFormat("%s[ERROR]%s", ServerTime::get_current_time(),ex.what_for_str()) << std::endl;
	}

	//ファイルディスクリプタキャッチ用のfdsを初期化
	FD_ZERO(&m_fds);
	FD_ZERO(&m_fd_read);

	//select関数のタイムアウト初期設定(tv_sec=秒の指定　tv_usec=マイクロ秒の指定)
	//デフォルトコンストラクタでは500マイクロ秒 * 1000　⇒ 500ミリ秒に設定
	m_timeout.tv_sec = 0;
	m_timeout.tv_usec = 500 * 1000;

	//リッスンソケットの初期化
	m_tcp_listen_socket = INVALID_SOCKET;

	//UDPブロードキャスト送信用ソケットの初期化
	m_udp_bloadcast_tx_socket = INVALID_SOCKET;

	//UDPマルチキャスト送信用ソケット
	m_udp_multicast_tx_socket = INVALID_SOCKET;

	//ソケット初期化用の変数の初期化
	m_init_sockaddr_in.sin_family = AF_INET;
	m_init_sockaddr_in.sin_port = 0;
	m_init_sockaddr_in.sin_addr.S_un.S_addr = INADDR_ANY;


	
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
	ZeroMemory(&m_param, sizeof(m_param));
	//wsa_dataの初期化
	try
	{
		int init_wsa_result = WSAStartup(MAKEWORD(2, 2), &m_wsa_data);
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
	catch (const exception::ExceptionBase& ex)
	{
		std::cerr << StringFormat("%s[ERROR]%s", ServerTime::get_current_time(),ex.what_for_str()) << std::endl;
	}

	//ファイルディスクリプタキャッチ用のfdsを初期化
	FD_ZERO(&m_fds);
	FD_ZERO(&m_fd_read);

	//select関数のタイムアウト初期設定(tv_sec=秒の指定　tv_usec=マイクロ秒の指定)
	m_timeout.tv_sec = tv_sec;
	m_timeout.tv_usec = tv_usec;
	
	//リッスンソケットの初期化
	m_tcp_listen_socket = INVALID_SOCKET;


	m_init_sockaddr_in.sin_family = AF_INET;
	m_init_sockaddr_in.sin_port = 50000;
	m_init_sockaddr_in.sin_addr.S_un.S_addr = INADDR_ANY;

}

//デストラクタ
SocketHandler::~SocketHandler()
{
	freeaddrinfo(m_result);
	closesocket(get_listen_socket());
	closesocket(get_udp_multicast_tx_socket());
	WSACleanup();
}


//ここのtry構文見直す
//設定値の読み込み
bool SocketHandler::setting_tcp_listen_socket(const std::string& file_pass)
{

	//iniファイルの読み込み
	ini::IniFile tcp_listen_socket_setting_ini;
	try
	{
		tcp_listen_socket_setting_ini.load(file_pass);
	}
	catch (const exception::ExceptionBase& ex)
	{
		THROW_APP_EXCEPTION("iniファイルの読み込みに失敗しました。");
	}

	std::cout << StringFormat("%s[INFO]Loading setting init file ...", ServerTime::get_current_time());
	
	//フラッグの設定
	std::string temp_flag = tcp_listen_socket_setting_ini["SocketSetting"]["flags"].as<std::string>();
	std::cout << "\033[32m [OK] \033[m" << std::endl;
	
	std::cout << StringFormat("%s[INFO]Setting flag in parameter...", ServerTime::get_current_time());
	if (std::equal(temp_flag.begin(), temp_flag.end(), "AI_PASSIVE")) {
		m_param.ai_flags = AI_PASSIVE;
#ifdef DEBUG
		std::cout << ServerTime::get_current_time() << "[INFO](tcp_listen setting)param：ai_flagsはAI_PASSIVEに設定されました。" << std::endl;
#endif // DEBUG
	}
	else if (std::equal(temp_flag.begin(), temp_flag.end(), "AI_CANONNAME")) {
		m_param.ai_flags = AI_CANONNAME;
#ifdef DEBUG
		std::cout << ServerTime::get_current_time() << "[INFO](tcp_listen setting)param：ai_flagsはAI_CANONNAMEに設定されました。" << std::endl;
#endif // DEBUG

	}
	else if (std::equal(temp_flag.begin(), temp_flag.end(), "AI_NUMERICHOST")) {
		m_param.ai_flags = AI_NUMERICHOST;
#ifdef DEBUG
		std::cout << ServerTime::get_current_time() << "[INFO](tcp_listen setting)param：ai_flagsはAI_NUMERICHOSTに設定されました。" << std::endl;
#endif // DEBUG
	}
	else if (std::equal(temp_flag.begin(), temp_flag.end(), "AI_ALL")) {
		m_param.ai_flags = AI_ALL;
#ifdef DEBUG
		std::cout << ServerTime::get_current_time() << "[INFO](tcp_listen setting)param：ai_flagsはAI_ALLに設定されました。" << std::endl;
#endif // DEBUG

	}
	else if (std::equal(temp_flag.begin(), temp_flag.end(), "AI_ADDRCONFIG")) {
		m_param.ai_flags = AI_ADDRCONFIG;
#ifdef DEBUG
		std::cout << ServerTime::get_current_time() << "[INFO](tcp_listen setting)param：ai_flagsはAI_ADDRCONFIGに設定されました。" << std::endl;
#endif // DEBUG	
	}
	else if (std::equal(temp_flag.begin(), temp_flag.end(), "AI_V4MAPPED")) {
		m_param.ai_flags = AI_V4MAPPED;
#ifdef DEBUG
		std::cout << ServerTime::get_current_time() << "[INFO](tcp_listen setting)param：ai_flagsはAI_V4MAPPEDに設定されました。" << std::endl;
#endif // DEBUG
	}
	else {
		std::cout << "\033[31m [ERROR]\033[m" << std::endl;
		return false;
	}
	std::cout << "\033[32m [OK] \033[m" << std::endl;

	std::cout << StringFormat("%s[INFO]Setting the address family in the parameters...", ServerTime::get_current_time());
	//iniファイルの各セクションからキーを読み込む
	//アドレスファミリの設定(ipv4 or ipv6)
	std::string temp_family = tcp_listen_socket_setting_ini["SocketSetting"]["family"].as<std::string>();

	//通信プロトコル
	if (std::equal(temp_family.begin(), temp_family.end(), "AF_INET") ){
		m_param.ai_family = AF_INET;
#ifdef DEBUG
		std::cout << ServerTime::get_current_time() << "[INFO](tcp_listen setting)param：ai_familyはAF_INET(アドレスファミリはipv4に設定されました)に設定されました。" << std::endl;
#endif // DEBUG

	}
	//通信プロトコルをipv6に設定する場合
	else if (std::equal(temp_family.begin(), temp_family.end(), "AF_INET6")) {
		m_param.ai_family = AF_INET6;
#ifdef DEBUG
		std::cout << ServerTime::get_current_time() << "[INFO](tcp_listen setting)param：ai_familyはAF_INET6(アドレスファミリはipv6に設定されました)に設定されました。" << std::endl;
#endif // DEBUG
		
	}
	//通信プロトコルがaf_unspecに設定された場合(ipv4とipv6に対応)
	//通信プロトコルがどちらにも設定していない場合及び読み取れなかった場合(af_unspecに設定)
	else if(std::equal(temp_family.begin(), temp_family.end(), "AF_UNSPEC")) {
		m_param.ai_family = AF_UNSPEC;
#ifdef DEBUG
		std::cout << ServerTime::get_current_time() << "[INFO](tcp_listen setting)param：ai_familyはAF_UNSPEC(アドレスファミリはAF=UNSPECに設定されました)に設定されました。" << std::endl;
#endif // DEBUG
	}
	else {
		std::cout << "\033[31m [ERROR]\033[m" << std::endl;
		return false;
	}
	std::cout << "\033[32m [OK] \033[m" << std::endl;


	std::cout << StringFormat("%s[INFO]Setting the socket type in the parameters...", ServerTime::get_current_time());
	//ソケット型の設定
	std::string temp_socket_type = tcp_listen_socket_setting_ini["SocketSetting"]["socktype"].as<std::string>();
	if (std::equal(temp_socket_type.begin(), temp_socket_type.end(), "SOCK_STREAM")) {
		m_param.ai_socktype = SOCK_STREAM;
#ifdef DEBUG
		std::cout << ServerTime::get_current_time() << "[INFO](tcp_listen setting)param：ai_socktypeはSOCK_STREAM(TCPプロトコルを使用)に設定されました。" << std::endl;
#endif // DEBUG		
	}
	else if(std::equal(temp_socket_type.begin(), temp_socket_type.end(), "SOCK_DGRAM")) {
		m_param.ai_socktype = SOCK_DGRAM;
#ifdef DEBUG
		std::cout << ServerTime::get_current_time() << "[INFO](tcp_listen setting)param：ai_socktypeはsock_dgram(UDPプロトコルを使用)に設定されました。" << std::endl;
#endif // DEBUG

	}
	else if (std::equal(temp_socket_type.begin(), temp_socket_type.end(), "SOCK_RDM")) {
		m_param.ai_socktype = SOCK_RDM;
#ifdef DEBUG
		std::cout << ServerTime::get_current_time() << "[INFO](tcp_listen setting)param：ai_socktypeはSOCK_RDM(RDMプロトコルを使用)に設定されました。" << std::endl;
#endif // DEBUG

	}
	else{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("(tcp_listen setting)param：ai_socktypeは設定されなかったため、プログラムを終了します。"));
		return false;
	}
	std::cout << "\033[32m [OK] \033[m" << std::endl;

	std::cout << StringFormat("%s[INFO]Setting the protocol in the parameters...", ServerTime::get_current_time());
	//プロトコルの設定
	std::string temp_protocol = tcp_listen_socket_setting_ini["SocketSetting"]["protocol"].as<std::string>();
	//std::equal(temp_family.begin(), temp_family.end(), "AF_INET")
	//プロトコルをtcpに設定する場合
	if ((std::equal(temp_protocol.begin(), temp_protocol.end(), "IPPROTO_TCP")) && ((m_param.ai_family == AF_INET || m_param.ai_family == AF_INET6)&& m_param.ai_socktype == SOCK_STREAM)) {
		m_param.ai_protocol = IPPROTO_TCP;
#ifdef DEBUG
		std::cout << ServerTime::get_current_time() << "[INFO](tcp_listen setting)param：ai_protocolはIPPROTO_TCP(tcpプロトコルを使用)に設定されました。" << std::endl;
#endif // DEBUG
	}

	//プロトコルをudpに設定する場合
	else if ((std::equal(temp_protocol.begin(), temp_protocol.end(), "IPPROTO_UDP")) && ((m_param.ai_family == AF_INET || m_param.ai_family == AF_INET6)&& m_param.ai_socktype == SOCK_DGRAM)) {
		m_param.ai_protocol = IPPROTO_UDP;
#ifdef DEBUG
		std::cout << ServerTime::get_current_time() << "[INFO](tcp_listen setting)param：ai_protocolはipproto_tcp(tcpプロトコルを使用)に設定されました。" << std::endl;
#endif // DEBUG
	}
	//プロトコルをpgmに設定する場合
	else if ((std::equal(temp_protocol.begin(), temp_protocol.end(), "IPPROTO_PGM")) && m_param.ai_family == AF_INET && m_param.ai_socktype == SOCK_RDM) {
		m_param.ai_protocol = IPPROTO_PGM;

#ifdef DEBUG
		std::cout << ServerTime::get_current_time() << "[INFO](tcp_listen setting)param：ai_protocolはipproto_pgm(pgmプロトコルを使用)に設定されました。" << std::endl;
#endif // DEBUG
	}
	else {
		std::cout << "\033[31m [ERROR] \033[m" << std::endl;
		return false;
	}
	std::cout << "\033[32m [OK] \033[m" << std::endl;

	std::cout << StringFormat("%s[INFO]Setting the port in the parameters...", ServerTime::get_current_time());
	//ポートの設定
	try
	{
		int temp_port = tcp_listen_socket_setting_ini["SocketSetting"]["port"].as<int>();

		if (temp_port < 0 || temp_port > 65535) {

			THROW_APP_EXCEPTION("The port number is set out of range. Reset to the range of 0~65535.");
		}
		m_port = temp_port;
#ifdef DEBUG
		std::cout << ServerTime::get_current_time() << "[INFO](tcp_listen setting)param：アプリケーションポートは" << m_port << "が割り当てられました。" << std::endl;

#endif // DEBUG
		
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));

		return false;
	}
	std::cout << "\033[32m [OK] \033[m" << std::endl;
	return true;
}

bool SocketHandler::setting_multicast_tx_socket(const std::string& file_pass)
{
	//iniファイルの読み込み
	ini::IniFile multicast_socket_setting_ini;
	try
	{
		multicast_socket_setting_ini.load(file_pass);
	}
	catch (const exception::ExceptionBase& ex)
	{
		THROW_APP_EXCEPTION("Failed to load the file for initialization.");
	}
	std::cout << StringFormat("%s[INFO]The file for initial configuration was successfully loaded.", ServerTime::get_current_time()) << std::endl;

	//アドレスファミリの設定(socket関数の引数設定)
	std::string temp_af = multicast_socket_setting_ini["MulticastTX_SocketSetting"]["sin_family"].as<std::string>();
	if (std::equal(temp_af.begin(), temp_af.end(), "AF_INET")) {
		m_multicast_tx_param.ai_family = AF_INET;
		std::cout << ServerTime::get_current_time() << "[INFO](multicast_tx)param：afはAF_INET(IPv4)に設定されました。" << std::endl;
	}
	else if (std::equal(temp_af.begin(), temp_af.end(), "AF_IPX")) {
		m_multicast_tx_param.ai_family = AF_IPX;
		std::cout << ServerTime::get_current_time() << "[INFO](multicast_tx)param：afはAF_IPXに設定されました。" << std::endl;
	}
	else if (std::equal(temp_af.begin(), temp_af.end(), "AF_APPLETALK")) {
		m_multicast_tx_param.ai_family = AF_APPLETALK;
		std::cout << ServerTime::get_current_time() << "[INFO](multicast_tx)param：afはAF_APPLETALKに設定されました。" << std::endl;
	}
	else if (std::equal(temp_af.begin(), temp_af.end(), "AF_NETBIOS")) {
		m_multicast_tx_param.ai_family = AF_NETBIOS;
		std::cout << ServerTime::get_current_time() << "[INFO](multicast_tx)param：afはAF_NETBIOSに設定されました。" << std::endl;
	}
	else if (std::equal(temp_af.begin(), temp_af.end(), "AF_INET6")) {
		m_multicast_tx_param.ai_family = AF_INET6;
		std::cout << ServerTime::get_current_time() << "[INFO](multicast_tx)param：afはAF_INET6(IPv6)に設定されました。" << std::endl;
	}
	else if (std::equal(temp_af.begin(), temp_af.end(), "AF_IRDA")) {
		m_multicast_tx_param.ai_family = AF_IRDA;
		std::cout << ServerTime::get_current_time() << "[INFO](multicast_tx)param：afはAF_IRDAに設定されました。" << std::endl;
	}
	else if (std::equal(temp_af.begin(), temp_af.end(), "AF_BTH")) {
		m_multicast_tx_param.ai_family = AF_BTH;
		std::cout << ServerTime::get_current_time() << "[INFO](multicast_tx)param：afはAF_BTHに設定されました。" << std::endl;
	}
	else {
		return false;
	}

	//ソケットタイプの設定
	std::string temp_socktype = multicast_socket_setting_ini["MulticastTX_SocketSetting"]["type"].as<std::string>();
	if (std::equal(temp_socktype.begin(), temp_socktype.end(), "SOCK_STREAM")) {
		m_multicast_tx_param.ai_socktype = SOCK_STREAM;
	}
	else if (std::equal(temp_socktype.begin(), temp_socktype.end(), "SCOK_DGRAM")) {
		m_multicast_tx_param.ai_socktype = SOCK_DGRAM;
	}
	else if (std::equal(temp_socktype.begin(), temp_socktype.end(), "SOCK_RAW")) {
		m_multicast_tx_param.ai_socktype = SOCK_RAW;
	}
	else if (std::equal(temp_socktype.begin(), temp_socktype.end(), "SOCK_RDM")) {
		m_multicast_tx_param.ai_socktype = SOCK_RDM;
	}
	else if (std::equal(temp_socktype.begin(), temp_socktype.end(), "SOCK_SEQPACKET")) {
		m_multicast_tx_param.ai_socktype = SOCK_SEQPACKET;
	}
	else {
		return false;
	}

	//マルチキャストアドレスのsockaddr_inのパラメータを設定する。
	m_multicast_tx_addr.sin_family = AF_INET;

	//マルチキャストのポートを設定する。
	try
	{
		int temp_port = multicast_socket_setting_ini["MulticastTX_SocketSetting"]["port"].as<int>();
		if (temp_port < 0 || temp_port > 65535) {

			THROW_APP_EXCEPTION("The port number is set out of range. Reset to the range of 0 - 65535.");
		}

		if (temp_port == m_port) {
			THROW_APP_EXCEPTION("There is a conflict between the application port used for TCP and the application port used for UDP multicast. Please reset the settings.");
		}

		m_multicast_tx_addr.sin_port = temp_port;
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return false;
	}


	std::string temp_multicast_addr = multicast_socket_setting_ini["MulticastTX_SocketSetting"]["multicast_address"].as<std::string>();
	//multicast_tx_addr.sin_addr.S_un.S_addr = inet_addr(temp_multicast_addr.c_str());

	//inet_addrの設定
	try
	{
		int inet_pton_result = inet_pton(m_multicast_tx_param.ai_family, temp_multicast_addr.c_str(), &m_multicast_tx_addr.sin_addr);
		if (inet_pton_result == -1) {
			switch (WSAGetLastError())
			{
			case WSAEAFNOSUPPORT:
				THROW_APP_EXCEPTION("WSA ERROR CASE:WSAEAFNOSUPPORT(Family パラメーターで指定されたアドレス ファミリ はサポートされていません。 このエラーは、指定された Family パラメーターが AF_INET または AF_INET6されていない場合に返されます。)");
				break;

			case WSAEFAULT:
				THROW_APP_EXCEPTION("WSA ERROR CASE:WSAEFAULT(pszAddrString パラメーターまたは pAddrBuf パラメーターが NULL であるか、ユーザー アドレス空間の一部ではありません。)");
				break;

			default:
				break;
			}
		}
		else if (inet_pton_result == 0) {
			THROW_APP_EXCEPTION("pAddrBuf パラメーターが有効な IPv4 ドット 10 進数文字列または有効な IPv6 アドレス文字列ではない文字列を指しています。");
		}
		//IPアドレスのバイナリ化が正常に終了した場合
		else {

			;
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
	}


	return true;
	
}

bool SocketHandler::setting_bloadcast_tx_socket(const std::string& file_pass)
{
	return false;
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

std::string SocketHandler::extraction_client_ip(sockaddr_in& client_sockaddr_in, size_t ipv4_addr_size)
{
	try
	{
		char client_ip_addr_char[100];
		struct sockaddr_in temp_paddr = client_sockaddr_in;
		PCSTR iresult;
		iresult = inet_ntop(temp_paddr.sin_family, &temp_paddr.sin_addr,
			client_ip_addr_char,ipv4_addr_size);
		std::string client_ip_addr_str(client_ip_addr_char);
		//client_handler->get_client_list()[client_num].set_client_ipv4_addr(temp_ip_addr_char);
		//new_client->set_client_ipv4_addr(temp_ip_addr_char);
		if (iresult == NULL) {
			switch (WSAGetLastError())
			{
			case WSAEAFNOSUPPORT:
				THROW_APP_EXCEPTION("WSA ERROR CASE:WSAEAFNOSUPPORT(Family パラメーターで指定されたアドレス ファミリ はサポートされていません。 このエラーは、指定された Family パラメーターが AF_INET または AF_INET6されていない場合に返されます。)");
				break;

			case ERROR_INVALID_PARAMETER:
				THROW_APP_EXCEPTION("WSA ERROR CASE:ERROR_INVALID_PARAMETER(無効なパラメーターが関数に渡されました。 このエラーは、pStringBuf で NULL ポインターが渡された場合、または StringBufSize パラメーターが 0 の場合に返されます。 このエラーは、 pStringBuf パラメーターが指すバッファーの長さが IP アドレスの文字列表現を受け取るのに十分な大きさでない場合にも返されます。)");
				break;

			default:
				break;
			}
		}else {
			return client_ip_addr_str;
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
	}
}


void SocketHandler::setting_select_info_to_listen_socket()
{
	FD_ZERO(&this->m_fd_read);
	FD_SET(this->m_tcp_listen_socket, &this->m_fd_read);
	

}

void SocketHandler::setting_select_info_to_client_socket(const SOCKET& client_socket, fd_set* fd)
{
	FD_SET(client_socket, fd);
	
}

void SocketHandler::copy_fd_read_to_fds()
{
	memcpy(&this->m_fds, &this->m_fd_read, sizeof(fd_set));
}




bool SocketHandler::generate_udp_multicast_tx_socket()
{
	if (!setting_multicast_tx_socket(INIT_FILE_PASS)) {
		std::cerr << StringFormat("%s\033[31m[ERROR]The socket configuration for multicast transmission was not completed successfully and will be terminated.");
		return false;
	}

	try
	{
		std::cout << StringFormat("%s[INFO]Creating UDP Multicast TX socket...") << std::endl;
		SOCKET result_socket = socket(m_multicast_tx_param.ai_family, m_multicast_tx_param.ai_socktype, 0);
		if (result_socket == INVALID_SOCKET) {
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				THROW_APP_EXCEPTION("A successful WSAStartup call must occur before using this function.");
				break;

			case WSAENETDOWN:
				THROW_APP_EXCEPTION("The network subsystem or the associated service provider has failed.");
				break;

			case WSAEAFNOSUPPORT:
				THROW_APP_EXCEPTION("The specified address family is not supported. For example, an application tried to create a socket for the AF_IRDA address family but an infrared adapter and device driver is not installed on the local computer.");
				break;

			case WSAEINPROGRESS:
				THROW_APP_EXCEPTION("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.");
				break;

			case WSAEMFILE:
				THROW_APP_EXCEPTION("No more socket descriptors are available.");
				break;

			case WSAEINVAL:
				THROW_APP_EXCEPTION("An invalid argument was supplied. This error is returned if the af parameter is set to AF_UNSPEC and the type and protocol parameter are unspecified.");
				break;

			case WSAEINVALIDPROVIDER:
				THROW_APP_EXCEPTION("The service provider returned a version other than 2.2.");
				break;

			case WSAENOBUFS:
				THROW_APP_EXCEPTION("The service provider returned an invalid or incomplete procedure table to the WSPStartup.");
				break;

			case WSAEPROTONOSUPPORT:
				THROW_APP_EXCEPTION("No buffer space is available. The socket cannot be created.");
				break;

			case WSAEPROTOTYPE:
				THROW_APP_EXCEPTION("The specified protocol is not supported.");
				break;

			case WSAEPROVIDERFAILEDINIT:
				THROW_APP_EXCEPTION("The specified protocol is the wrong type for this socket.");
				break;

			case WSAESOCKTNOSUPPORT:
				THROW_APP_EXCEPTION("The specified socket type is not supported in this address family.");
				break;


			default:
				m_udp_multicast_tx_socket = result_socket;
				std::cout << "\033[32m [OK] \033[m" << std::endl;
				break;
			}
		
		
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return false;
	}
	
	//setsocloptの設定
	try
	{
		int multicast_if_set_sockopt_result = setsockopt(this->m_udp_multicast_tx_socket, 
											IPPROTO_IP, IP_MULTICAST_IF,
											this->m_local_ip_addr.c_str(), 
											sizeof(this->m_local_ip_addr.c_str()));
		if (!multicast_if_set_sockopt_result) {
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				THROW_APP_EXCEPTION("A successful WSAStartup call must occur before using this function.");
				break;

			case WSAENETDOWN:
				THROW_APP_EXCEPTION("The network subsystem has failed.");
				break;

			case WSAEFAULT:
				THROW_APP_EXCEPTION("The buffer pointed to by the optval parameter is not in a valid part of the process address space or the optlen parameter is too small.");
				break;

			case WSAEINPROGRESS:
				THROW_APP_EXCEPTION("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.");
				break;

			case WSAEINVAL:
				THROW_APP_EXCEPTION("The level parameter is not valid, or the information in the buffer pointed to by the optval parameter is not valid.");
				break;

			case WSAENETRESET:
				THROW_APP_EXCEPTION("The connection has timed out when SO_KEEPALIVE is set.");
				break;

			case WSAENOPROTOOPT:
				THROW_APP_EXCEPTION("The option is unknown or unsupported for the specified provider or socket (see SO_GROUP_PRIORITY limitations).");
				break;

			case WSAENOTCONN:
				THROW_APP_EXCEPTION("The connection has been reset when SO_KEEPALIVE is set.");
				break;

			case WSAENOTSOCK:
				THROW_APP_EXCEPTION("The descriptor is not a socket.");
				break;

			default:
				break;
			}
		}

	}
	catch (const exception::ExceptionBase& ex)
	{
		std::cerr << StringFormat("%s\033[31m[ERORR][m%s", ServerTime::get_current_time(), ex.what_for_str());
		
		//TODOネットワークインタフェースの数で戻り値を変えたい
	}
	
	try
	{
		int multicast_ttl_set_sockopt_result = setsockopt(m_udp_multicast_tx_socket,
			IPPROTO_IP,
			IP_MULTICAST_TTL,
			(char*)&m_ttl,
			sizeof(m_ttl));

		if (!multicast_ttl_set_sockopt_result) {
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				THROW_APP_EXCEPTION("A successful WSAStartup call must occur before using this function.");
				break;

			case WSAENETDOWN:
				THROW_APP_EXCEPTION("The network subsystem has failed.");
				break;

			case WSAEFAULT:
				THROW_APP_EXCEPTION("The buffer pointed to by the optval parameter is not in a valid part of the process address space or the optlen parameter is too small.");
				break;

			case WSAEINPROGRESS:
				THROW_APP_EXCEPTION("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.");
				break;

			case WSAEINVAL:
				THROW_APP_EXCEPTION("The level parameter is not valid, or the information in the buffer pointed to by the optval parameter is not valid.");
				break;

			case WSAENETRESET:
				THROW_APP_EXCEPTION("The connection has timed out when SO_KEEPALIVE is set.");
				break;

			case WSAENOPROTOOPT:
				THROW_APP_EXCEPTION("The option is unknown or unsupported for the specified provider or socket (see SO_GROUP_PRIORITY limitations).");
				break;

			case WSAENOTCONN:
				THROW_APP_EXCEPTION("The connection has been reset when SO_KEEPALIVE is set.");
				break;

			case WSAENOTSOCK:
				THROW_APP_EXCEPTION("The descriptor is not a socket.");
				break;

			default:
				break;
			}

		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
	}

	return true;
}


bool SocketHandler::obtain_local_ip_addr()
{
	DWORD get_ip_addr_table_result;
	//IPアドレステーブルの取得
	try
	{
		if (GetIpAddrTable(NULL, &m_dw_size, 0) == ERROR_INSUFFICIENT_BUFFER) {
			m_ip_addr_table = (MIB_IPADDRTABLE*)malloc(m_dw_size);
		}

		get_ip_addr_table_result = GetIpAddrTable(m_ip_addr_table, &m_dw_size, 0);
		switch (get_ip_addr_table_result)
		{
		case ERROR_INSUFFICIENT_BUFFER:
			THROW_APP_EXCEPTION("ERROR CASE: ERROR_INSUFFICIENT_BUFFER(The buffer pointed to by the pIpAddrTable parameter is not large enough. The required size is returned in the DWORD variable pointed to by the pdwSize parameter.)");
			break;

		case ERROR_INVALID_PARAMETER:
			THROW_APP_EXCEPTION("ERROR CASE:ERROR_INVALID_PARAMETER(The pdwSize parameter is NULL, or GetIpAddrTable is unable to write to the memory pointed to by the pdwSize parameter.)");
			break;

		case ERROR_NOT_SUPPORTED:
			THROW_APP_EXCEPTION("ERROR CASE:ERROR_NOT_SUPPORTED(This function is not supported on the operating system in use on the local system.)");
			break;

		case NO_ERROR:
			std::cout << StringFormat("%s[INFO]Successfully acquired IP address table!!.", ServerTime::get_current_time()) << std::endl;
			break;
		default:
			THROW_APP_EXCEPTION("ERROR CASE:Other(Use FormatMessage to obtain the message string for the returned error.)");
			break;
		}

	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));

	}


	//IPアドレステーブルの結果を格納
	try
	{
		DWORD dw_ret_val_result = GetIpAddrTable(m_ip_addr_table, &m_dw_size, 0);
		switch (dw_ret_val_result)
		{
		case ERROR_INSUFFICIENT_BUFFER:
			THROW_APP_EXCEPTION("ERROR CASE: ERROR_INSUFFICIENT_BUFFER(The buffer pointed to by the pIpAddrTable parameter is not large enough. The required size is returned in the DWORD variable pointed to by the pdwSize parameter.)");
			break;

		case ERROR_INVALID_PARAMETER:
			THROW_APP_EXCEPTION("ERROR CASE:ERROR_INVALID_PARAMETER(The pdwSize parameter is NULL, or GetIpAddrTable is unable to write to the memory pointed to by the pdwSize parameter.)");
			break;

		case ERROR_NOT_SUPPORTED:
			THROW_APP_EXCEPTION("ERROR CASE:ERROR_NOT_SUPPORTED(This function is not supported on the operating system in use on the local system.)");
			break;

		case NO_ERROR:
#ifdef DEBUG
			std::cout << StringFormat("%s[INFO]Successfully acquired IP address table!!.", ServerTime::get_current_time()) << std::endl;
#endif // DEBUG

			break;

		default:
			break;
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		std::cerr << StringFormat("%s\033[31m[ERROR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str()) << std::endl;
	}
	if (m_ip_addr_table->dwNumEntries > 0) {
		char temp_local_ip_addr[100];
		for (m_i_count = 0; m_i_count < m_ip_addr_table->dwNumEntries; m_i_count++) {
			//std::cout << StringFormat("Address : %s", inet_ntoa(*(struct in_addr*)&ip_addr_table->table[i_count].dwAddr));
			inet_ntop(AF_INET, (struct in_addr*)&m_ip_addr_table->table[m_i_count].dwAddr, temp_local_ip_addr, sizeof(temp_local_ip_addr));
			if (m_i_count == 0) {
				m_local_ip_addr = temp_local_ip_addr;
			}
#ifdef DEBUG
			std::cout << StringFormat("%s[INFO]Address : %s", ServerTime::get_current_time(), m_local_ip_addr) << std::endl;
#endif // DEBUG
		}
		return true;
	}
	else{
		return false;
	}
}


bool SocketHandler::bind_tcp_listen_socket()
{
	std::cout << StringFormat("%s[INFO]Bind a TCP listening socket ...", ServerTime::get_current_time());
	try
	{
		int bind_result = bind(m_tcp_listen_socket, m_result->ai_addr, (int)m_result->ai_addrlen);
		if (bind_result == SOCKET_ERROR) {
			std::stringstream socket_error_msg;
			socket_error_msg << "An error occurred while binding the TCP listening socket. WSAGetLastError Code : " << std::to_string(WSAGetLastError());
			THROW_APP_EXCEPTION(socket_error_msg.str());
			
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return false;
	}
	
	std::cout << "\033[32m [OK] \033[m" << std::endl;
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

std::tuple<SOCKET, sockaddr_in> SocketHandler::accept_client_socket(int check_next_invalid_result, socklen_t client_sockaddr_in_length)
{
	if (check_next_invalid_result != -1) {
		try
		{
			//ClientHandler::Client new_client = ClientHandler::Client();
			SOCKET accept_client_socket_result;
			struct sockaddr_in accept_sockaddr_in_result;
			//socklen_t accept_socket_addr_in_length_rength_result = client_handler.get_connected_client_list()[check_n_invalid_socket_result].get_sockaddr_in_length();
			accept_client_socket_result = accept(m_tcp_listen_socket, (struct sockaddr*)&accept_sockaddr_in_result, &client_sockaddr_in_length);

			//acceptしたソケットがINVALID_SOCKETである場合
			if (accept_client_socket_result == INVALID_SOCKET) {
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
				return std::make_tuple(accept_client_socket_result, accept_sockaddr_in_result);
			}
		}
		catch (const exception::ExceptionBase& ex)
		{
			LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
			struct sockaddr_in error_addr_in = { 0 };
			return std::make_tuple(INVALID_SOCKET, error_addr_in);
		}

	}
}


void SocketHandler::disconnect_socket(SOCKET client_socket, std::string client_ip_address, int monitor_result)
{
	try
	{
		int close_socket_result = closesocket(client_socket);
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
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
	}
	LogHandler::logging_info(LogHandler::LogType::INFO, std::cout, StringFormat("Successfully initialized the information in the client list."));

}



bool SocketHandler::generate_tcp_listen_socket()
{
	//パラメータ設定
	if (!setting_tcp_listen_socket(INIT_FILE_PASS)) {
		return false;
	}

	//名前解決
	try
	{
		int get_addr_result = getaddrinfo(NULL, std::to_string(m_port).c_str(), &m_param, &m_result);
		if (get_addr_result != 0) {
			switch (get_addr_result)
			{
			case EAI_AGAIN:
				THROW_APP_EXCEPTION("名前解決の一時的なエラーが発生しました。");
				break;

			case EAI_BADFLAGS:
				THROW_APP_EXCEPTION("pHints パラメーターのai_flags メンバーに無効な値が指定されました。");
				break;
			
			case EAI_FAIL:
				THROW_APP_EXCEPTION("名前解決で回復不可能なエラーが発生しました。");
				break;

			case EAI_FAMILY:
				THROW_APP_EXCEPTION("pHints パラメーターのai_familyメンバーはサポートされていません。");
				break;

			case EAI_MEMORY:
				THROW_APP_EXCEPTION("メモリ割り当てエラーが発生しました。");
				break;

			case EAI_NONAME:
				THROW_APP_EXCEPTION("指定されたパラメーターの名前が解決されないか、 pNodeName パラメーターと pServiceName パラメーターが指定されませんでした。");
				break;

			case EAI_SERVICE:
				THROW_APP_EXCEPTION("pServiceName パラメーターは、pHints パラメーターの指定したai_socktype メンバーではサポートされていません。");
				break;

			case EAI_SOCKTYPE:
				THROW_APP_EXCEPTION("pHints パラメーターのai_socktype メンバーはサポートされていません。");
				break;

			default:
				THROW_APP_EXCEPTION("何らかの原因でgetaddrinfoの結果が正常ではありません。");
				break;
			}
		}

		std::cout << StringFormat("%s[INFO]Creating TCP listening socket...", ServerTime::get_current_time());
		//リッスンソケットの生成
		m_tcp_listen_socket = socket(m_result -> ai_family, m_result -> ai_socktype, m_result->ai_protocol);
		if (m_tcp_listen_socket == INVALID_SOCKET) {
			THROW_APP_EXCEPTION("The generated listening socket is not valid.");
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		freeaddrinfo(m_result);
		WSACleanup();
		return false;
	}
	std::cout << "\033[32m [OK] \033[m" << std::endl;

	bind_tcp_listen_socket();

	return true;
}



int SocketHandler::monitor_socket(const ClientHandler& client_handler)
{
	try
	{
		int select_result = 0;
		select_result = select(0, &m_fds, NULL, NULL, &m_timeout);

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
			if (FD_ISSET(m_tcp_listen_socket, &m_fds)) {
				std::cout << ServerTime::get_current_time() << "[INFO]There was a request to connect to the server from a new client." << std::endl;
				return REQUEST_ACCEPT_CLIENT_DETECTION;
			}
			else {
				//接続中のクライアントリストのどのクライアントからの通知かを判定する
				for (int detection_client_num = 0; detection_client_num < MAX_CONNECT_NUM; detection_client_num++) {
					if (client_handler.connected_client_list[detection_client_num].get_client_socket() != INVALID_SOCKET && FD_ISSET(client_handler.connected_client_list[detection_client_num].get_client_socket(), &m_fds)) {
						//std::cout << ServerTime::get_current_time() << "[REQUEST]接続中のクライアント(client_net_info_list[" << detection_client_num << "])から要求がありました" << std::endl;
						return detection_client_num;
					}
				}
			}
			break;
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		return SOCKET_ERROR;
	}

	return 0;
}

bool SocketHandler::open_tcp_listen_socket()
{
	//tcp_listen_socketのオープンに失敗した場合
	if (!generate_tcp_listen_socket()) {
		//std::cerr << StringFormat("%s[ERROR]tcp_listen_socketのオープンに失敗しました。") << std::endl;
		return false;
	}


	std::cout << StringFormat("%s[INFO]Open a TCP listening socket...", ServerTime::get_current_time());
	try
	{
		if (listen(m_tcp_listen_socket, SOMAXCONN) == SOCKET_ERROR) {

			freeaddrinfo(m_result);
			closesocket(m_tcp_listen_socket);
			WSACleanup();
			THROW_APP_EXCEPTION("Tcp Listen socket has been SOCKET_ERROR for some reason.");

		}
		else
		{
			std::cout << "\033[32m [OK] \033[m" << std::endl;
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
	}
	return true;
}

/**********************************************************************************
 * @fn	  check_client_socket(SOCKET socket)
 * socketが閉じているか確認する。
 * @brief socketが閉じているか確認する。
 * @param [in] SOCKET socket
 * @return
 **********************************************************************************/
bool SocketHandler::check_valid_socket(SOCKET request_socket)
{
	if (request_socket == INVALID_SOCKET)
		return false;
	return true;
}

