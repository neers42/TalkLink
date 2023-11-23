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

#define INIT_FILE_PASS  ".//ini/socket_setting.ini"
//#define MAX_CONNECT_NUM 10000
#define SELECT_TIME_OUT -2
#define REQUEST_ACCEPT_CLIENT_DETECTION -3
#define NOT_EXIST_IP_ADDRESS -1


class SocketHandler
{
public:

	//接続中のクライアント情報
	struct client_socket_info {
		//コンストラクタ
		client_socket_info(SOCKET socket, sockaddr_in sockaddr) :
			client_socket{ socket }, client_sockaadr_in{ sockaddr } {}

		//接続中のクライアントソケット
		SOCKET client_socket;

		//接続中のクライアントソケット情報
		struct sockaddr_in client_sockaadr_in;

		//接続中のクライアントソケットのサイズ
		socklen_t socket_addr_in_length = sizeof(client_sockaadr_in);

		std::string client_ip_addr = "";
	};



private:
/*
:::::::::::::::::::::::::::::::
             変数
:::::::::::::::::::::::::::::::
*/
	//リッスンソケット生成用のパラメータ
	struct addrinfo *m_result = NULL, m_param, m_multicast_tx_param, m_bloadcast_tx_param;

	WSADATA m_wsa_data;

	fd_set m_fds, m_fd_read;

	timeval m_timeout;

	//リッスンソケット
	SOCKET m_tcp_listen_socket;

	//UDPブロードキャスト送信用ソケット
	SOCKET m_udp_bloadcast_tx_socket;

	//UDPマルチキャスト送信用ソケット
	SOCKET m_udp_multicast_tx_socket;

	//サーバのアプリケーションポート番号
	int m_port = 0;

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
	

	//Listenソケットをbindする【未実装】
	bool bind_tcp_listen_socket();

	//クライアントリストの空いている箇所を探索
	int check_next_invalid_socket(const ClientHandler& client_handler);


	//ソケットが有効であるか判定する【未実装】
	bool check_valid_socket(SOCKET request_socket);

	//tcp listenソケットの設定を読み込む
	bool setting_tcp_listen_socket(const std::string& file_pass);

	//udpマルチキャスト送信用のソケット設定を読み込む
	bool setting_multicast_tx_socket(const std::string& file_pass);

	//udp ブロードキャスト送信用のソケット設定を読み込む
	bool setting_bloadcast_tx_socket(const std::string& file_pass);


	//Listenソケットを生成する
	bool generate_tcp_listen_socket();

	
	//クライアントの情報を初期化する
	void init_client_net_info(int client_num, ClientHandler* client_handler);

	void print_client_net_info(int client_num);


	//select関数の初期化
	void setting_select_info(SOCKET* socket, fd_set* fd, fd_set* fds, const ClientHandler& client_handler);

	bool generate_udp_multicast_tx_socket();

	bool generate_udp_bloadcast_tx_socket();

	bool obtain_local_ip_addr();

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

	
	SOCKET get_listen_socket() {
		return m_tcp_listen_socket;
	}

	SOCKET get_udp_multicast_tx_socket() {
		return m_udp_multicast_tx_socket;
	}

	//サーバのIPアドレスを表示する
	void show_server_info();

	//ファイルディスクリプタに変化がないか監視する
	int monitor_socket(const ClientHandler& client_handler);

	//リッスンソケットをアクティブにする
	bool open_tcp_listen_socket();

	//クライアントのIPアドレスを抽出する
	void extraction_client_ip(uint32_t client_num, ClientHandler::Client* new_client);


	std::tuple<SOCKET, sockaddr_in, socklen_t, uint32_t> accept_client_socket(ClientHandler& client_handler);

	//接続中のクライアント数を計算する【未実装】
	int search_connect_client_socket_num_from_ip(const ClientHandler& client_handler, const std::string& search_client_ip);

	//接続中のクライアント数を計算する【未実装】
	int search_connect_client_socket_num_from_id(const ClientHandler& client_handler, const std::string& search_client_id);
	
	//接続中のクライアントソケットをクローズする
	void disconnect_socket(ClientHandler& client_handler, int socket_num);

};

#endif // !SOCKETHANDLER_H_


