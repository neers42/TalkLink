#include "SocketHandler.h"

//リッスンソケット生成用のパラメータ
//struct addrinfo* param, *result;

//WSADATA wsa_data;

//fd_set fds, fd_read;

//timeval timeout;

//リッスンソケット
//SOCKET listen_socket;

//接続中のクライアントソケットリスト
//std::vector<SOCKET>* client_socket_list;


//デフォルトコンストラクタ
SocketHandler::SocketHandler() : param(NULL), result(NULL)
{
	//wsa_dataの初期化
	int init_result = WSAStartup(MAKEWORD(2, 0), &wsa_data);

	//ファイルディスクリプタキャッチ用のfdsを初期化
	FD_ZERO(&fds);
	FD_ZERO(&fd_read);

	//select関数のタイムアウト初期設定(tv_sec=秒の指定　tv_usec=マイクロ秒の指定)
	//デフォルトコンストラクタでは500マイクロ秒 * 1000　⇒ 500ミリ秒に設定
	timeout.tv_sec = 0;
	timeout.tv_usec = 500 * 1000;

	//リッスンソケットの初期化
	listen_socket = INVALID_SOCKET;

	//クライアントのリスト初期化
	client_socket_list->reserve(10000);
	client_socket_list->push_back(INVALID_SOCKET);


}

//コンストラクタ
SocketHandler::SocketHandler(int tv_sec, int tv_usec)
{

	//select関数のタイムアウト初期設定(tv_sec=秒の指定　tv_usec=マイクロ秒の指定)
	timeout.tv_sec = tv_sec;
	timeout.tv_usec = tv_usec;

	//wsa_dataの初期化
	int init_result = WSAStartup(MAKEWORD(2, 0), &wsa_data);


}

//デストラクタ
SocketHandler::~SocketHandler()
{
	WSACleanup();
}


void SocketHandler::bind_listen_socket()
{

}

void SocketHandler::accept_client_socket()
{
}

void SocketHandler::push_client_socket_list(const SOCKET new_client_socket)
{

}

int SocketHandler::cal_connect_client_socket_num(const SOCKET serach_socket)
{
	return 0;
}

void SocketHandler::close_socket(SOCKET request_close_socket)
{

}

void SocketHandler::check_valid_socket(SOCKET request_socket)
{

}

SOCKET SocketHandler::genarated_listen_socket()
{
	return SOCKET();
}

int SocketHandler::monitor_socket()
{
	return 0;
}
