#ifndef CHATSERVICEFACADE_HPP
#define CHATSERVICEFACADE_HPP
#include "Application/AppContainer.hpp"
#include "Utils/common.hpp"
#include "MessageBase.hpp"
#ifdef _WIN32
using socket_t = SOCKET;
#define INVALID_SOCKET_FD INVALID_SOCKET
#else
using socket_t = int; // POSIX systems use int for sockets
#define INVALID_SOCKET_FD -1
#endif // _WIN32

#ifdef __linux__
#define USE_POLL
#else
#define USE_SELECT
#endif

#define DISCONNECT_SOCKET 0
#define ACCEPT_FAILED 0
#define ACCEPT_SUCCEEDED 1
#define SOCKET_CONFIG_FILE_PATH "/app/ChatService/conf/socket_setting.conf"

class ChatServiceFacade
{
private:
	// AppContainerで依存関係を管理
	AppContainer& _app_container;

public:
	// コンストラクタ
	ChatServiceFacade();

	// デストラクタ
	~ChatServiceFacade();

	// サーバプログラム実行
	void run();
};

#endif // !CHATSERVICEFACADE_HPP
