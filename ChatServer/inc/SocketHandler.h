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

#define INIT_FILE_PASS  ".//ini/socket_setting.ini"
#define MAX_CONNECT_NUM 10000
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
	struct addrinfo *result = NULL, param, multicast_tx_param, bloadcast_tx_param;

	WSADATA wsa_data;

	fd_set fds, fd_read;

	timeval timeout;

	//リッスンソケット
	SOCKET tcp_listen_socket;

	//UDPブロードキャスト送信用ソケット
	SOCKET udp_bloadcast_tx_socket;

	//UDPブロードキャスト受信用ソケット
	SOCKET udp_bloadcast_rx_socket;

	//UDPマルチキャスト送信用ソケット
	SOCKET udp_multicast_tx_socket;

	//UDPマルチキャスト受信用ソケット
	SOCKET udp_multicast_rx_socket;

	//接続中のクライアントソケットリスト
	std::vector<SOCKET> client_socket_list;

	//接続中のクライアントリスト
	std::vector<client_socket_info> client_socket_info_list;

	//サーバのアプリケーションポート番号
	int port = 0;

	//初期化用のsockaddr_in構造体
	struct sockaddr_in init_sockaddr_in;

	//マルチキャスト用のsockaddr_in構造体
	struct sockaddr_in multicast_tx_addr, multicast_rx_addr,
					   bloadcast_tx_addr, bloadcast_rx_addr;

	//ローカルIPアドレス
	DWORD i_count;
	PMIB_IPADDRTABLE ip_addr_table;
	DWORD dw_size = 0;
	DWORD dw_ret_val = 0;
	std::string local_ip_addr = "";


/*
:::::::::::::::::::::::::::::::
			 関数
:::::::::::::::::::::::::::::::
*/
	

	//Listenソケットをbindする【未実装】
	bool bind_tcp_listen_socket();

	//クライアントリストの空いている箇所を探索
	int check_next_invalid_socket();

	//Acceptしたクライアントを接続リストに追加する【未実装】
	void push_client_socket_list(const SOCKET new_client_socket);

	//接続中のクライアント数を計算する【未実装】
	int search_connect_client_socket_num(const std::string search_ip_addr);

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

	//クライアントのIPアドレスを抽出する
	void extraction_client_ip(int client_num);

	//クライアントの情報を初期化する
	void init_client_net_info(int client_num);

	void print_client_net_info(int client_num);

	//select関数の初期化
	void set_select_info(SOCKET* socket, fd_set* fd, fd_set* fds);

	

	bool generate_udp_multicast_tx_socket();

	bool generate_udp_multicast_rx_socket();

	bool generate_udp_bloadcast_tx_socket();

	bool generate_udp_bloadcast_rx_socket();

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

	
	//【getter】接続中のクライアントリスト(参照)
	std::vector<SOCKET> get_client_socket_list() {
		return client_socket_list;
	}

	SOCKET get_listen_socket() {
		return tcp_listen_socket;
	}

	//サーバのIPアドレスを表示する
	void show_server_info();

	//ファイルディスクリプタに変化がないか監視する
	int monitor_socket();

	//リッスンソケットをアクティブにする
	bool open_tcp_listen_socket();

	//UDPマルチキャストの送信用ソケットをオープンにする。
	bool open_udp_muticast_tx_socket();

	//UDPマルチキャストの受信用ソケットをオープンにする。
	bool open_udp_multicast_rx_socket();

	//UDPブロードキャストの送信用ソケットをオープンにする。
	bool open_udp_bloadcast_tx_socket();

	//UDPブロードキャストの送信用ソケットをオープンにする。
	bool open_udp_bloadcast_rx_socket();

	//Accept処理
	bool accept_client_socket();

	std::vector<client_socket_info> get_client_socket_info_list() {
		return client_socket_info_list;
	}

	//接続中のクライアントソケットをクローズする【未実装】
	void disconnect_socket(int socket_num);

};

#endif // !SOCKETHANDLER_H_


