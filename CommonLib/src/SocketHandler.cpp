#include "SocketHandler.h"

//#define DEBUG


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
	_fds{},
	_fd_read{},
	_timeout{0, 500*1000},
	_tcp_listen_socket{ INVALID_SOCKET_FD },
	_udp_bloadcast_tx_socket{ INVALID_SOCKET_FD },
	_udp_multicast_tx_socket{ INVALID_SOCKET_FD },
	_port{ 0 },
	_multicast_tx_addr{},
	_bloadcast_tx_addr{},
	_local_ip_addr{""},
	_ttl{ 0 },
	socket_config_file_pass{ ""}
#ifdef _WIN32
	, _wsa_data{}
#endif // _WIN32
{
	try
	{
		memset(&_tcp_socket_param, 0, sizeof(_tcp_socket_param));
#ifdef _WIN32
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
#endif // _WIN32

		
		//ファイルディスクリプタキャッチ用のfdsを初期化
		FD_ZERO(&_fds);
		FD_ZERO(&_fd_read);

		if ((set_socket_param() == false || (set_multicast_tx_socket_param()) == false)) {
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
	_fds{},
	_fd_read{},
	_timeout{ 0, 500*1000 },
	_tcp_listen_socket{ INVALID_SOCKET_FD },
	_udp_bloadcast_tx_socket{ INVALID_SOCKET_FD },
	_udp_multicast_tx_socket{ INVALID_SOCKET_FD },
	_port{ 0 },
	_multicast_tx_addr{},
	_bloadcast_tx_addr{},
	_local_ip_addr{ "" },
	_ttl{ 0 },
	socket_config_file_pass{ file_pass }
#ifdef _WIN32
	, _wsa_data{}
#endif // _WIN32
{
	try
	{
		memset(&_tcp_socket_param, 0, sizeof(_tcp_socket_param));
#ifdef _WIN32
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
#endif // _WIN32

		//ファイルディスクリプタキャッチ用のfdsを初期化
		FD_ZERO(&_fds);
		FD_ZERO(&_fd_read);
		if ((set_socket_param(file_pass) == false || (set_multicast_tx_socket_param(file_pass)) == false)) {
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
	//freeaddrinfo(_result_tcp);
	//closesocket(get_listen_socket());
	//closesocket(get_udp_multicast_tx_socket());
	//WSACleanup();

	if (_result_tcp != nullptr) {
		freeaddrinfo(_result_tcp);
	}

	if (get_listen_socket() != INVALID_SOCKET_FD) {
#ifdef _WIN32
		closesocket(get_listen_socket());
#else
		close(get_listen_socket());
#endif
	}

	if (get_udp_multicast_tx_socket() != INVALID_SOCKET_FD) {
#ifdef _WIN32
		closesocket(get_udp_multicast_tx_socket());
#else
		close(get_udp_multicast_tx_socket());
#endif
	}

#ifdef _WIN32
	WSACleanup();
#endif
}

socket_t SocketHandler::get_listen_socket()
{
	return this->_tcp_listen_socket;
}

socket_t SocketHandler::get_udp_multicast_tx_socket()
{
	return this->_udp_multicast_tx_socket;
}

fd_set& SocketHandler::get_fd_read()
{
	return this->_fd_read;
}

fd_set& SocketHandler::get_fds()
{
	return this->_fds;
}

std::string& SocketHandler::get_server_ip_address()
{
	return this->_local_ip_addr;
}

int SocketHandler::get_port()
{
	return this->_port;
}


bool SocketHandler::set_socket_param()
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
bool SocketHandler::set_socket_param(const std::string& config_file_path)
{

	//iniファイルの読み込み
	ini::IniFile socket_param_setting_ini;
	socket_param_setting_ini.load(config_file_path);
	if (this->set_tcp_flags(socket_param_setting_ini) == false) {
		return false;
	}
	if (this->set_tcp_family(socket_param_setting_ini) == false) {
		return false;
	}
	if (this->set_tcp_socktype(socket_param_setting_ini) == false) {
		return false;
	}
	if (this->set_tcp_protocol(socket_param_setting_ini) == false) {
		return false;
	}
	if (this->set_tcp_port(socket_param_setting_ini) == false) {
		return false;
	}
	return true;
}

bool SocketHandler::set_multicast_tx_socket_param()
{
	this->_multicast_tx_param.ai_flags = AI_PASSIVE; //マルチキャスト送信用のソケット
	this->_multicast_tx_param.ai_family = AF_UNSPEC; //IPv4とIPv6の両方をサポート
	this->_multicast_tx_param.ai_socktype = SOCK_DGRAM; //UDPソケット
	this->_multicast_tx_param.ai_protocol = IPPROTO_UDP; //UDPプロトコルを使用
	this->_port = 50001; //マルチキャスト送信用のポート番号は50001に設定
	return true;
}

bool SocketHandler::set_multicast_tx_socket_param(const std::string& config_file_path)
{
	//iniファイルの読み込み
	ini::IniFile multicast_socket_setting_ini;
	multicast_socket_setting_ini.load(config_file_path);
	
	if (this->set_multicast_tx_sock_af(multicast_socket_setting_ini) == false) {
		return false;
	}
	if (this->set_multicast_tx_sock_type(multicast_socket_setting_ini) == false) {
		return false;
	}
	if (this->set_multicast_tx_sock_protocol(multicast_socket_setting_ini) == false) {
		return false;
	}
	if (this->set_multicast_tx_sin_family(multicast_socket_setting_ini) == false) {
		return false;
	}
	if (this->set_multicast_tx_sin_port(multicast_socket_setting_ini) == false) {
		return false;
	}
	if (this->set_multicast_tx_sin_address(multicast_socket_setting_ini) == false) {
		return false;
	}
	return true;


}

bool SocketHandler::set_bloadcast_tx_socket_param()
{
	return false;
}

bool SocketHandler::set_bloadcast_tx_socket_param(const std::string& file_pass)
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
		const char* iresult;
		iresult = inet_ntop(temp_paddr.sin_family, &temp_paddr.sin_addr,
			client_ip_addr_char,ipv4_addr_size);
		std::string client_ip_addr_str(client_ip_addr_char);
		if (iresult == NULL) {
#ifdef _WIN32
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
#else
			throw std::runtime_error("inet_ntop failed to convert IP address to string. Check the address family and parameters.");
#endif // _WIN32

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


void SocketHandler::initialize_select_param()
{
	FD_ZERO(&this->_fd_read);
	FD_SET(this->_tcp_listen_socket, &this->_fd_read);
	this->_timeout.tv_sec = 0; //タイムアウト秒数を0に設定
	this->_timeout.tv_usec = 500 * 1000; //タイムアウトマイクロ秒数を500000に設定(0.5秒)
}


void SocketHandler::bind_select_info_to_socket(socket_t socket, fd_set* fd)
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

	try
	{
		int get_addr_result_msg = getaddrinfo(NULL, std::to_string(_multicast_tx_addr.sin_port).c_str(), &_multicast_tx_param, &_result_multicast);
		this->_udp_multicast_tx_socket = socket(_multicast_tx_param.ai_family, _multicast_tx_param.ai_socktype, 0);


		if (this->_udp_multicast_tx_socket == INVALID_SOCKET_FD) {
#ifdef _WIN32
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
#else
			throw std::runtime_error("Failed to create UDP multicast transmission socket. Check the address family and parameters.");
#endif // _WIN32
		
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
#ifdef _WIN32
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
#else
			throw std::runtime_error("Failed to set multicast interface option on the UDP multicast transmission socket. Check the parameters and address family.");
#endif // _WIN32
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
			//(char*)&_ttl,
			reinterpret_cast<const char*>(&_ttl),
			sizeof(_ttl));

		if (!multicast_ttl_set_sockopt_result) {
#ifdef _WIN32
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
#else
			throw std::runtime_error("Failed to set multicast TTL option on the UDP multicast transmission socket. Check the parameters and address family.");
#endif // _WIN32

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


bool SocketHandler::is_socket_closed(const socket_t socket_fd)
{
	//char buf;
	//if (socket_fd == INVALID_SOCKET_FD) {
	//	return true; // 無効なソケットは閉じられているとみなす
	//}
	//int result = recv(socket_fd, &buf, 1, MSG_PEEK);

	//if (result == 0) {
	//	// クライアントが正常に切断した（FINを受け取った）
	//	return true;
	//}
	//if (result < 0) {
	//	if (errno == EWOULDBLOCK || errno == EAGAIN) {
	//		// ノンブロッキングでデータがまだないだけ
	//		return false;
	//	}
	//	else {
	//		// 何らかのエラー（ECONNRESETなど）→切断とみなす
	//		return true;
	//	}
	//}

	//// データがある → 切断されていない
	//return false;



	char buf;

	if (socket_fd == INVALID_SOCKET_FD) {
		return true; // 無効なソケットは閉じているとみなす
	}

	int result = recv(socket_fd, &buf, 1, MSG_PEEK);

#ifdef _WIN32
	if (result == 0) {
		return true; // FIN を受け取った（切断済）
	}
	if (result == SOCKET_ERROR_FD) {
		int error_code = WSAGetLastError();
		if (error_code == WSAEWOULDBLOCK) {
			return false; // データはまだないがソケットは有効
		}
		else {
			return true; // その他のエラーは切断とみなす
		}
	}
#else
	if (result == 0) {
		return true; // FIN を受け取った（切断済）
	}
	if (result < 0) {
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			return false; // ノンブロッキングでまだデータがない
		}
		else {
			return true; // その他のエラー（ECONNRESETなど）
		}
	}
#endif

	return false; // recvでバイトが返された → 切断されていない
}

bool SocketHandler::set_tcp_flags(ini::IniFile tcp_listen_socket_param_config)
{
	//フラッグの設定
	std::string temp_flag = tcp_listen_socket_param_config["SocketSetting"]["flags"].as<std::string>();
	if (trans_flags(temp_flag, &_tcp_socket_param) == false) {
		return false;
	}
	else {
		return true;
	}

}

bool SocketHandler::set_tcp_family(ini::IniFile tcp_listen_socket_param_config)
{
	//iniファイルの各セクションからキーを読み込む
	//アドレスファミリの設定(ipv4 or ipv6)
	std::string temp_family = tcp_listen_socket_param_config["SocketSetting"]["family"].as<std::string>();
	if (trans_family(temp_family, &_tcp_socket_param) == false) {
		return false;
	}
	else {
		return true;
	}
	
}

bool SocketHandler::set_tcp_socktype(ini::IniFile tcp_listen_socket_param_config)
{
	//ソケット型の設定
	std::string temp_socket_type = tcp_listen_socket_param_config["SocketSetting"]["socktype"].as<std::string>();
	if (trans_socktype(temp_socket_type, &_tcp_socket_param) == false) {
		return false;
}
	else {
		return true;
	}
}

bool SocketHandler::set_tcp_protocol(ini::IniFile tcp_listen_socket_param_config)
{
	//プロトコルの設定
	std::string temp_protocol = tcp_listen_socket_param_config["SocketSetting"]["protocol"].as<std::string>();
	//std::equal(temp_family.begin(), temp_family.end(), "AF_INET")
	//プロトコルをtcpに設定する場合
	if (trans_protocol(temp_protocol, &_tcp_socket_param) == false) {
		return false;
	}
	else {
		return true;
	}
}

bool SocketHandler::set_tcp_port(ini::IniFile tcp_listen_socket_param_config)
{
	//ポートの設定
	try
	{
		int temp_port = tcp_listen_socket_param_config["SocketSetting"]["port"].as<int>();

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

bool SocketHandler::set_multicast_tx_sock_af(ini::IniFile socket_param_config)
{
	std::string temp_af = socket_param_config["MulticastTX_SocketSetting"]["family"].as<std::string>();
	if (trans_family(temp_af, &_multicast_tx_param) == false) {
		return false;
	}
	return true;
}


bool SocketHandler::set_multicast_tx_sock_type(ini::IniFile socket_param_config)
{
	//ソケットタイプの設定
	std::string temp_socktype = socket_param_config["MulticastTX_SocketSetting"]["type"].as<std::string>();
	if (trans_socktype(temp_socktype, &_multicast_tx_param) == false) {
		return false;
	}
	else {
		return true;
	}

}

bool SocketHandler::set_multicast_tx_sock_protocol(ini::IniFile socket_param_config)
{
	int temp_protocol = socket_param_config["MulticastTX_SocketSetting"]["protocol"].as<int>();
	if (trans_protocol(temp_protocol, &_multicast_tx_param) == false) {
		return false;
	
	}
	return true;
}

bool SocketHandler::set_multicast_tx_sin_family(ini::IniFile socket_param_config)
{
	//アドレスファミリの設定(socket関数の引数設定)
	std::string temp_af = socket_param_config["MulticastTX_SocketSetting"]["sin_family"].as<std::string>();

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
	else if (std::equal(temp_af.begin(), temp_af.end(), "AF_INET6")) {
		_multicast_tx_addr.sin_family = AF_INET6;
#ifdef DEBUG
		spdlog::debug("multicast param：sin_family = AF_INET6");
#endif // DEBUG

	}
	else {
		LOG_ERROR("The multicast transmit socket settings were not set correctly(sin_family). Exit the program.");
		return false;
	}
	return true;
}



bool SocketHandler::set_multicast_tx_sin_port(ini::IniFile socket_param_setting_ini)
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

bool SocketHandler::set_multicast_tx_sin_address(ini::IniFile socket_param_config)
{
	std::string temp_multicast_addr = socket_param_config["MulticastTX_SocketSetting"]["sin_address"].as<std::string>();

	//inet_addrの設定
	try
	{
		int inet_pton_result = inet_pton(_multicast_tx_param.ai_family, temp_multicast_addr.c_str(), &_multicast_tx_addr.sin_addr);
		if (inet_pton_result == -1) {
#ifdef _WIN32
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
#else
			throw std::runtime_error("inet_pton failed to convert the address string to binary format. Check the address family and parameters.");
#endif //_WIN32
		
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
#ifdef _WIN32
std::string SocketHandler::get_last_wsa_error_message()
{
	DWORD error_code = WSAGetLastError();
	LPSTR message_buffer = nullptr;

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error_code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&message_buffer,
		0,
		NULL
	);

	std::string message = message_buffer ? message_buffer : "Unknown error";
	LocalFree(message_buffer); // メモリを解放
	return message;
}
#endif //_WIN32


std::string SocketHandler::obtain_local_ip_addr()
{
#ifdef _WIN32
	DWORD get_ip_addr_table_result;
	DWORD _i_count = 0;
	PMIB_IPADDRTABLE _ip_addr_table = nullptr;
	DWORD _dw_size = 0;
	std::string _local_ip_addr;

	try
	{
		if (GetIpAddrTable(NULL, &_dw_size, 0) == ERROR_INSUFFICIENT_BUFFER) {
			_ip_addr_table = (MIB_IPADDRTABLE*)malloc(_dw_size);
		}

		get_ip_addr_table_result = GetIpAddrTable(_ip_addr_table, &_dw_size, 0);
		if (get_ip_addr_table_result != NO_ERROR) {
			throw std::runtime_error("Failed to get IP address table");
		}

		if (_ip_addr_table->dwNumEntries > 0) {
			char temp_local_ip_addr[INET_ADDRSTRLEN];
			for (_i_count = 0; _i_count < _ip_addr_table->dwNumEntries; _i_count++) {
				inet_ntop(AF_INET, (struct in_addr*)&_ip_addr_table->table[_i_count].dwAddr, temp_local_ip_addr, sizeof(temp_local_ip_addr));
				if (_i_count == 0) {
					_local_ip_addr = temp_local_ip_addr;
					break;
				}
			}
		}
		else {
			throw std::runtime_error("No IP entries found");
		}

		if (_ip_addr_table) free(_ip_addr_table);

		return _local_ip_addr;
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Error obtaining local IP address: {}", e.what());
		return "";
	}
#else
	// Linux/macOS版
	struct ifaddrs* ifaddr, * ifa;
	char host[NI_MAXHOST];
	std::string local_ip_addr;

	if (getifaddrs(&ifaddr) == -1) {
		return "";
	}

	for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == nullptr)
			continue;

		int family = ifa->ifa_addr->sa_family;
		if (family == AF_INET) { // IPv4
			int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
				host, NI_MAXHOST,
				nullptr, 0, NI_NUMERICHOST);
			if (s == 0) {
				std::string ifname(ifa->ifa_name);
				// "lo" (ループバック)を除外したい場合
				if (ifname != "lo") {
					local_ip_addr = host;
					break;  // 最初の非ループバックIPv4アドレスを返す
				}
			}
		}
	}

	freeifaddrs(ifaddr);
	return local_ip_addr;
#endif

}

bool SocketHandler::bind_tcp_listen_socket(int listen_socket, struct addrinfo* result)
{
	try
	{
#ifdef _WIN32
		SOCKET casted_listen_socket = static_cast<SOCKET>(listen_socket);
		int bind_result = bind(casted_listen_socket, result->ai_addr, (int)result->ai_addrlen);
		if (bind_result == SOCKET_ERROR_FD) {
			std::stringstream socket_error_msg;
			socket_error_msg << "An error occurred while binding the TCP listening socket. WSAGetLastError Code : " << std::to_string(WSAGetLastError());
			throw std::runtime_error(socket_error_msg.str());

		}
#else
		int bind_result = bind(listen_socket, result->ai_addr, (int)result->ai_addrlen);
		if (bind_result == -1) {
			std::stringstream socket_error_msg;
			socket_error_msg << "An error occurred while binding the TCP listening socket.";
			throw std::runtime_error(socket_error_msg.str());

		}
#endif //_WIN32
		
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

std::tuple<socket_t, sockaddr_in> SocketHandler::accept_client_socket(socklen_t client_sockaddr_in_length)
{
	socket_t accept_client_socket_result;
	struct sockaddr_in accept_sockaddr_in_result;
	try
	{
		
#ifdef _WIN32
		int addr_len = static_cast<int>(client_sockaddr_in_length);
		accept_client_socket_result = accept(_tcp_listen_socket, (struct sockaddr*)&accept_sockaddr_in_result, &addr_len);
		client_sockaddr_in_length = static_cast<socklen_t>(addr_len);
#else
		accept_client_socket_result = accept(_tcp_listen_socket, (struct sockaddr*)&accept_sockaddr_in_result, &client_sockaddr_in_length);
#endif

		//accept_client_socket_result = accept(_tcp_listen_socket, (struct sockaddr*)&accept_sockaddr_in_result, &client_sockaddr_in_length);

		//acceptしたソケットがINVALID_SOCKET_FDである場合
		if (accept_client_socket_result == INVALID_SOCKET_FD) {
#ifdef _WIN32
			int error_code = WSAGetLastError();
			std::string message = get_last_wsa_error_message();
			std::string error_message = "accept failed. WSA error code: " + std::to_string(error_code) + ", error message:" + message + ".";
#else
			int error_code = errno;
			std::string error_message = "accept failed with error code: " + std::to_string(error_code) + ", error message:" + std::string(strerror(error_code)) + ".";
#endif // _WIN32
			throw std::runtime_error(error_message);
		}
		else {
			//acceptが成功した場合
			return std::make_tuple(accept_client_socket_result, accept_sockaddr_in_result);
		}
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR("runtime error : {}",e.what());
		struct sockaddr_in error_addr_in = { 0 };
		return std::make_tuple(INVALID_SOCKET_FD, error_addr_in);
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("exception error : {}", e.what());
		struct sockaddr_in error_addr_in = { 0 };
		return std::make_tuple(INVALID_SOCKET_FD, error_addr_in);
	}
}


void SocketHandler::close_socket(const socket_t client_socket)
{
	int clnt_socket_result = 0;
	try
	{
		if (client_socket == INVALID_SOCKET_FD) {
			throw std::runtime_error("The client socket is invalid. The socket may already be closed.");
		}
#ifdef _WIN32
		shutdown(client_socket, SD_BOTH);
		int close_socket_result = closesocket(client_socket);
#else
		shutdown(client_socket, SHUT_RDWR);
		int close_socket_result = close(client_socket);
#endif // _WIN32

		if (close_socket_result == SOCKET_ERROR_FD) {
#ifdef _WIN32
			int error_code = WSAGetLastError();
			std::string message = get_last_wsa_error_message();
			std::string error_message = "closesocket failed. WSA error code: " + std::to_string(error_code) + ", error message:" + message + ".";
#else
			int error_code = errno;
			std::string error_message = "close failed with error code: " + std::to_string(error_code) + ", error message:" + std::string(strerror(error_code)) + ".";
#endif // _WIN32
			throw std::runtime_error(error_message);
		}
		else {
			spdlog::debug("client socket({:d}) is closed.", client_socket);
		}
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
	}
	

}



bool SocketHandler::generate_tcp_listen_socket()
{
	
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
		if (_tcp_listen_socket == INVALID_SOCKET_FD) {
#ifdef _WIN32
			throw std::runtime_error("failed generated socket. WSAGetLastError Code : " + std::to_string(WSAGetLastError()));
#else
			throw std::runtime_error("failed generated socket. Errno Code : " + std::to_string(errno) + ", Error message : " + std::string(strerror(errno)));
#endif // _WIN32
		}
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
		if (_result_tcp != nullptr) {
			freeaddrinfo(_result_tcp);
		}
#ifdef _WIN32
		WSACleanup();
#endif
		return false;
	}
	catch (const std::exception& e)
	{
		LOG_ERROR(e.what());
		if (_result_tcp != nullptr) {
			freeaddrinfo(_result_tcp);
		}
#ifdef _WIN32
		WSACleanup();
#endif
		return false;
	}
	spdlog::debug("server generated listen socket({:d}).", _tcp_listen_socket);

	bool bind_result = bind_tcp_listen_socket(static_cast<int>(_tcp_listen_socket), _result_tcp);
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
#ifdef _WIN32
	//プロトコルをpgmに設定する場合
	else if ((std::equal(protocol_str.begin(), protocol_str.end(), "IPPROTO_PGM")) && param->ai_family == AF_INET && param->ai_socktype == SOCK_RDM) {
		param->ai_protocol = IPPROTO_PGM;
#ifdef DEBUG
		spdlog::debug("param：ai_protocol is set to IPPROTO_PGM.");
#endif // DEBUG
	}
#endif // _WIN32	
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
#ifdef _WIN32
	//protocol = IGMPの場合
	if ((protocol_int == 2) && ((param->ai_family == AF_INET || param->ai_family == AF_INET6 || param->ai_family == AF_UNSPEC) && (param->ai_socktype == SOCK_RAW || param->ai_family == 0))) {
		param->ai_protocol = IPPROTO_IGMP;
#ifdef DEBUG
		spdlog::debug("param：ai_protocol is set to IPPROTO_IGMP.");
#endif // DEBUG
	}
#endif // _WIN32

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
#ifdef _WIN32
	//プロトコルをpgmに設定する場合
	else if ((protocol_int == 113) && param->ai_family == AF_INET && param->ai_socktype == SOCK_RDM) {
		param->ai_protocol = IPPROTO_PGM;
#ifdef DEBUG
		spdlog::debug("param：ai_protocol is set to IPPROTO_PGM.");
#endif // DEBUG	
	}
#endif // _WIN32
	else if (protocol_int == 0) {
		//param->ai_protocol == IPPROTO_UDP;
	}
	else {
		LOG_ERROR("Failed to set ai_protocol.");
		return false;
	}
	return true;
}


void SocketHandler::monitor_socket_descriptor(const std::vector<socket_t>& socket_list, std::function<void(socket_t)> callback)
{
	try {
		int select_result = 0;
#ifdef _WIN32
		select_result = select(0, &_fds, NULL, NULL, &_timeout);
#else	//LINUX, MACOS, UNIX系OSの場合
		auto max_ite = std::max_element(socket_list.begin(), socket_list.end());
		int max_socket_fd = 0;
		if (max_ite == socket_list.end()) {
			return; // ソケットリストが空の場合は何もしない
		}
		else {
			max_socket_fd = *max_ite;
		}
		select_result = select(max_socket_fd + 1, &_fds, NULL, NULL, &_timeout);										

#endif // _WIN32

		if (select_result < 0) {
#ifdef _WIN32
			int error_code = WSAGetLastError();
			std::string message = get_last_wsa_error_message();
			std::string error_message = "select failed. WSA error code: " + std::to_string(error_code) + ", error message:" + message + ".";
#else
			int error_code = errno;
			std::string error_message = "select failed with error code: " + std::to_string(error_code) + ", error message:" + std::string(strerror(error_code)) + ".";

#endif // _WIN32
			throw std::runtime_error(error_message);
		}
		else if (select_result == 0) {
			return; // タイムアウト
		}
		else {
			if (FD_ISSET(_tcp_listen_socket, &_fds)) {
				callback(_tcp_listen_socket); // 新しい接続リクエスト
			}
			for (const auto& socket : socket_list) {
				if (socket != INVALID_SOCKET_FD && FD_ISSET(socket, &_fds)) {
					callback(socket); // 変化のあったソケットを通知
				}
			}
		}
	}
	catch (const std::runtime_error& e) {
		LOG_ERROR(e.what());
	}
	catch (const std::exception& e) {
		LOG_ERROR(e.what());
	}
	catch (...)
	{
		LOG_ERROR("An unknown error occurred in monitor_socket_descriptor.");
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
		if (listen(_tcp_listen_socket, SOMAXCONN) == SOCKET_ERROR_FD) {
			throw std::runtime_error("Tcp Listen socket has been SOCKET_ERROR for some reason.");
			
		}
		else
		{
			spdlog::info("server open listen socket:{:d}.", _tcp_listen_socket);
		}
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(e.what());
		if (_result_tcp != nullptr) {
			freeaddrinfo(_result_tcp);
		}

		if (get_listen_socket() != INVALID_SOCKET_FD) {
#ifdef _WIN32
			closesocket(_tcp_listen_socket);
#else
			close(_tcp_listen_socket);
#endif
		}

		if (get_udp_multicast_tx_socket() != INVALID_SOCKET_FD) {
#ifdef _WIN32
			closesocket(_udp_multicast_tx_socket);
#else
			close(_udp_multicast_tx_socket);
#endif
		}

#ifdef _WIN32
		WSACleanup();
#endif


		return false;
	}
	catch (const std::exception& e)
	{
		LOG_ERROR(e.what());
		if (_result_tcp != nullptr) {
			freeaddrinfo(_result_tcp);
		}

		if (get_listen_socket() != INVALID_SOCKET_FD) {
#ifdef _WIN32
			closesocket(_tcp_listen_socket);
#else
			close(_tcp_listen_socket);
#endif
		}

		if (get_udp_multicast_tx_socket() != INVALID_SOCKET_FD) {
#ifdef _WIN32
			closesocket(_udp_multicast_tx_socket);
#else
			close(_udp_multicast_tx_socket);
#endif
		}

#ifdef _WIN32
		WSACleanup();
#endif
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
bool SocketHandler::check_valid_socket(socket_t request_socket)
{
	if (request_socket == INVALID_SOCKET_FD)
		return false;
	return true;
}

