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
SocketHandler::SocketHandler() : 
	_tcp_listen_socket{INVALID_SOCKET},
	_udp_bloadcast_tx_socket{ INVALID_SOCKET },
	_udp_multicast_tx_socket{INVALID_SOCKET}
{
	ZeroMemory(&m_param_tcp, sizeof(m_param_tcp));

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
		spdlog::error(ex.what_for_str());
	}

	//ファイルディスクリプタキャッチ用のfdsを初期化
	FD_ZERO(&m_fds);
	FD_ZERO(&m_fd_read);

	//select関数のタイムアウト初期設定(tv_sec=秒の指定　tv_usec=マイクロ秒の指定)
	//デフォルトコンストラクタでは500マイクロ秒 * 1000　⇒ 500ミリ秒に設定
	_timeout.tv_sec = 0;
	_timeout.tv_usec = 500 * 1000;

	//リッスンソケットの初期化
	_tcp_listen_socket = INVALID_SOCKET;

	//UDPブロードキャスト送信用ソケットの初期化
	_udp_bloadcast_tx_socket = INVALID_SOCKET;

	//UDPマルチキャスト送信用ソケット
	_udp_multicast_tx_socket = INVALID_SOCKET;

	//ソケット初期化用の変数の初期化
	_init_sockaddr_in.sin_family = AF_INET;
	_init_sockaddr_in.sin_port = 0;
	_init_sockaddr_in.sin_addr.S_un.S_addr = INADDR_ANY;


	
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
	ZeroMemory(&m_param_tcp, sizeof(m_param_tcp));
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
		spdlog::error(ex.what_for_str());
	}

	//ファイルディスクリプタキャッチ用のfdsを初期化
	FD_ZERO(&m_fds);
	FD_ZERO(&m_fd_read);


	//select関数のタイムアウト初期設定(tv_sec=秒の指定　tv_usec=マイクロ秒の指定)
	_timeout.tv_sec = tv_sec;
	_timeout.tv_usec = tv_usec;

	//リッスンソケットの初期化
	_tcp_listen_socket = INVALID_SOCKET;


	_init_sockaddr_in.sin_family = AF_INET;
	_init_sockaddr_in.sin_port = 50000;
	_init_sockaddr_in.sin_addr.S_un.S_addr = INADDR_ANY;

}

//デストラクタ
SocketHandler::~SocketHandler()
{
	freeaddrinfo(m_result_tcp);
	closesocket(get_listen_socket());
	closesocket(get_udp_multicast_tx_socket());
	WSACleanup();
}


//ここのtry構文見直す
//設定値の読み込み
bool SocketHandler::setting_tcp_listen_socket_param(const std::string& file_pass)
{

	//iniファイルの読み込み
	ini::IniFile socket_param_setting_ini;
	try
	{
		socket_param_setting_ini.load(file_pass);
	}
	catch (const exception::ExceptionBase& ex)
	{
		spdlog::error(ex.what_for_str());
	}
	if (this->setting_tcp_flags(socket_param_setting_ini) == false) {
		return false;
	}
	if (this->setting_tcp_family(socket_param_setting_ini) == false) {
		return false;
	}
	if (this->setting_tcp_socktype(socket_param_setting_ini) == false) {
		return false;
	}
	if (this->setting_tcp_protocol(socket_param_setting_ini) == false) {
		return false;
	}
	if (this->setting_tcp_port(socket_param_setting_ini) == false) {
		return false;
	}
	return true;
}

bool SocketHandler::setting_multicast_tx_socket_param(const std::string& file_pass)
{
	//iniファイルの読み込み
	ini::IniFile multicast_socket_setting_ini;
	try
	{
		multicast_socket_setting_ini.load(file_pass);
	}
	catch (const exception::ExceptionBase& ex)
	{
		spdlog::error(ex.what_for_str());
	}

	if (this->setting_multicast_tx_sock_af(multicast_socket_setting_ini) == false) {
		return false;
	}
	if (this->setting_multicast_tx_sock_type(multicast_socket_setting_ini) == false) {
		return false;
	}
	if (this->setting_multicast_tx_sock_protocol(multicast_socket_setting_ini) == false) {
		return false;
	}
	if (this->setting_multicast_tx_sin_family(multicast_socket_setting_ini) == false) {
		return false;
	}
	if (this->setting_multicast_tx_sin_port(multicast_socket_setting_ini) == false) {
		return false;
	}
	if (this->setting_multicast_tx_sin_address(multicast_socket_setting_ini) == false) {
		return false;
	}
	return true;


}

