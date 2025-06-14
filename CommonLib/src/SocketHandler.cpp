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
	_result_tcp{ nullptr },
	_result_multicast{ nullptr },
	_result_bloadcast{ nullptr },
	_tcp_socket_param{},
	_multicast_tx_param{},
	_bloadcast_tx_param{},
	_wsa_data{},
	_fds{},
	_fd_read{},
	_timeout{0, 500*1000},
	_tcp_listen_socket{ INVALID_SOCKET },
	_udp_bloadcast_tx_socket{ INVALID_SOCKET },
	_udp_multicast_tx_socket{ INVALID_SOCKET },
	_port{ 0 },
	_multicast_tx_addr{},
	_bloadcast_tx_addr{},
	_local_ip_addr{""},
	_ttl{ 0 },
	setting_file_pass{ ""}
{
	try
	{
		ZeroMemory(&_tcp_socket_param, sizeof(_tcp_socket_param));
		int init_wsa_result = WSAStartup(MAKEWORD(2, 2), &_wsa_data);
		if (init_wsa_result == false) {
			switch (init_wsa_result) {
			case WSASYSNOTREADY:
				throw std::runtime_error("CASE:WSASYSNOTREADY(ネットワークサブシステムがネットワークへの接続を準備出来ていません)");
				break;

			case WSAVERNOTSUPPORTED:
				throw std::runtime_error("CASE:WSAVERNOTSUPPORTED:(要求された Windows ソケットのサポートのバージョンは、この特定の Windows ソケットの実装では提供されません)");
				break;

			case WSAEINPROGRESS:
				throw std::runtime_error("CASE:WSAEINPROGRESS(Windows ソケット 1.1 のブロック操作が進行中です)");
				break;

			case WSAEPROCLIM:
				throw std::runtime_error("CASE:WSAEPROCLIM(Windows ソケットの実装でサポートされるタスクの数の制限に達しました)");
				break;

			case WSAEFAULT:
				throw std::runtime_error("CASE:WSAEFAULT(lpWSAData パラメーターが有効なポインターではありません)");
				break;
			}
		}
		//ファイルディスクリプタキャッチ用のfdsを初期化
		FD_ZERO(&_fds);
		FD_ZERO(&_fd_read);

		if ((setting_tcp_listen_socket_param() == false || (setting_multicast_tx_socket_param()) == false)) {
			LOG_ERROR("The socket configuration for TCP listening and multicast transmission was not completed successfully and will be terminated.");
			return;
		}
	
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
		return;
	}
	catch (const std::exception& e)
	{
		LOG_ERROR(e.what());
		return;
	}

}

SocketHandler::SocketHandler(const std::string& file_pass):
	_result_tcp{ nullptr },
	_result_multicast{ nullptr },
	_result_bloadcast{ nullptr },
	_tcp_socket_param{},
	_multicast_tx_param{},
	_bloadcast_tx_param{},
	_wsa_data{},
	_fds{},
	_fd_read{},
	_timeout{ 0, 500*1000 },
	_tcp_listen_socket{ INVALID_SOCKET },
	_udp_bloadcast_tx_socket{ INVALID_SOCKET },
	_udp_multicast_tx_socket{ INVALID_SOCKET },
	_port{ 0 },
	_multicast_tx_addr{},
	_bloadcast_tx_addr{},
	_local_ip_addr{ "" },
	_ttl{ 0 },
	setting_file_pass{ file_pass }
{
	try
	{
		ZeroMemory(&_tcp_socket_param, sizeof(_tcp_socket_param));
		int init_wsa_result = WSAStartup(MAKEWORD(2, 2), &_wsa_data);
		if (init_wsa_result == false) {
			switch (init_wsa_result) {
			case WSASYSNOTREADY:
				throw std::runtime_error("CASE:WSASYSNOTREADY(ネットワークサブシステムがネットワークへの接続を準備出来ていません)");
				break;

			case WSAVERNOTSUPPORTED:
				throw std::runtime_error("CASE:WSAVERNOTSUPPORTED:(要求された Windows ソケットのサポートのバージョンは、この特定の Windows ソケットの実装では提供されません)");
				break;

			case WSAEINPROGRESS:
				throw std::runtime_error("CASE:WSAEINPROGRESS(Windows ソケット 1.1 のブロック操作が進行中です)");
				break;

			case WSAEPROCLIM:
				throw std::runtime_error("CASE:WSAEPROCLIM(Windows ソケットの実装でサポートされるタスクの数の制限に達しました)");
				break;

			case WSAEFAULT:
				throw std::runtime_error("CASE:WSAEFAULT(lpWSAData パラメーターが有効なポインターではありません)");
				break;
			}
		}
		//ファイルディスクリプタキャッチ用のfdsを初期化
		FD_ZERO(&_fds);
		FD_ZERO(&_fd_read);
		if ((setting_tcp_listen_socket_param(file_pass) == false || (setting_multicast_tx_socket_param(file_pass)) == false)) {
			LOG_ERROR("The socket configuration for TCP listening and multicast transmission was not completed successfully and will be terminated.");
			return;
		}
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
		return;
	}
	catch (const std::exception& e)
	{
		LOG_ERROR(e.what());
		return;
	}

}


