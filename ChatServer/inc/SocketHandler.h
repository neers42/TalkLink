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

#define INIT_FILE_PASS  ".//ini/socket_setting.ini"
#define DEFFAULT_PORT "50000"
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
	struct addrinfo *result = NULL, param;

	WSADATA wsa_data;

	fd_set fds, fd_read;

	timeval timeout;

	//リッスンソケット
	SOCKET listen_socket;

	//接続中のクライアントソケットリスト
	std::vector<SOCKET> client_socket_list;

	//接続中のクライアントリスト
	std::vector<client_socket_info> client_socket_info_list;

	//サーバのアプリケーションポート番号
	int port = 0;

	//初期化用のアドレス
	sockaddr_in init_sockaddr_in;


/*
:::::::::::::::::::::::::::::::
			 関数
:::::::::::::::::::::::::::::::
*/
	

	//Listenソケットをbindする【未実装】
	bool bind_listen_socket();

	//クライアントリストの空いている箇所を探索
	int check_next_invalid_socket();

	//Acceptしたクライアントを接続リストに追加する【未実装】
	void push_client_socket_list(const SOCKET new_client_socket);

	//接続中のクライアント数を計算する【未実装】
	int search_connect_client_socket_num(const std::string search_ip_addr);

	

	//ソケットが有効であるか判定する【未実装】
	bool check_valid_socket(SOCKET request_socket);

	bool load_socket_setting_ini_file(const std::string& file_pass);

	//クライアントのIPアドレスを抽出する
	void extraction_client_ip(int client_num);

	//クライアントの情報を初期化する
	void init_client_net_info(int client_num);

	void print_client_net_info(int client_num);

	//select関数の初期化
	void set_select_info(SOCKET* socket, fd_set* fd, fd_set* fds);

	//サーバのIPアドレスを表示する
	void print_server_ip();


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
		return listen_socket;
	}


	//Listenソケットを生成する
	bool generate_listen_socket();

	//ファイルディスクリプタに変化がないか監視する
	int monitor_socket();

	//リッスンソケットをアクティブにする
	void active_listen_socket();

	//Accept処理
	bool accept_client_socket();

	std::vector<client_socket_info> get_client_socket_info_list() {
		return client_socket_info_list;
	}

	//接続中のクライアントソケットをクローズする【未実装】
	void disconnect_socket(int socket_num);

};

#endif // !SOCKETHANDLER_H_


