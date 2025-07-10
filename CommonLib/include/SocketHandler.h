#ifndef SOCKETHANDLER_H_
#define SOCKETHANDLER_H_
#include <iostream>
#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <iphlpapi.h>
	using socket_t = SOCKET;
	#define INVALID_SOCKET_FD INVALID_SOCKET
	#define SOCKET_ERROR_FD SOCKET_ERROR
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <sys/select.h>
	#include <netdb.h>
	#include <ifaddrs.h>
	#include <shared_mutex>
	using socket_t = int;
	#define INVALID_SOCKET_FD -1
	#define SOCKET_ERROR_FD -1
#endif // _WIN32

#ifdef __linux__
	#include <sys/epoll.h>
	#include <fcntl.h>
	#define USE_POLL
	#define NEVENTS 16
#else
	#define USE_SELECT
#endif //__linux__

#include <vector>
#include <string>
#include <exception>
#include <algorithm>
#include "inicpp.h"
#include "StringFormat.h"
#include "ServerTime.h"

#include "LogHandler.h"
#include "spdlog/spdlog.h"
#define SELECT_TIME_OUT -2
#define REQUEST_ACCEPT_CLIENT_DETECTION -3
#define NOT_EXIST_IP_ADDRESS -1

class SocketHandler
{

private:
/*
:::::::::::::::::::::::::::::::
         private変数
:::::::::::::::::::::::::::::::
*/
	//リッスンソケット生成用のパラメータ
	struct addrinfo *_result_tcp , *_result_multicast , *_result_bloadcast;
	struct addrinfo _tcp_socket_param;		//TCPソケット生成用のパラメータ
	struct addrinfo _multicast_tx_param;	//UDPマルチキャスト送信用のソケット生成用のパラメータ
	struct addrinfo _bloadcast_tx_param;	//UDPブロードキャスト送信用のソケット生成用のパラメータ
	socket_t _tcp_listen_socket;				//TCPリッスンソケット	
	socket_t _udp_bloadcast_tx_socket;		//UDPブロードキャスト送信用ソケット
	socket_t _udp_multicast_tx_socket;		//UDPマルチキャスト送信用ソケット
	
	int _port;								//リッスンソケットのポート番号
	struct sockaddr_in _multicast_tx_addr;	//UDPマルチキャスト送信用のアドレス構造体
	struct sockaddr_in _bloadcast_tx_addr;	//UDPブロードキャスト用送信用のアドレス構造体
	std::string _local_ip_addr;
	int _ttl;								//UDPマルチキャスト送信用のTTL値
	std::string socket_config_file_pass;	//ソケット設定ファイルのパス
#ifdef USE_POLL
	//epoll file descriptor(epoll インスタンスのfd)
	int _epfd;
	//number of file descriptor(監視対象のソケットやファイルディスクリプタを登録・待機するためのfdの数)
	int _nfds;	
  	struct epoll_event listen_ev, _ev_ret[NEVENTS];
	std::shared_mutex smtx;
#elif defined(USE_SELECT)
	fd_set _fds, _fd_read;
	timeval _timeout;
#endif //USE_POLL

#ifdef _WIN32
	WSADATA _wsa_data;
#endif // _WIN32


/*
:::::::::::::::::::::::::::::::
	      private関数
:::::::::::::::::::::::::::::::
*/
	
private:
	//Listenソケットをbindする
	bool bind_tcp_listen_socket(int listen_socket, struct addrinfo* result);	//winsock版

	//tcp listenソケットの設定を読み込む
	bool set_socket_param();
	bool set_socket_param(const std::string& config_file_path);

	//udpマルチキャスト送信用のソケット設定を読み込む
	bool set_multicast_tx_socket_param();
	bool set_multicast_tx_socket_param(const std::string& config_file_path);

	//udp ブロードキャスト送信用のソケット設定を読み込む
	bool set_bloadcast_tx_socket_param();
	bool set_bloadcast_tx_socket_param(const std::string& file_pass);


	//Listenソケットを生成する
	bool generate_tcp_listen_socket();

	bool trans_flags(const std::string& flag_str, struct addrinfo* param);
	bool trans_family(const std::string& family_str, struct addrinfo* param);
	bool trans_socktype(const std::string& socktype_str, struct addrinfo* param);
	bool trans_protocol(const std::string& protocol_str, struct addrinfo* param);
	bool trans_protocol(const int protocol_int, struct addrinfo* param);


	//tcp_paramの設定用関数
	bool set_tcp_flags(ini::IniFile socket_param_config);
	bool set_tcp_family(ini::IniFile socket_param_config);
	bool set_tcp_socktype(ini::IniFile socket_param_config);
	bool set_tcp_protocol(ini::IniFile socket_param_config);
	bool set_tcp_port(ini::IniFile socket_param_config);

	//muticast_tx_paramの設定用関数
	bool set_multicast_tx_sock_af(ini::IniFile socket_param_config);
	bool set_multicast_tx_sock_type(ini::IniFile socket_param_config);
	bool set_multicast_tx_sock_protocol(ini::IniFile socket_param_config);
	bool set_multicast_tx_sin_family(ini::IniFile socket_param_config);
	bool set_multicast_tx_sin_port(ini::IniFile socket_param_config);
	bool set_multicast_tx_sin_address(ini::IniFile socket_param_config);

	

#ifdef _WIN32
	//error処理用の関数群
	std::string get_last_wsa_error_message();
#endif

public:
/*
:::::::::::::::::::::::::::::::
		  public関数
:::::::::::::::::::::::::::::::
*/
	//コンストラクタ
	SocketHandler();

	SocketHandler(const std::string& file_pass);

	//デストラクタ
	~SocketHandler();

	//getter関数
	socket_t get_listen_socket();
	socket_t get_udp_multicast_tx_socket();

#ifdef USE_SELECT
	fd_set& get_fd_read();
	fd_set& get_fds();
#endif
	std::string& get_server_ip_address();
	int get_port();

	//other
	//ファイルディスクリプタに変化がないか監視する
	void monitor_socket_descriptor(const std::vector<socket_t>& socket_list, std::function<void(socket_t)> callback);
	//メッセージ用のリッスンソケットをアクティブにする
	bool open_tcp_listen_socket();
	//クライアントのIPアドレスを抽出する
	std::string extraction_client_ip(struct sockaddr_in& client_sockaddr_in, size_t ipv4_addr_size);
	//accept処理(メッセージ用のソケット)
	std::tuple<socket_t, sockaddr_in> accept_client_socket(socklen_t client_sockaddr_in_length);
	//接続中のクライアントソケットをクローズする
	void close_socket(const socket_t client_socket);
	//ソケットが有効であるか判定する【未実装】
	bool check_valid_socket(socket_t request_socket);

#ifdef USE_SELECT
	//select関数の初期化
	void initialize_select_param();
	void bind_select_info_to_socket(socket_t socket, fd_set* fd);
	void copy_fd_read_to_fds();
	void copy_fd_read_to_fds(fd_set* fds, fd_set* fd);
#endif
	std::string  obtain_local_ip_addr();
	bool open_udp_multicast_tx_socket();
	bool open_udp_bloadcast_tx_socket();
	bool is_socket_closed(const socket_t socket_fd);
	//epoll関連
	void add_epoll_ctrl(socket_t socket);
	void delete_epoll_ctrl(socket_t socket);

};

#endif // !SOCKETHANDLER_H_