//デストラクタ
SocketHandler::~SocketHandler()
{
	freeaddrinfo(_result_tcp);
	closesocket(get_listen_socket());
	closesocket(get_udp_multicast_tx_socket());
	WSACleanup();
}


bool SocketHandler::setting_tcp_listen_socket_param()
{
    this->_tcp_socket_param.ai_flags = AI_PASSIVE; 
	this->_tcp_socket_param.ai_family = AF_UNSPEC; //IPv4とIPv6の両方をサポート
	this->_tcp_socket_param.ai_socktype = SOCK_STREAM; //TCPソケット
	this->_tcp_socket_param.ai_protocol = IPPROTO_TCP; //TCPプロトコルを使用
	this->_port = 50000; //ポート番号は後で設定
	return true;
}

//ここのtry構文見直す
//設定値の読み込み
bool SocketHandler::setting_tcp_listen_socket_param(const std::string& file_pass)
{

	//iniファイルの読み込み
	ini::IniFile socket_param_setting_ini;
	socket_param_setting_ini.load(file_pass);
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

bool SocketHandler::setting_multicast_tx_socket_param()
{
	this->_multicast_tx_param.ai_flags = AI_PASSIVE; //マルチキャスト送信用のソケット
	this->_multicast_tx_param.ai_family = AF_UNSPEC; //IPv4とIPv6の両方をサポート
	this->_multicast_tx_param.ai_socktype = SOCK_DGRAM; //UDPソケット
	this->_multicast_tx_param.ai_protocol = IPPROTO_UDP; //UDPプロトコルを使用
	this->_port = 50001; //マルチキャスト送信用のポート番号は50001に設定
	return true;
}

bool SocketHandler::setting_multicast_tx_socket_param(const std::string& file_pass)
{
	//iniファイルの読み込み
	ini::IniFile multicast_socket_setting_ini;
	multicast_socket_setting_ini.load(file_pass);
	
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

bool SocketHandler::setting_bloadcast_tx_socket_param()
{
	return false;
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
				throw std::runtime_error("WSA ERROR CASE:WSAEAFNOSUPPORT(Family パラメーターで指定されたアドレス ファミリ はサポートされていません。 このエラーは、指定された Family パラメーターが AF_INET または AF_INET6されていない場合に返されます。)");
				break;

			case ERROR_INVALID_PARAMETER:
				throw std::runtime_error("WSA ERROR CASE:ERROR_INVALID_PARAMETER(無効なパラメーターが関数に渡されました。 このエラーは、pStringBuf で NULL ポインターが渡された場合、または StringBufSize パラメーターが 0 の場合に返されます。 このエラーは、 pStringBuf パラメーターが指すバッファーの長さが IP アドレスの文字列表現を受け取るのに十分な大きさでない場合にも返されます。)");
				break;

			default:
				break;
			}
		}else {
			return client_ip_addr_str;
		}
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
		return "";
	}
}


void SocketHandler::setting_select_info_to_listen_socket()
{
	FD_ZERO(&this->_fd_read);
	FD_SET(this->_tcp_listen_socket, &this->_fd_read);
	this->_timeout.tv_sec = 0; //タイムアウト秒数を0に設定
	this->_timeout.tv_usec = 500 * 1000; //タイムアウトマイクロ秒数を500000に設定(0.5秒)
}


void SocketHandler::bind_select_info_to_socket(const SOCKET& socket, fd_set* fd)
{
	FD_SET(socket, fd);
}

void SocketHandler::copy_fd_read_to_fds()
{
	memcpy(&this->_fds, &this->_fd_read, sizeof(fd_set));
}