bool SocketHandler::setting_bloadcast_tx_socket_param(const std::string& file_pass)
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
		spdlog::error(ex.what_for_str());
	}
}


void SocketHandler::setting_select_info_to_listen_socket()
{
	FD_ZERO(&this->m_fd_read);
	FD_SET(this->_tcp_listen_socket, &this->m_fd_read);
	

}


void SocketHandler::bind_select_info_to_socket(const SOCKET& socket, fd_set* fd)
{
	FD_SET(socket, fd);
}

void SocketHandler::copy_fd_read_to_fds()
{
	memcpy(&this->m_fds, &this->m_fd_read, sizeof(fd_set));
}

void SocketHandler::copy_fd_read_to_fds(fd_set* fds, fd_set* fd)
{
	memcpy(fds, fd, sizeof(fd_set));
}




bool SocketHandler::generate_udp_multicast_tx_socket()
{
	if (!setting_multicast_tx_socket_param(INIT_FILE_PASS)) {
		spdlog::error("The socket configuration for multicast transmission was not completed successfully and will be terminated.");
		return false;
	}

	try
	{
		int get_addr_result_msg = getaddrinfo(NULL, std::to_string(_multicast_tx_addr.sin_port).c_str(), &m_multicast_tx_param, &m_result_multicast);
		this->_udp_multicast_tx_socket = socket(m_multicast_tx_param.ai_family, m_multicast_tx_param.ai_socktype, 0);
		if (this->_udp_multicast_tx_socket == INVALID_SOCKET) {
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
				break;
			}
		
		
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		spdlog::error(ex.what_for_str());
		return false;
	}
	
	//setsocloptの設定
	try
	{
		int multicast_if_set_sockopt_result = setsockopt(this->_udp_multicast_tx_socket, 
											IPPROTO_IP, IP_MULTICAST_IF,
											this->_local_ip_addr.c_str(), 
											sizeof(this->_local_ip_addr.c_str()));
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
		spdlog::error(ex.what_for_str());
		return false;
		
		//TODOネットワークインタフェースの数で戻り値を変えたい
	}
	
	try
	{
		int multicast_ttl_set_sockopt_result = setsockopt(_udp_multicast_tx_socket,
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
		spdlog::error(ex.what_for_str());
		return false;
	}
	spdlog::info("server generate udp muluticast tx socket:{:d}.", _udp_multicast_tx_socket);
	return true;
}

bool SocketHandler::generate_udp_bloadcast_tx_socket()
{
	return false;
}

bool SocketHandler::add_connect_socket(const SOCKET socket)
{
	try
	{
		if (this->_connect_sockets.size() >= MAX_CONNECT_NUM) {
			THROW_APP_EXCEPTION("The maximum number of client connections has been reached and the connection will be abandoned");
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		spdlog::error(ex.what_for_str());
		return false;
	}

	this->_connect_sockets.push_back(socket);
	return true;
}

bool SocketHandler::delete_connect_socket(const SOCKET socket)
{
	if (this->_connect_sockets.empty() == true) {
		spdlog::error("Client list is empty.");
		return false;
	}
	else {
		auto it = std::find(this->_connect_sockets.begin(), this->_connect_sockets.end(), socket);
		this->_connect_sockets.erase(it);
		return true;
	}
}

bool SocketHandler::setting_tcp_flags(ini::IniFile tcp_listen_socket_param_setting_ini)
{
	//フラッグの設定
	std::string temp_flag = tcp_listen_socket_param_setting_ini["SocketSetting"]["flags"].as<std::string>();
	if (trans_flags(temp_flag, &m_param_tcp) == false) {
		return false;
	}
	else {
		return true;
	}

}

bool SocketHandler::setting_tcp_family(ini::IniFile tcp_listen_socket_param_setting_ini)
{
	//iniファイルの各セクションからキーを読み込む
	//アドレスファミリの設定(ipv4 or ipv6)
	std::string temp_family = tcp_listen_socket_param_setting_ini["SocketSetting"]["family"].as<std::string>();
	if (trans_family(temp_family, &m_param_tcp) == false) {
		return false;
	}
	else {
		return true;
	}
	
}

bool SocketHandler::setting_tcp_socktype(ini::IniFile tcp_listen_socket_param_setting_ini)
{
	//ソケット型の設定
	std::string temp_socket_type = tcp_listen_socket_param_setting_ini["SocketSetting"]["socktype"].as<std::string>();
	if (trans_socktype(temp_socket_type, &m_param_tcp) == false) {
		return false;
}
	else {
		return true;
	}
}

bool SocketHandler::setting_tcp_protocol(ini::IniFile tcp_listen_socket_param_setting_ini)
{
	//プロトコルの設定
	std::string temp_protocol = tcp_listen_socket_param_setting_ini["SocketSetting"]["protocol"].as<std::string>();
	//std::equal(temp_family.begin(), temp_family.end(), "AF_INET")
	//プロトコルをtcpに設定する場合
	if (trans_protocol(temp_protocol, &m_param_tcp) == false) {
		return false;
	}
	else {
		return true;
	}
}

bool SocketHandler::setting_tcp_port(ini::IniFile tcp_listen_socket_param_setting_ini)
{
	//ポートの設定
	try
	{
		int temp_port = tcp_listen_socket_param_setting_ini["SocketSetting"]["message_port"].as<int>();

		if (temp_port < 0 || temp_port > 65535) {

			THROW_APP_EXCEPTION("The port number is set out of range. Reset to the range of 0~65535.");
		}
		_port = temp_port;
}
	catch (const exception::ExceptionBase& ex)
	{
		spdlog::error(ex.what_for_str());

		return false;
	}
	return true;
}

bool SocketHandler::setting_multicast_tx_sock_af(ini::IniFile socket_param_setting_ini)
{
	std::string temp_af = socket_param_setting_ini["MulticastTX_SocketSetting"]["family"].as<std::string>();
	if (trans_family(temp_af, &m_multicast_tx_param) == false) {
		return false;
	}
	return true;
}


bool SocketHandler::setting_multicast_tx_sock_type(ini::IniFile socket_param_setting_ini)
{
	//ソケットタイプの設定
	std::string temp_socktype = socket_param_setting_ini["MulticastTX_SocketSetting"]["type"].as<std::string>();
	if (trans_socktype(temp_socktype, &m_multicast_tx_param) == false) {
		return false;
	}
	else {
		return true;
	}

}

bool SocketHandler::setting_multicast_tx_sock_protocol(ini::IniFile socket_param_setting_ini)
{
	int temp_protocol = socket_param_setting_ini["MulticastTX_SocketSetting"]["protocol"].as<int>();
	if (trans_protocol(temp_protocol, &m_multicast_tx_param) == false) {
		return false;
	
	}
	return true;
}

bool SocketHandler::setting_multicast_tx_sin_family(ini::IniFile socket_param_setting_ini)
{
	//アドレスファミリの設定(socket関数の引数設定)
	std::string temp_af = socket_param_setting_ini["MulticastTX_SocketSetting"]["sin_family"].as<std::string>();

	if (std::equal(temp_af.begin(), temp_af.end(), "AF_UNSPEC")) {
		_multicast_tx_addr.sin_family = AF_UNSPEC;
	}
#ifdef DEBUG
	spdlog::debug("multicast param：sin_family = AF_UNSPEC");
#endif // DEBUG

	if (std::equal(temp_af.begin(), temp_af.end(), "AF_INET")) {
		_multicast_tx_addr.sin_family = AF_INET;
#ifdef DEBUG
		spdlog::debug("multicast param：sin_family = AF_INET");
#endif // DEBUG
	}
	else if (std::equal(temp_af.begin(), temp_af.end(), "AF_IPX")) {
		_multicast_tx_addr.sin_family = AF_IPX;
#ifdef DEBUG
		spdlog::debug("multicast param：sin_family = AF_IPX");
#endif // DEBUG
	}
	else if (std::equal(temp_af.begin(), temp_af.end(), "AF_APPLETALK")) {
		_multicast_tx_addr.sin_family = AF_APPLETALK;
#ifdef DEBUG
		spdlog::debug("multicast param：sin_family = AF_APPLETALK");
#endif // DEBUG
	}
	else if (std::equal(temp_af.begin(), temp_af.end(), "AF_NETBIOS")) {
		_multicast_tx_addr.sin_family = AF_NETBIOS;
#ifdef DEBUG
		spdlog::debug("multicast param：sin_family = AF_NETBIOS");
#endif // DEBUG
	}
	else if (std::equal(temp_af.begin(), temp_af.end(), "AF_INET6")) {
		_multicast_tx_addr.sin_family = AF_INET6;
#ifdef DEBUG
		spdlog::debug("multicast param：sin_family = AF_INET6");
#endif // DEBUG

	}
	else if (std::equal(temp_af.begin(), temp_af.end(), "AF_IRDA")) {
		_multicast_tx_addr.sin_family = AF_IRDA;
#ifdef DEBUG
		spdlog::debug("multicast param：sin_family = AF_IRDA");
#endif // DEBUG
	}
	else if (std::equal(temp_af.begin(), temp_af.end(), "AF_BTH")) {
		_multicast_tx_addr.sin_family = AF_BTH;
#ifdef DEBUG
		spdlog::debug("multicast param：sin_family = AF_BTH");
#endif // DEBUG
	}
	else {
		spdlog::error("The multicast transmit socket settings were not set correctly(sin_family). Exit the program.");
		return false;
	}
	return true;
}



bool SocketHandler::setting_multicast_tx_sin_port(ini::IniFile socket_param_setting_ini)
{
	//マルチキャストのポートを設定する。
	try
	{
		int temp_port = socket_param_setting_ini["MulticastTX_SocketSetting"]["sin_port"].as<int>();
		if (temp_port < 0 || temp_port > 65535) {

			THROW_APP_EXCEPTION("The port number is set out of range. Reset to the range of 0 - 65535.");
		}

		if (temp_port == _port) {
			THROW_APP_EXCEPTION("There is a conflict between the application port used for TCP and the application port used for UDP multicast. Please reset the settings.");
		}
		_multicast_tx_addr.sin_port = temp_port;
#ifdef DEBUG
		spdlog::debug("The send port for multicast is now set to {:d}", m_multicast_tx_addr.sin_port);
#endif // DEBUG

	}
	catch (const exception::ExceptionBase& ex)
	{
		spdlog::error(ex.what_for_str());
		return false;
	}
	return true;
}

bool SocketHandler::setting_multicast_tx_sin_address(ini::IniFile socket_param_setting_ini)
{
	std::string temp_multicast_addr = socket_param_setting_ini["MulticastTX_SocketSetting"]["sin_address"].as<std::string>();

	//inet_addrの設定
	try
	{
		int inet_pton_result = inet_pton(m_multicast_tx_param.ai_family, temp_multicast_addr.c_str(), &_multicast_tx_addr.sin_addr);
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
#ifdef DEBUG
			spdlog::debug("The multicast transmission address has been set successfully (address: {:s})", temp_multicast_addr);
#endif // DEBUG
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		spdlog::error(ex.what_for_str());
	}


	return true;
}


std::string SocketHandler::obtain_local_ip_addr()
{
	DWORD get_ip_addr_table_result;
	//IPアドレステーブルの取得
	try
	{
		if (GetIpAddrTable(NULL, &_dw_size, 0) == ERROR_INSUFFICIENT_BUFFER) {
			_ip_addr_table = (MIB_IPADDRTABLE*)malloc(_dw_size);
		}

		get_ip_addr_table_result = GetIpAddrTable(_ip_addr_table, &_dw_size, 0);
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
			break;
		default:
			THROW_APP_EXCEPTION("ERROR CASE:Other(Use FormatMessage to obtain the message string for the returned error.)");
			break;
		}

	}
	catch (const exception::ExceptionBase& ex)
	{
		spdlog::error(ex.what_for_str());

	}


	//IPアドレステーブルの結果を格納
	try
	{
		DWORD dw_ret_val_result = GetIpAddrTable(_ip_addr_table, &_dw_size, 0);
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

			break;

		default:
			break;
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		spdlog::error(ex.what_for_str());
	}
	if (_ip_addr_table->dwNumEntries > 0) {
		char temp_local_ip_addr[100];
		for (_i_count = 0; _i_count < _ip_addr_table->dwNumEntries; _i_count++) {
			//std::cout << StringFormat("Address : %s", inet_ntoa(*(struct in_addr*)&ip_addr_table->table[i_count].dwAddr));
			inet_ntop(AF_INET, (struct in_addr*)&_ip_addr_table->table[_i_count].dwAddr, temp_local_ip_addr, sizeof(temp_local_ip_addr));
			if (_i_count == 0) {
				_local_ip_addr = temp_local_ip_addr;
			}
		}
		spdlog::info("server-ip, success, obtain server ip address.");
	}
	else{
		spdlog::error("server-ip, failed, failed obtain server ip address.");
	}

	return _local_ip_addr;
}


bool SocketHandler::bind_tcp_listen_socket(SOCKET* listen_socket, struct addrinfo* result)
{
	try
	{
		int bind_result = bind(*listen_socket, result->ai_addr, (int)result->ai_addrlen);
		if (bind_result == SOCKET_ERROR) {
			std::stringstream socket_error_msg;
			socket_error_msg << "An error occurred while binding the TCP listening socket. WSAGetLastError Code : " << std::to_string(WSAGetLastError());
			THROW_APP_EXCEPTION(socket_error_msg.str());
			
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		spdlog::error(ex.what_for_str());
		return false;
	}

	
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

std::tuple<SOCKET, sockaddr_in> SocketHandler::accept_client_socket(socklen_t client_sockaddr_in_length)
{
	try
	{
		SOCKET accept_client_socket_result;
		struct sockaddr_in accept_sockaddr_in_result;
		accept_client_socket_result = accept(_tcp_listen_socket, (struct sockaddr*)&accept_sockaddr_in_result, &client_sockaddr_in_length);

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
		spdlog::error(ex.what_for_str());
		struct sockaddr_in error_addr_in = { 0 };
		return std::make_tuple(INVALID_SOCKET, error_addr_in);
	}
}


void SocketHandler::disconnect_socket(const SOCKET client_socket)
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
		else {
			spdlog::info("client socket({:d}) is closed.", client_socket);
		}

	}
	catch (const exception::ExceptionBase& ex)
	{
		spdlog::error(ex.what_for_str());
	}


}



bool SocketHandler::generate_tcp_listen_socket()
{
	//パラメータ設定
	if (!setting_tcp_listen_socket_param(INIT_FILE_PASS)) {
		return false;
	}

	//名前解決
	try
	{
		int get_addr_result_msg = getaddrinfo(NULL, std::to_string(_port).c_str(), &m_param_tcp, &m_result_tcp);
		if (get_addr_result_msg != 0) {
			switch (get_addr_result_msg)
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

		//リッスンソケットの生成
		_tcp_listen_socket = socket(m_result_tcp -> ai_family, m_result_tcp -> ai_socktype, m_result_tcp->ai_protocol);
		if (_tcp_listen_socket == INVALID_SOCKET) {
			THROW_APP_EXCEPTION("The generated listening socket for message is not valid.");
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		spdlog::error(ex.what_for_str());
		freeaddrinfo(m_result_tcp);
		WSACleanup();
		return false;
	}
	spdlog::debug("server generated listen socket({:d}).", _tcp_listen_socket);

	bool bind_result = bind_tcp_listen_socket(&_tcp_listen_socket, m_result_tcp);
	if (bind_result == true) {
		spdlog::debug("server bind setting on listen socket({:d}).", _tcp_listen_socket);
	}
	else {
		spdlog::error("server can't bind setting on listen socket({:d}).", _tcp_listen_socket);
		return false;
	}

	return true;
}


bool SocketHandler::trans_flags(const std::string& flag_str, struct addrinfo* param)
{
	std::string return_flag = "";
	if (std::equal(flag_str.begin(), flag_str.end(), "AI_PASSIVE")) {
		param->ai_flags = AI_PASSIVE;
#ifdef DEBUG
		spdlog::debug("param：ai_flags is set to AI_PASSIVE.");
#endif // DEBUG
	}
	else if (std::equal(flag_str.begin(), flag_str.end(), "AI_CANONNAME")) {
		param->ai_flags = AI_CANONNAME;
#ifdef DEBUG
		spdlog::debug("param：ai_flags is set to AI_CANONNAME.");
#endif // DEBUG
	}
	else if (std::equal(flag_str.begin(), flag_str.end(), "AI_NUMERICHOST")) {
		param->ai_flags = AI_NUMERICHOST;
#ifdef DEBUG
		spdlog::debug("param：ai_flags is set to AI_NUMERICHOST.");
#endif // DEBUG
	}
	else if (std::equal(flag_str.begin(), flag_str.end(), "AI_ALL")) {
		param->ai_flags = AI_ALL;
#ifdef DEBUG
		spdlog::debug("param：ai_flags is set to AI_ALL.");
#endif // DEBUG

	}
	else if (std::equal(flag_str.begin(), flag_str.end(), "AI_ADDRCONFIG")) {
		param->ai_flags = AI_ADDRCONFIG;
#ifdef DEBUG
		spdlog::debug("param：ai_flags is set to AI_ADDRCONFIG.");
#endif // DEBUG	
	}
	else if (std::equal(flag_str.begin(), flag_str.end(), "AI_V4MAPPED")) {
		param->ai_flags = AI_V4MAPPED;
#ifdef DEBUG
		spdlog::debug("param：ai_flags is set to AI_V4MAPPED.");
#endif // DEBUG
	}
	else {
		spdlog::error("Failed to set ai_flags.");
		return false;
	}
	return true;
}

bool SocketHandler::trans_family(const std::string& family_str, addrinfo* param)
{
	if (std::equal(family_str.begin(), family_str.end(), "AF_INET")) {
		param->ai_family = AF_INET;
#ifdef DEBUG
		spdlog::debug("param：ai_family is set to AF_INET.");
#endif // DEBUG

	}
	//通信プロトコルをipv6に設定する場合
	else if (std::equal(family_str.begin(), family_str.end(), "AF_INET6")) {
		param->ai_family = AF_INET6;
#ifdef DEBUG
		spdlog::debug("param：ai_family is set to AF_INET6.");
#endif // DEBUG

	}
	//通信プロトコルがaf_unspecに設定された場合(ipv4とipv6に対応)
	//通信プロトコルがどちらにも設定していない場合及び読み取れなかった場合(af_unspecに設定)
	else if (std::equal(family_str.begin(), family_str.end(), "AF_UNSPEC")) {
		param->ai_family = AF_UNSPEC;
#ifdef DEBUG
		spdlog::debug("param：ai_family is set to AF_UNSPEC.");
#endif // DEBUG
	}
	else {
		spdlog::error("Failed to set ai_family.");
		return false;
	}
	return true;
}

bool SocketHandler::trans_socktype(const std::string& socktype_str, addrinfo* param)
{
	if (std::equal(socktype_str.begin(), socktype_str.end(), "SOCK_STREAM")) {
		param->ai_socktype = SOCK_STREAM;
#ifdef DEBUG
		spdlog::debug("param：ai_socktype is set to SOCK_STREAM.");
#endif // DEBUG		
	}
	else if (std::equal(socktype_str.begin(), socktype_str.end(), "SOCK_DGRAM")) {
		param->ai_socktype = SOCK_DGRAM;
#ifdef DEBUG
		spdlog::debug("param：ai_socktype is set to SOCK_DGRAM.");
#endif // DEBUG
	}
	else if (std::equal(socktype_str.begin(), socktype_str.end(), "SOCK_RAW")) {
		param->ai_socktype = SOCK_RAW;
#ifdef DEBUG
		spdlog::debug("param：ai_socktype is set to SOCK_RAW.");
#endif // DEBUG
	}
	else if (std::equal(socktype_str.begin(), socktype_str.end(), "SOCK_RDM")) {
		param->ai_socktype = SOCK_RDM;
#ifdef DEBUG
		spdlog::debug("param：ai_socktype is set to SOCK_RDM.");
#endif // DEBUG

	}
	else if (std::equal(socktype_str.begin(), socktype_str.end(), "SOCK_SEQPACKET")) {
		param->ai_socktype = SOCK_SEQPACKET;
#ifdef DEBUG
		spdlog::debug("param：ai_socktype is set to SOCK_SEQPACKET.");

#endif // DEBUG
	}
	else {
		spdlog::error("Failed to set ai_socktype.");
		return false;
	}
	return true;
}

bool SocketHandler::trans_protocol(const std::string& protocol_str, addrinfo* param)
{
	if ((std::equal(protocol_str.begin(), protocol_str.end(), "IPPROTO_TCP")) && ((param->ai_family == AF_INET || param->ai_family == AF_INET6) && param->ai_socktype == SOCK_STREAM)) {
		param->ai_protocol = IPPROTO_TCP;
#ifdef DEBUG
		spdlog::debug("param：ai_protocol is set to IPPROTO_TCP.");
#endif // DEBUG
	}

	//プロトコルをudpに設定する場合
	else if ((std::equal(protocol_str.begin(), protocol_str.end(), "IPPROTO_UDP")) && ((param->ai_family == AF_INET || param->ai_family == AF_INET6) && param->ai_socktype == SOCK_DGRAM)) {
		param->ai_protocol = IPPROTO_UDP;
#ifdef DEBUG
		spdlog::debug("param：ai_protocol is set to IPPROTO_UDP.");
#endif // DEBUG
	}
	//プロトコルをpgmに設定する場合
	else if ((std::equal(protocol_str.begin(), protocol_str.end(), "IPPROTO_PGM")) && param->ai_family == AF_INET && param->ai_socktype == SOCK_RDM) {
		param->ai_protocol = IPPROTO_PGM;
#ifdef DEBUG
		spdlog::debug("param：ai_protocol is set to IPPROTO_PGM.");
#endif // DEBUG
	}
	else {
		spdlog::error("Failed to set ai_protocol.");
		return false;
	}
	return true;
}

bool SocketHandler::trans_protocol(const int protocol_int, addrinfo* param)
{

	//protocol = ICMPの場合
	if ((protocol_int == 1) && ((param->ai_family == AF_INET || param->ai_family == AF_INET6 || param->ai_family == AF_UNSPEC) && (param->ai_socktype == SOCK_RAW || param->ai_family == 0))) {
		param->ai_protocol = IPPROTO_ICMP;
#ifdef DEBUG
		spdlog::debug("param：ai_protocol is set to IPPROTO_ICMP.");
#endif // DEBUG
	}

	//protocol = IGMPの場合
	if ((protocol_int == 2) && ((param->ai_family == AF_INET || param->ai_family == AF_INET6 || param->ai_family == AF_UNSPEC) && (param->ai_socktype == SOCK_RAW || param->ai_family == 0))) {
		param->ai_protocol = IPPROTO_IGMP;
#ifdef DEBUG
		spdlog::debug("param：ai_protocol is set to IPPROTO_IGMP.");
#endif // DEBUG
	}
	if ((protocol_int == 6) && ((param->ai_family == AF_INET || param->ai_family == AF_INET6) && param->ai_socktype == SOCK_STREAM)) {
		param->ai_protocol = IPPROTO_TCP;
#ifdef DEBUG
		spdlog::debug("param：ai_protocol is set to IPPROTO_TCP.");
#endif // DEBUG
	}

	//プロトコルをudpに設定する場合
	else if ((protocol_int == 17) && ((param->ai_family == AF_INET || param->ai_family == AF_INET6) && param->ai_socktype == SOCK_DGRAM)) {
		param->ai_protocol = IPPROTO_UDP;
#ifdef DEBUG
		spdlog::debug("param：ai_protocol is set to IPPROTO_UDP.");
#endif // DEBUG
	}

	//protocol = RFCOMMの場合
	else if ((protocol_int == 58) && ((param->ai_family == AF_INET || param->ai_family == AF_INET6 || param->ai_family == AF_UNSPEC) && (param->ai_socktype == SOCK_RAW || param->ai_family == 0))) {
		param->ai_protocol = IPPROTO_ICMPV6;
#ifdef DEBUG
		spdlog::debug("param：ai_protocol is set to IPPROTO_ICMPV6.");
#endif // DEBUG
	}
	//プロトコルをpgmに設定する場合
	else if ((protocol_int == 113) && param->ai_family == AF_INET && param->ai_socktype == SOCK_RDM) {
		param->ai_protocol = IPPROTO_PGM;
#ifdef DEBUG
		spdlog::debug("param：ai_protocol is set to IPPROTO_PGM.");
#endif // DEBUG	
	}
	else if (protocol_int == 0) {
		//param->ai_protocol == IPPROTO_UDP;
	}
	else {
		spdlog::error("Failed to set ai_protocol.");
		return false;
	}
	return true;
}




//int SocketHandler::monitor_socket_descriptor(const std::vector<ClientHandler::Client>& client_list)
//{
//	try
//	{
//		int select_result = 0;
//		select_result = select(0, &m_fds, NULL, NULL, &_timeout);
//
//		switch (select_result)
//		{
//			//fd_set構造体に含まれているソケット ハンドルの合計数、期限切れの場合
//		case 0:
//			//THROW_APP_EXCEPTION("CASE:SELECT FUNCTION ERROR(fd_set構造体に含まれているソケット ハンドルの合計数、期限切れです。)");
//			return SELECT_TIME_OUT;
//			break;
//
//			//エラーが発生した場合
//		case SOCKET_ERROR:
//			switch (WSAGetLastError())
//			{
//			case WSANOTINITIALISED:
//				THROW_APP_EXCEPTION("WSA ERROR CASE:WSANOTINITIALISED(この関数を使用する前に 、WSAStartup 呼び出しが成功する必要があります。)");
//				break;
//			case WSAEFAULT:
//				THROW_APP_EXCEPTION("WSA ERROR CASE:WSAEFAULT(Windows ソケットの実装では、内部操作に必要なリソースを割り当てることができませんでした。または、 readfds、 writefds、 exceptfds、または timeval パラメーターがユーザー アドレス空間の一部ではありません。)");
//				break;
//			case ENETDOWN:
//				THROW_APP_EXCEPTION("WSA ERROR CASE:ENETDOWN(ネットワーク サブシステムが失敗しました。)");
//				break;
//			case WSAEINVAL:
//				THROW_APP_EXCEPTION("WSA ERROR CASE:WSAEINVAL(タイムアウト値が無効であるか、3 つの記述子パラメーターがすべて null でした。)");
//				break;
//			case WSAEINTR:
//				THROW_APP_EXCEPTION("WSA ERROR CASE:WSAEINTR(WSACancelBlockingCall を使用して、Windows ソケット 1.1 呼び出しのブロックが取り消されました。)");
//				break;
//			case WSAEINPROGRESS:
//				THROW_APP_EXCEPTION("WSA ERROR CASE:WSAEINPROGRESS(ブロック中の Windows ソケット 1.1 呼び出しが進行中であるか、サービス プロバイダーがコールバック関数を処理しています。)");
//				break;
//			case WSAENOTSOCK:
//				THROW_APP_EXCEPTION("WSA ERROR CASE:WSAENOTSOCK(記述子セットの 1 つに、ソケットではないエントリが含まれています。)");
//				break;
//			default:
//				break;
//			}
//			break;
//
//
//
//			//ディスクリプションに変化が起こった場合
//		default:
//			if (FD_ISSET(_tcp_listen_socket, &m_fds)) {
//				return REQUEST_ACCEPT_CLIENT_DETECTION;
//			}
//			else {
//				//接続中のクライアントリストのどのクライアントからの通知かを判定する
//				for (int detection_client_num = 0; detection_client_num < client_list.size(); detection_client_num++) {
//					if (client_list[detection_client_num].socket != INVALID_SOCKET && FD_ISSET(client_list[detection_client_num].socket, &m_fds)) {
//						return detection_client_num;
//					}
//				}
//			}
//			break;
//		}
//	}
//	catch (const exception::ExceptionBase& ex)
//	{
//		spdlog::error(ex.what_for_str());
//		return SOCKET_ERROR;
//	}
//
//}

void SocketHandler::monitor_socket_descriptor(const std::vector<SOCKET>& socket_list, std::function<void(SOCKET)> callback)
{
	try {
		int select_result = select(0, &m_fds, NULL, NULL, &_timeout);

		switch (select_result) {
		case 0:
			return; // タイムアウト
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
			return;
		default:
			if (FD_ISSET(_tcp_listen_socket, &m_fds)) {
				callback(_tcp_listen_socket); // 新しい接続リクエスト
			}
			for (const auto& socket : socket_list) {
				if (socket != INVALID_SOCKET && FD_ISSET(socket, &m_fds)) {
					callback(socket); // 変化のあったソケットを通知
				}
			}
			break;
		}
	}
	catch (const exception::ExceptionBase& ex) {
		spdlog::error(ex.what_for_str());
	}
}


bool SocketHandler::open_tcp_listen_socket()
{
	try
	{
		//tcp_listen_socketのオープンに失敗した場合
		if (!generate_tcp_listen_socket()) {
			THROW_APP_EXCEPTION("Failed to generate TCP listen socket for message.");
			return false;
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		spdlog::error(ex.what_for_str());
	}

	try
	{
		if (listen(_tcp_listen_socket, SOMAXCONN) == SOCKET_ERROR) {

			freeaddrinfo(m_result_tcp);
			closesocket(_tcp_listen_socket);
			WSACleanup();
			THROW_APP_EXCEPTION("Tcp Listen socket has been SOCKET_ERROR for some reason.");

		}
		else
		{
			spdlog::info("server open listen socket:{:d}.", _tcp_listen_socket);
		}
	}
	catch (const exception::ExceptionBase& ex)
	{
		spdlog::error(ex.what_for_str());
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

