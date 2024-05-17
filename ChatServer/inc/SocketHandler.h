#ifndef SOCKETHANDLER_H_
#define SOCKETHANDLER_H_
#include <iostream>
#include <winsock2.h>
#include <vector>
#include <string>
#include <exception>
#include <ws2tcpip.h>
#include "inicpp.h"
#include "AppException.h"
#include "StringFormat.h"
#include "ServerTime.h"
#include <iphlpapi.h>
#include "LogHandler.h"
#include "ClientHandler.h"
#include "common.h"

#define INIT_FILE_PASS  ".//ini/socket_setting.ini"
//#define MAX_CONNECT_NUM 10000
#define SELECT_TIME_OUT -2
#define REQUEST_ACCEPT_CLIENT_DETECTION -3
#define NOT_EXIST_IP_ADDRESS -1



class SocketHandler
{

private:
/*
:::::::::::::::::::::::::::::::
             変数
:::::::::::::::::::::::::::::::
*/
	//リッスンソケット生成用のパラメータ
	struct addrinfo *m_result_msg = NULL, m_param_msg, m_multicast_tx_param, m_bloadcast_tx_param;
	struct addrinfo *m_result_hb = NULL, m_param_hb;

	WSADATA m_wsa_data;

	fd_set m_fds_msg, m_fd_read_msg;
	fd_set m_fds_hb, m_fd_read_hb;

	timeval m_timeout;

	//リッスンソケット
	SOCKET m_tcp_listen_for_msg_socket;

	SOCKET m_tcp_listen_for_hb_socket;

	//UDPブロードキャスト送信用ソケット
	SOCKET m_udp_bloadcast_tx_socket;

	//UDPマルチキャスト送信用ソケット
	SOCKET m_udp_multicast_tx_socket;

	//サーバのアプリケーションポート番号
	int m_message_port = 0;

	int m_heart_beat_port = 0;

	//初期化用のsockaddr_in構造体
	struct sockaddr_in m_init_sockaddr_in;
	
	//マルチキャスト用のsockaddr_in構造体
	struct sockaddr_in m_multicast_tx_addr, m_bloadcast_tx_addr;

	//ローカルIPアドレス
	DWORD m_i_count;
	PMIB_IPADDRTABLE m_ip_addr_table;
	DWORD m_dw_size = 0;
	DWORD m_dw_ret_val = 0;
	std::string m_local_ip_addr = "";

	int m_ttl = 0;


/*
:::::::::::::::::::::::::::::::
			 関数
:::::::::::::::::::::::::::::::
*/
	

	//Listenソケットをbindする
	bool bind_tcp_listen_socket(SOCKET* listen_socket, struct addrinfo* result);

	//tcp listenソケットの設定を読み込む
	bool setting_tcp_listen_socket_param(const std::string& file_pass);

	//udpマルチキャスト送信用のソケット設定を読み込む
	bool setting_multicast_tx_socket(const std::string& file_pass);

	//udp ブロードキャスト送信用のソケット設定を読み込む
	bool setting_bloadcast_tx_socket(const std::string& file_pass);


	//Listenソケットを生成する
	bool generate_tcp_listen_socket_msg();

	bool generate_tcp_listen_socket_heart_beat();

	bool generate_udp_multicast_tx_socket();

	bool generate_udp_bloadcast_tx_socket();

	bool setting_flags(ini::IniFile tcp_listen_socket_param_setting_ini);

	bool setting_family(ini::IniFile tcp_listen_socket_param_setting_ini);

	bool setting_socktype(ini::IniFile tcp_listen_socket_param_setting_ini);

	bool setting_protocol(ini::IniFile tcp_listen_socket_param_setting_ini);

	bool setting_port_for_message(ini::IniFile tcp_listen_socket_param_setting_ini);

	bool setting_port_for_heart_beat(ini::IniFile tcp_listen_socket_param_setting_ini);


public:
/*
:::::::::::::::::::::::::::::::
			 関数
:::::::::::::::::::::::::::::::
*/
	//コンストラクタ
	SocketHandler();

	//コピーコンストラクタ
	SocketHandler(int tv_sec, int tv_usec);

	//デストラクタ
	~SocketHandler();

	
	SOCKET get_listen_socket_for_msg() {
		return m_tcp_listen_for_msg_socket;
	}

	SOCKET get_listen_socket_for_hb() {
		return m_tcp_listen_for_hb_socket;
	}

	SOCKET get_udp_multicast_tx_socket() {
		return m_udp_multicast_tx_socket;
	}

	fd_set& get_fd_read_msg() {
		return m_fd_read_msg;
	}

	fd_set& get_fd_read_hb() {
		return m_fd_read_hb;
	}

	fd_set& get_fds_msg(){
		return m_fds_msg;
	}

	fd_set& get_fds_hb() {
		return m_fds_hb;
	}
	
	

	std::string& get_server_ip_address() {
		return m_local_ip_addr;
	}


	//ファイルディスクリプタに変化がないか監視する
	int monitor_descriptor_for_message(const ClientHandler& client_handler);

	//メッセージ用のリッスンソケットをアクティブにする
	bool open_tcp_listen_socket_for_message();

	//ハートビート用のリッスンソケットをアクティブにする
	bool open_tcp_listen_socket_for_heart_beat();

	//クライアントのIPアドレスを抽出する
	std::string extraction_client_ip(struct sockaddr_in& client_sockaddr_in, size_t ipv4_addr_size);

	//accept処理(メッセージ用のソケット)
	std::tuple<SOCKET, sockaddr_in> accept_client_socket_msg(int check_next_invalid_result, socklen_t client_sockaddr_in_length);

	//accept処理(ハートビート用のソケット)
	std::tuple<SOCKET, sockaddr_in> accept_client_socket_hb(socklen_t client_sockaddr_in_hb_length);

	//接続中のクライアントソケットをクローズする
	void disconnect_socket(SOCKET client_socket, std::string client_ip_address, int monitor_result);

	//ソケットが有効であるか判定する【未実装】
	bool check_valid_socket(SOCKET request_socket);

	//select関数の初期化
	void setting_select_info_to_listen_socket_for_msg();

	void setting_select_info_to_listen_socket_for_hb();

	void bind_select_info_to_socket(const SOCKET& socket, fd_set* fd);
	
	void copy_fd_read_to_fds();

	void copy_fd_read_to_fds(fd_set* fds, fd_set* fd);

	bool obtain_local_ip_addr();
	

};

#endif // !SOCKETHANDLER_H_