void SocketHandler::copy_fd_read_to_fds(fd_set* fds, fd_set* fd)
{
	memcpy(fds, fd, sizeof(fd_set));
}




bool SocketHandler::generate_udp_multicast_tx_socket()
{
	if (!setting_multicast_tx_socket_param(INIT_FILE_PASS)) {
		LOG_ERROR("The socket configuration for multicast transmission was not completed successfully and will be terminated.");
		return false;
	}

	try
	{
		int get_addr_result_msg = getaddrinfo(NULL, std::to_string(_multicast_tx_addr.sin_port).c_str(), &_multicast_tx_param, &_result_multicast);
		this->_udp_multicast_tx_socket = socket(_multicast_tx_param.ai_family, _multicast_tx_param.ai_socktype, 0);
		if (this->_udp_multicast_tx_socket == INVALID_SOCKET) {
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				throw std::runtime_error("A successful WSAStartup call must occur before using this function.");
				break;

			case WSAENETDOWN:
				throw std::runtime_error("The network subsystem has failed.");
				break;

			case WSAEAFNOSUPPORT:
				throw std::runtime_error("The specified address family is not supported. For example, an application tried to create a socket for the AF_INET6 address family but IPv6 is not enabled on the local computer.");
				break;

			case WSAEINPROGRESS:
				throw std::runtime_error("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.");
				break;

			case WSAEMFILE:
				throw std::runtime_error("No more socket descriptors are available. The maximum number of sockets is already open in this process.");
				break;

			case WSAEINVAL:
				throw std::runtime_error("An invalid argument was supplied. This error is returned if the af parameter is set to AF_UNSPEC and the type and protocol parameter are unspecified.");
				break;

			case WSAEINVALIDPROVIDER:
				throw std::runtime_error("The service provider returned a version other than 2.2.");
				break;

			case WSAENOBUFS:
				throw std::runtime_error("The service provider returned an invalid or incomplete procedure table to the WSPStartup.");
				break;

			case WSAEPROTONOSUPPORT:
				throw std::runtime_error("The specified protocol is not supported within this address family. For example, an application tried to create a socket for the AF_INET address family with the IPX protocol.");
				break;

			case WSAEPROTOTYPE:
				throw std::runtime_error("The specified protocol is the wrong type for this socket. This error is returned if the type parameter is set to SOCK_STREAM and the protocol parameter is set to IPPROTO_UDP, or if the type parameter is set to SOCK_DGRAM and the protocol parameter is set to IPPROTO_TCP.");
				break;

			case WSAEPROVIDERFAILEDINIT:
				throw std::runtime_error("The service provider failed to initialize. This error is returned if the service provider fails to initialize or if the service provider is not a valid Windows Sockets DLL.");
				break;

			case WSAESOCKTNOSUPPORT:
				throw std::runtime_error("The specified socket type is not supported in this address family. For example, an application tried to create a socket for the AF_INET address family with the SOCK_RAW socket type.");
				break;


			default:
				break;
			}
		
		
		}
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
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
				throw std::runtime_error("A successful WSAStartup call must occur before using this function.");
				break;

			case WSAENETDOWN:
				throw std::runtime_error("The network subsystem has failed.");
				break;

			case WSAEFAULT:
				throw std::runtime_error("The buffer pointed to by the optval parameter is not in a valid part of the process address space or the optlen parameter is too small.");
				break;

			case WSAEINPROGRESS:
				throw std::runtime_error("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.");
				break;

			case WSAEINVAL:
				throw std::runtime_error("The level parameter is not valid, or the information in the buffer pointed to by the optval parameter is not valid.");
				break;

			case WSAENETRESET:
				throw std::runtime_error("The connection has timed out when SO_KEEPALIVE is set.");
				break;

			case WSAENOPROTOOPT:
				throw std::runtime_error("The option is unknown or unsupported for the specified provider or socket (see SO_GROUP_PRIORITY limitations).");
				break;

			case WSAENOTCONN:
				throw std::runtime_error("The connection has been reset when SO_KEEPALIVE is set.");
				break;

			case WSAENOTSOCK:
				throw std::runtime_error("The descriptor is not a socket.");
				break;

			default:
				break;
			}
		}

	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
		return false;
	}
	
	try
	{
		int multicast_ttl_set_sockopt_result = setsockopt(_udp_multicast_tx_socket,
			IPPROTO_IP,
			IP_MULTICAST_TTL,
			(char*)&_ttl,
			sizeof(_ttl));

		if (!multicast_ttl_set_sockopt_result) {
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				throw std::runtime_error("A successful WSAStartup call must occur before using this function.");
				break;

			case WSAENETDOWN:
				throw std::runtime_error("The network subsystem has failed.");
				break;

			case WSAEFAULT:
				throw std::runtime_error("The buffer pointed to by the optval parameter is not in a valid part of the process address space or the optlen parameter is too small.");
				break;

			case WSAEINPROGRESS:
				throw std::runtime_error("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.");
				break;

			case WSAEINVAL:
				throw std::runtime_error("The level parameter is not valid, or the information in the buffer pointed to by the optval parameter is not valid.");
				break;

			case WSAENETRESET:
				throw std::runtime_error("The connection has timed out when SO_KEEPALIVE is set.");
				break;

			case WSAENOPROTOOPT:
				throw std::runtime_error("The option is unknown or unsupported for the specified provider or socket (see SO_GROUP_PRIORITY limitations).");
				break;

			case WSAENOTCONN:
				throw std::runtime_error("The connection has been reset when SO_KEEPALIVE is set.");
				break;

			case WSAENOTSOCK:
				throw std::runtime_error("The descriptor is not a socket.");
				break;

			default:
				break;
			}

		}
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
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
			throw std::out_of_range("The maximum number of client connections has been reached. The connection will be abandoned.");
		}
	}
	catch (const std::out_of_range& e) {
		LOG_ERROR(e.what());
		return false;
	}

	this->_connect_sockets.push_back(socket);
	return true;
}

