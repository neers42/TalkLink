#ifndef SOCKETHANDLER_H_
#define SOCKETHANDLER_H_
#include <iostream>
#include <winsock2.h>
#include <vector>
#include <string>
#include <exception>
#include <ws2tcpip.h>
#include "inicpp.h"
#include "StringFormat.h"
#include "ServerTime.h"
#include <iphlpapi.h>
#include "LogHandler.h"
//#include "ClientHandler.h"
//#include "common.h"
#include "spdlog/spdlog.h"
#define INIT_FILE_PASS  ".//////conf/socket_setting.ini"
//#define MAX_CONNECT_NUM 10000
#define SELECT_TIME_OUT -2
#define REQUEST_ACCEPT_CLIENT_DETECTION -3
#define NOT_EXIST_IP_ADDRESS -1
#define MAX_CONNECT_NUM 50000

class SocketHandler
{

private:
/*
:::::::::::::::::::::::::::::::
             変数
:::::::::::::::::::::::::::::::
*/
	//リッスンソケット生成用のパラメータ
	struct addrinfo *_result_tcp , *_result_multicast , *_result_bloadcast;
	struct addrinfo _tcp_socket_param;		//TCPソケット生成用のパラメータ
	struct addrinfo _multicast_tx_param;	//UDPマルチキャスト送信用のソケット生成用のパラメータ
	struct addrinfo _bloadcast_tx_param;	//UDPブロードキャスト送信用のソケット生成用のパラメータ

	WSADATA _wsa_data;

	fd_set _fds, _fd_read;

	timeval _timeout;

	//リッスンソケット
	SOCKET _tcp_listen_socket;
	
	//UDPブロードキャスト送信用ソケット
	SOCKET _udp_bloadcast_tx_socket;

	//UDPマルチキャスト送信用ソケット
	SOCKET _udp_multicast_tx_socket;

	//サーバのアプリケーションポート番号
	int _port;
	
	//マルチキャスト用のsockaddr_in構造体
	struct sockaddr_in _multicast_tx_addr, _bloadcast_tx_addr;

	std::string _local_ip_addr;

	int _ttl;

	std::string setting_file_pass;

	std::vector<SOCKET> _connect_sockets;


/*
:::::::::::::::::::::::::::::::
			 関数
:::::::::::::::::::::::::::::::
*/
	
private:
	//Listenソケットをbindする
	bool bind_tcp_listen_socket(SOCKET* listen_socket, struct addrinfo* result);

	//tcp listenソケットの設定を読み込む
	bool setting_tcp_listen_socket_param();
	bool setting_tcp_listen_socket_param(const std::string& file_pass);

	//udpマルチキャスト送信用のソケット設定を読み込む
	bool setting_multicast_tx_socket_param();
	bool setting_multicast_tx_socket_param(const std::string& file_pass);

	//udp ブロードキャスト送信用のソケット設定を読み込む
	bool setting_bloadcast_tx_socket_param();
	bool setting_bloadcast_tx_socket_param(const std::string& file_pass);


	//Listenソケットを生成する
	bool generate_tcp_listen_socket();

	bool trans_flags(const std::string& flag_str, struct addrinfo* param);
	bool trans_family(const std::string& family_str, struct addrinfo* param);
	bool trans_socktype(const std::string& socktype_str, struct addrinfo* param);
	bool trans_protocol(const std::string& protocol_str, struct addrinfo* param);
	bool trans_protocol(const int protocol_int, struct addrinfo* param);


	//tcp_paramの設定用関数
	bool setting_tcp_flags(ini::IniFile socket_param_setting_ini);
	bool setting_tcp_family(ini::IniFile socket_param_setting_ini);
	bool setting_tcp_socktype(ini::IniFile socket_param_setting_ini);
	bool setting_tcp_protocol(ini::IniFile socket_param_setting_ini);
	bool setting_tcp_port(ini::IniFile socket_param_setting_ini);

	//muticast_tx_paramの設定用関数
	bool setting_multicast_tx_sock_af(ini::IniFile socket_param_setting_ini);
	bool setting_multicast_tx_sock_type(ini::IniFile socket_param_setting_ini);
	bool setting_multicast_tx_sock_protocol(ini::IniFile socket_param_setting_ini);
	bool setting_multicast_tx_sin_family(ini::IniFile socket_param_setting_ini);
	bool setting_multicast_tx_sin_port(ini::IniFile socket_param_setting_ini);
	bool setting_multicast_tx_sin_address(ini::IniFile socket_param_setting_ini);

public:
/*
:::::::::::::::::::::::::::::::
			 関数
:::::::::::::::::::::::::::::::
*/
	//コンストラクタ
	SocketHandler();

	SocketHandler(const std::string& file_pass);

	//デストラクタ
	~SocketHandler();

	
	SOCKET get_listen_socket() {
		return _tcp_listen_socket;
	}

	SOCKET get_udp_multicast_tx_socket() {
		return _udp_multicast_tx_socket;
	}

	fd_set& get_fd_read() {
		return _fd_read;
	}


	fd_set& get_fds(){
		return _fds;
	}

	std::string& get_server_ip_address() {
		return _local_ip_addr;
	}

	int get_port() {
		return _port;
	}


	//ファイルディスクリプタに変化がないか監視する
	void monitor_socket_descriptor(const std::vector<SOCKET>& socket_list, std::function<void(SOCKET)> callback);

	//メッセージ用のリッスンソケットをアクティブにする
	bool open_tcp_listen_socket();

	//クライアントのIPアドレスを抽出する
	std::string extraction_client_ip(struct sockaddr_in& client_sockaddr_in, size_t ipv4_addr_size);

	//accept処理(メッセージ用のソケット)
	std::tuple<SOCKET, sockaddr_in> accept_client_socket(socklen_t client_sockaddr_in_length);

	
	//接続中のクライアントソケットをクローズする
	void disconnect_socket(const SOCKET client_socket);

	//ソケットが有効であるか判定する【未実装】
	bool check_valid_socket(SOCKET request_socket);

	//select関数の初期化
	void setting_select_info_to_listen_socket();


	void bind_select_info_to_socket(const SOCKET& socket, fd_set* fd);
	
	void copy_fd_read_to_fds();

	void copy_fd_read_to_fds(fd_set* fds, fd_set* fd);

	std::string  obtain_local_ip_addr();

	bool generate_udp_multicast_tx_socket();

	bool generate_udp_bloadcast_tx_socket();

	bool add_connect_socket(const SOCKET socket);
	bool delete_connect_socket(const SOCKET socket);

	

};

#endif // !SOCKETHANDLER_H_


