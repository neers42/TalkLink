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

class SocketHandler
{

private:
/*
:::::::::::::::::::::::::::::::
             変数
:::::::::::::::::::::::::::::::
*/
	//リッスンソケット生成用のパラメータ
	struct addrinfo *result = NULL;
	struct addrinfo param;

	WSADATA wsa_data;

	fd_set fds, fd_read;

	timeval timeout;

	//リッスンソケット
	SOCKET listen_socket;

	//接続中のクライアントソケットリスト
	std::vector<SOCKET> client_socket_list;

	//接続中のクライアント情報
	struct client_net_info {
		client_net_info(SOCKET socket, sockaddr_in sockaddr) : 
			client_socket{ socket }, client_sockaadr_in{ sockaddr } {}
		SOCKET client_socket;
		struct sockaddr_in client_sockaadr_in;
	};

	//接続中のクライアントリスト
	std::vector<client_net_info> client_net_info_list;

	//サーバのアプリケーションポート番号
	int port = 0;

	sockaddr_in init_sockaddr_in;

/*
:::::::::::::::::::::::::::::::
			 関数
:::::::::::::::::::::::::::::::
*/
	

	//Listenソケットをbindする【未実装】
	bool bind_listen_socket();

	//Accept処理【未実装】
	bool accept_client_socket();

	int check_next_invalid_socket();


	//Acceptしたクライアントを接続リストに追加する【未実装】
	void push_client_socket_list(const SOCKET new_client_socket);

	//接続中のクライアント数を計算する【未実装】
	int search_connect_client_socket_num(const SOCKET serach_socket);

	//接続中のクライアントソケットをクローズする【未実装】
	void close_socket(SOCKET request_close_socket);

	//ソケットが有効であるか判定する【未実装】
	bool check_valid_socket(SOCKET request_socket);

	bool load_socket_setting_ini_file(const std::string& file_pass);

	std::string get_client_ip(SOCKET client_socket);



public:
/*
:::::::::::::::::::::::::::::::
			 関数
:::::::::::::::::::::::::::::::
*/
	//コンストラクタ
	SocketHandler();

	//コンストラクタ
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


	//Listenソケットを生成する【未実装】
	bool generate_listen_socket();

	//ファイルディスクリプタに変化がないか監視する【未実装】
	int monitor_socket();

};

#endif // !SOCKETHANDLER_H_