bool SocketHandler::delete_connect_socket(const SOCKET socket)
{
	if (this->_connect_sockets.empty() == true) {
		LOG_ERROR("Client list is empty.");
		return false;
	}
	else {
		auto it = std::find(this->_connect_sockets.begin(), this->_connect_sockets.end(), socket);
		this->_connect_sockets.erase(it);
		return true;
	}
}

bool SocketHandler::is_socket_closed(const SOCKET socket_fd)
{
	char buf;
	if (socket_fd == INVALID_SOCKET) {
		return true; // 無効なソケットは閉じられているとみなす
	}
	int result = recv(socket_fd, &buf, 1, MSG_PEEK);

	if (result == 0) {
		// クライアントが正常に切断した（FINを受け取った）
		return true;
	}
	if (result < 0) {
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			// ノンブロッキングでデータがまだないだけ
			return false;
		}
		else {
			// 何らかのエラー（ECONNRESETなど）→切断とみなす
			return true;
		}
	}

	// データがある → 切断されていない
	return false;
}

bool SocketHandler::setting_tcp_flags(ini::IniFile tcp_listen_socket_param_setting_ini)
{
	//フラッグの設定
	std::string temp_flag = tcp_listen_socket_param_setting_ini["SocketSetting"]["flags"].as<std::string>();
	if (trans_flags(temp_flag, &_tcp_socket_param) == false) {
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
	if (trans_family(temp_family, &_tcp_socket_param) == false) {
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
	if (trans_socktype(temp_socket_type, &_tcp_socket_param) == false) {
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
	if (trans_protocol(temp_protocol, &_tcp_socket_param) == false) {
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
		int temp_port = tcp_listen_socket_param_setting_ini["SocketSetting"]["port"].as<int>();

		if (temp_port < 0 || temp_port > 65535) {
			throw std::out_of_range("The port number is set out of range. Reset to the range of 0 - 65535.");
		}
		_port = temp_port;
	}
	catch (const std::out_of_range& e)
	{
		LOG_ERROR(e.what());
		return false;
	}
	return true;
}

bool SocketHandler::setting_multicast_tx_sock_af(ini::IniFile socket_param_setting_ini)
{
	std::string temp_af = socket_param_setting_ini["MulticastTX_SocketSetting"]["family"].as<std::string>();
	if (trans_family(temp_af, &_multicast_tx_param) == false) {
		return false;
	}
	return true;
}


bool SocketHandler::setting_multicast_tx_sock_type(ini::IniFile socket_param_setting_ini)
{
	//ソケットタイプの設定
	std::string temp_socktype = socket_param_setting_ini["MulticastTX_SocketSetting"]["type"].as<std::string>();
	if (trans_socktype(temp_socktype, &_multicast_tx_param) == false) {
		return false;
	}
	else {
		return true;
	}

}

bool SocketHandler::setting_multicast_tx_sock_protocol(ini::IniFile socket_param_setting_ini)
{
	int temp_protocol = socket_param_setting_ini["MulticastTX_SocketSetting"]["protocol"].as<int>();
	if (trans_protocol(temp_protocol, &_multicast_tx_param) == false) {
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
		LOG_ERROR("The multicast transmit socket settings were not set correctly(sin_family). Exit the program.");
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

			throw std::out_of_range("The port number is set out of range. Reset to the range of 0 - 65535.");
		}

		if (temp_port == _port) {
			throw std::invalid_argument("The port number for multicast transmission is the same as the port number for TCP transmission. Please change the settings.");
		}
		_multicast_tx_addr.sin_port = temp_port;
#ifdef DEBUG
		spdlog::debug("The send port for multicast is now set to {:d}", m_multicast_tx_addr.sin_port);
#endif // DEBUG

	}
	catch (const std::out_of_range& e)
	{
		LOG_ERROR(e.what());
		return false;
	}
	catch (const std::invalid_argument& e)
	{
		LOG_ERROR(e.what());
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
		int inet_pton_result = inet_pton(_multicast_tx_param.ai_family, temp_multicast_addr.c_str(), &_multicast_tx_addr.sin_addr);
		if (inet_pton_result == -1) {
			switch (WSAGetLastError())
			{
			case WSAEAFNOSUPPORT:
				throw std::runtime_error("WSA ERROR CASE:WSAEAFNOSUPPORT(Family パラメーターで指定されたアドレス ファミリ はサポートされていません。 このエラーは、指定された Family パラメーターが AF_INET または AF_INET6されていない場合に返されます。)");
				break;

			case WSAEFAULT:
				throw std::runtime_error("WSA ERROR CASE:WSAEFAULT(pszAddrString パラメーターまたは pAddrBuf パラメーターが NULL であるか、ユーザー アドレス空間の一部ではありません。)");
				break;

			default:
				break;
			}
		}
		else if (inet_pton_result == 0) {
			throw std::invalid_argument("The pAddrBuf parameter points to a string that is not a valid IPv4 dotted decimal string or a valid IPv6 address string.");
		}
		//IPアドレスのバイナリ化が正常に終了した場合
		else {
#ifdef DEBUG
			spdlog::debug("The multicast transmission address has been set successfully (address: {:s})", temp_multicast_addr);
#endif // DEBUG
		}
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
		return false;
	}
	catch (const std::invalid_argument& e)
	{
		LOG_ERROR(e.what());
		return false;
	}
	catch (const std::exception& e)
	{
		LOG_ERROR(e.what());
		return false;
	}
	
	return true;
}


std::string SocketHandler::obtain_local_ip_addr()
{
	DWORD get_ip_addr_table_result;
	//ローカルIPアドレス
	DWORD _i_count = 0;
	PMIB_IPADDRTABLE _ip_addr_table = nullptr;
	DWORD _dw_size = 0;
	DWORD _dw_ret_val = 0;
	
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
			throw std::runtime_error("ERROR CASE: ERROR_INSUFFICIENT_BUFFER(The buffer pointed to by the pIpAddrTable parameter is not large enough. The required size is returned in the DWORD variable pointed to by the pdwSize parameter.)");
			break;

		case ERROR_INVALID_PARAMETER:
			throw std::runtime_error("ERROR CASE:ERROR_INVALID_PARAMETER(The pdwSize parameter is NULL, or GetIpAddrTable is unable to write to the memory pointed to by the pdwSize parameter.)");
			break;

		case ERROR_NOT_SUPPORTED:
			throw std::runtime_error("ERROR CASE:ERROR_NOT_SUPPORTED(This function is not supported on the operating system in use on the local system.)");
			break;

		case NO_ERROR:
			break;
		default:
			throw std::runtime_error("ERROR CASE:Other(Use FormatMessage to obtain the message string for the returned error.)");
			break;
		}

	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
		return "";
	}
	catch (const std::exception& e)
	{
		LOG_ERROR(e.what());
		return "";
	}
	
	//IPアドレステーブルの結果を格納
	try
	{
		DWORD dw_ret_val_result = GetIpAddrTable(_ip_addr_table, &_dw_size, 0);
		switch (dw_ret_val_result)
		{
		case ERROR_INSUFFICIENT_BUFFER:
			throw std::runtime_error("ERROR CASE: ERROR_INSUFFICIENT_BUFFER(The buffer pointed to by the pIpAddrTable parameter is not large enough. The required size is returned in the DWORD variable pointed to by the pdwSize parameter.)");
			break;

		case ERROR_INVALID_PARAMETER:
			throw std::runtime_error("ERROR CASE:ERROR_INVALID_PARAMETER(The pdwSize parameter is NULL, or GetIpAddrTable is unable to write to the memory pointed to by the pdwSize parameter.)");
			break;

		case ERROR_NOT_SUPPORTED:
			throw std::runtime_error("ERROR CASE:ERROR_NOT_SUPPORTED(This function is not supported on the operating system in use on the local system.)");
			break;

		case NO_ERROR:

			break;

		default:
			break;
		}
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
		return "";
	}
	catch (const std::exception& e)
	{
		LOG_ERROR(e.what());
		return "";
	}


	if (_ip_addr_table->dwNumEntries > 0) {
		char temp_local_ip_addr[100];
		for (_i_count = 0; _i_count < _ip_addr_table->dwNumEntries; _i_count++) {
			inet_ntop(AF_INET, (struct in_addr*)&_ip_addr_table->table[_i_count].dwAddr, temp_local_ip_addr, sizeof(temp_local_ip_addr));
			if (_i_count == 0) {
				_local_ip_addr = temp_local_ip_addr;
			}
		}
		spdlog::debug("server-ip, success, obtain server ip address.");
	}
	else{
		LOG_ERROR("Failed to obtain the server IP address.");
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
			throw std::runtime_error(socket_error_msg.str());
			
		}
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
		return false;
	}
	catch (const std::exception& e)
	{
		LOG_ERROR(e.what());
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
				throw std::runtime_error("WSA ERROR CASE:WSANOTINITIALISED(この関数を使用する前に、 WSAStartup 呼び出しが正常に行われる必要があります。)");
				break;

			case WSAECONNRESET:
				throw std::runtime_error("WSA ERROR CASE:WSAECONNRESET(着信接続が示されましたが、その後、呼び出しを受け入れる前にリモート ピアによって終了されました。)");
				break;

			case WSAEFAULT:
				throw std::runtime_error("WSA ERROR CASE:WSAEFAULT(addrlen パラメーターが小さすぎるか、addr がユーザー アドレス空間の有効な部分ではありません。)");
				break;

			case WSAEINTR:
				throw std::runtime_error("WSA ERROR CASE:WSAEINTR(ブロッキング Windows Sockets 1.1 呼び出しは、WSACancelBlockingCall を介して取り消されました。)");
				break;

			case WSAEINVAL:
				throw std::runtime_error("WSA ERROR CASE:WSAEINVAL(リッスン関数は、受け入れる前に呼び出されませんでした。)");
				break;

			case WSAEINPROGRESS:
				throw std::runtime_error("WSA ERROR CASE:WSAEINPROGRESS(ブロックしている Windows Sockets 1.1 呼び出しが進行中であるか、サービス プロバイダーがコールバック関数を処理しています。)");
				break;

			case EMFILE:
				throw std::runtime_error("WSA ERROR CASE:EMFILE(キューは、受け入れるエントリに空きがなく、使用可能な記述子がありません。)");
				break;

			case WSAENETDOWN:
				throw std::runtime_error("WSA ERROR CASE:WSAENETDOWN(ネットワーク サブシステムが失敗しました。)");
				break;

			case WSAENOBUFS:
				throw std::runtime_error("WSA ERROR CASE:WSAENOBUFS(バッファーに空き領域がありません。)");
				break;

			case WSAENOTSOCK:
				throw std::runtime_error("WSA ERROR CASE:WSAENOTSOCK(記述子はソケットではありません。)");
				break;

			case WSAEOPNOTSUPP:
				throw std::runtime_error("WSA ERROR CASE:WSAEOPNOTSUPP(参照されるソケットは、接続指向サービスをサポートする型ではありません。)");
				break;

			case WSAEWOULDBLOCK:
				throw std::runtime_error("WSA ERROR CASE:WSAEWOULDBLOCK(ソケットは非ブロッキングとしてマークされ、受け入れられる接続はありません。)");
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
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
		struct sockaddr_in error_addr_in = { 0 };
		return std::make_tuple(INVALID_SOCKET, error_addr_in);
	}
}


