#ifndef SOCKETHANDLER_H_
#define SOCKETHANDLER_H_
#include <stdio.h>
#include <winsock2.h>
#include <vector>
#include <string>
#include "inicpp.h"

#define INIT_FILE_PASS : ".//ini/socket_setting.ini"

class SocketHandler
{

private:
/*
:::::::::::::::::::::::::::::::
             変数
:::::::::::::::::::::::::::::::
*/
	//リッスンソケット生成用のパラメータ
	struct addrinfo* param, *result;

	WSADATA wsa_data;

	fd_set fds, fd_read;

	timeval timeout;

	//リッスンソケット
	SOCKET listen_socket;

	//接続中のクライアントソケットリスト
	std::vector<SOCKET>* client_socket_list;

/*
:::::::::::::::::::::::::::::::
			 関数
:::::::::::::::::::::::::::::::
*/
	void load_socket_setting_ini_file(const std::string file_pass);

	//Listenソケットをbindする【未実装】
	void bind_listen_socket();

	//Accept処理【未実装】
	void accept_client_socket();

	//Acceptしたクライアントを接続リストに追加する【未実装】
	void push_client_socket_list(const SOCKET new_client_socket);

	//接続中のクライアント数を計算する【未実装】
	int cal_connect_client_socket_num(const SOCKET serach_socket);

	//接続中のクライアントソケットをクローズする【未実装】
	void close_socket(SOCKET request_close_socket);

	//ソケットが有効であるか判定する【未実装】
	void check_valid_socket(SOCKET request_socket);



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
	std::vector<SOCKET>* get_client_socket_list() {
		return client_socket_list;
	}

	//Listenソケットを生成する【未実装】
	SOCKET genarated_listen_socket();

	//ファイルディスクリプタに変化がないか監視する【未実装】
	int monitor_socket();

};

#endif // !SOCKETHANDLER_H_