void SocketHandler::disconnect_socket(const SOCKET client_socket)
{
	try
	{
		if (client_socket == INVALID_SOCKET) {
			throw std::runtime_error("The client socket is invalid. The socket may already be closed.");
		}
		int close_socket_result = closesocket(client_socket);
		if (close_socket_result == SOCKET_ERROR) {
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				throw std::runtime_error("WSA ERROR CASE : WSANOTINITIALISED(この関数を使用する前に、 WSAStartup 呼び出しが正常に行われる必要があります。)");
				break;

			case WSAENETDOWN:
				throw std::runtime_error("WSA ERROR CASE : WSAENETDOWN(ネットワーク サブシステムが失敗しました。)");
				break;

			case WSAENOTSOCK:
				throw std::runtime_error("WSA ERROR CASE : WSAENOTSOCK(記述子はソケットではありません。)");
				break;

			case WSAEINPROGRESS:
				throw std::runtime_error("WSA ERROR CASE : WSAEINPROGRESS(ブロックしている Windows Sockets 1.1 呼び出しが進行中であるか、サービス プロバイダーがコールバック関数を処理しています。)");
				break;

			case WSAEINTR:
				throw std::runtime_error("WSA ERROR CASE : WSAEINTR(ブロッキング Windows Socket 1.1 呼び出しは、WSACancelBlockingCall を介して取り消されました。)");
				break;

			case WSAEWOULDBLOCK:
				throw std::runtime_error("WSA ERROR CASE : WSAEWOULDBLOCK(ソケットは非ブロッキングとしてマークされ、残留構造のl_onoff メンバーは 0 以外に設定され、残留構造のl_linger メンバーは 0 以外のタイムアウト値に設定されます。)");
				break;

			default:
				break;
			}
		}
		else {
			spdlog::info("client socket({:d}) is closed.", client_socket);
		}

	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
	}
	

}



bool SocketHandler::generate_tcp_listen_socket()
{
	////パラメータ設定
	//if (!setting_tcp_listen_socket_param(INIT_FILE_PASS)) {
	//	return false;
	//}

	//名前解決
	try
	{
		int get_addr_result_msg = getaddrinfo(NULL, std::to_string(_port).c_str(), &_tcp_socket_param, &_result_tcp);
		if (get_addr_result_msg != 0) {
			switch (get_addr_result_msg)
			{
			case EAI_AGAIN:
				throw std::runtime_error("名前解決の一時的なエラーが発生しました。再試行してください。");
				break;

			case EAI_BADFLAGS:
				throw std::runtime_error("pHints パラメーターの ai_flags メンバーに無効な値が指定されました。");
				break;
			
			case EAI_FAIL:
				throw std::runtime_error("名前解決で回復不可能なエラーが発生しました。ネットワーク接続を確認してください。");
				break;

			case EAI_FAMILY:
				throw std::runtime_error("pHints パラメーターの ai_family メンバーに無効な値が指定されました。AF_INET または AF_INET6 を使用してください。");
				break;

			case EAI_MEMORY:
				throw std::runtime_error("メモリ割り当てエラーが発生しました。システムのメモリを確認してください。");
				break;

			case EAI_NONAME:
				throw std::runtime_error("指定されたパラメーターの名前が解決されないか、pNodeName パラメーターと pServiceName パラメーターが指定されませんでした。");
				break;

			case EAI_SERVICE:
				throw std::runtime_error("pServiceName パラメーターが指定されているが、pHints パラメーターの ai_socktype メンバーでサポートされていないサービス名が指定されました。");
				break;

			case EAI_SOCKTYPE:
				throw std::runtime_error("pHints パラメーターの ai_socktype メンバーに無効な値が指定されました。SOCK_STREAM または SOCK_DGRAM を使用してください。");
				break;

			default:
				throw std::runtime_error("何らかの原因でgetaddrinfoの結果が正常ではありません。エラーコード: " + std::to_string(get_addr_result_msg));
				break;
			}
		}

		//リッスンソケットの生成
		_tcp_listen_socket = socket(_result_tcp -> ai_family, _result_tcp -> ai_socktype, _result_tcp->ai_protocol);
		if (_tcp_listen_socket == INVALID_SOCKET) {
			throw std::runtime_error("ソケットの生成に失敗しました。WSAGetLastError Code : " + std::to_string(WSAGetLastError()));
		}
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
		freeaddrinfo(_result_tcp);
		WSACleanup();
		return false;
	}
	catch (const std::exception& e)
	{
		LOG_ERROR(e.what());
		freeaddrinfo(_result_tcp);
		WSACleanup();
		return false;
	}
	spdlog::debug("server generated listen socket({:d}).", _tcp_listen_socket);

	bool bind_result = bind_tcp_listen_socket(&_tcp_listen_socket, _result_tcp);
	if (bind_result == true) {
		spdlog::debug("server bind setting on listen socket({:d}).", _tcp_listen_socket);
	}
	else {
		LOG_ERROR("server can't bind setting on listen socket({:d}).", _tcp_listen_socket);
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
		LOG_ERROR("Failed to set ai_flags.");
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
		LOG_ERROR("Failed to set ai_family.");
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
		LOG_ERROR("Failed to set ai_socktype.");
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
		LOG_ERROR("Failed to set ai_protocol.");
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
		LOG_ERROR("Failed to set ai_protocol.");
		return false;
	}
	return true;
}


void SocketHandler::monitor_socket_descriptor(const std::vector<SOCKET>& socket_list, std::function<void(SOCKET)> callback)
{
	try {
		int select_result = select(0, &_fds, NULL, NULL, &_timeout);

		switch (select_result) {
		case 0:
			return; // タイムアウト
		case SOCKET_ERROR:
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				throw std::runtime_error("WSA ERROR CASE:WSANOTINITIALISED(この関数を使用する前に、 WSAStartup 呼び出しが正常に行われる必要があります。)");
				break;
			case WSAEFAULT:
				throw std::runtime_error("WSA ERROR CASE:WSAEFAULT(Windows ソケットの実装では、内部操作に必要なリソースを割り当てることができませんでした。または、 readfds、 writefds、 exceptfds、または timeval パラメーターがユーザー アドレス空間の一部ではありません。)");
				break;
			case ENETDOWN:
				throw std::runtime_error("WSA ERROR CASE:ENETDOWN(ネットワーク サブシステムが失敗しました。)");
				break;
			case WSAEINVAL:
				throw std::runtime_error("WSA ERROR CASE:WSAEINVAL(タイムアウト値が無効であるか、3 つの記述子パラメーターがすべて null でした。)");
				break;
			case WSAEINTR:
				throw std::runtime_error("WSA ERROR CASE:WSAEINTR(WSACancelBlockingCall を使用して、Windows ソケット 1.1 呼び出しのブロックが取り消されました。)");
				break;
			case WSAEINPROGRESS:
				throw std::runtime_error("WSA ERROR CASE:WSAEINPROGRESS(ブロック中の Windows ソケット 1.1 呼び出しが進行中であるか、サービス プロバイダーがコールバック関数を処理しています。)");
				break;
			case WSAENOTSOCK:
				throw std::runtime_error("WSA ERROR CASE:WSAENOTSOCK(記述子セットの 1 つに、ソケットではないエントリが含まれています。)");
				break;
			default:
				break;
			}
			return;
		default:
			if (FD_ISSET(_tcp_listen_socket, &_fds)) {
				callback(_tcp_listen_socket); // 新しい接続リクエスト
			}
			for (const auto& socket : socket_list) {
				if (socket != INVALID_SOCKET && FD_ISSET(socket, &_fds)) {
					callback(socket); // 変化のあったソケットを通知
				}
			}
			break;
		}
	}
	catch (const std::runtime_error& e) {
		LOG_ERROR(e.what());
	}
}


bool SocketHandler::open_tcp_listen_socket()
{	
	try
	{
		//tcp_listen_socketのオープンに失敗した場合
		if (!generate_tcp_listen_socket()) {
			throw std::runtime_error("Failed to generate TCP listen socket for message.");
		}
		if (listen(_tcp_listen_socket, SOMAXCONN) == SOCKET_ERROR) {
			throw std::runtime_error("Tcp Listen socket has been SOCKET_ERROR for some reason. WSAGetLastError Code : " + std::to_string(WSAGetLastError()));
			
		}
		else
		{
			spdlog::info("server open listen socket:{:d}.", _tcp_listen_socket);
		}
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
		freeaddrinfo(_result_tcp);
		closesocket(_tcp_listen_socket);
		WSACleanup();
		return false;
	}
	catch (const std::exception& e)
	{
		LOG_ERROR(e.what());
		freeaddrinfo(_result_tcp);
		closesocket(_tcp_listen_socket);
		WSACleanup();
		return false;
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

