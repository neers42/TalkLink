#ifndef CHATSERVERFACADE_HPP
#define CHATSERVERFACADE_HPP
#include "Application/ServiceContainer.hpp"
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
#define SOCKET_CONFIG_FILE_PATH "/app/ChatServer/conf/socket_setting.conf"

class ChatServerFacade
{
private:
	// ServiceContainerで依存関係を管理
	ServiceContainer& _service_container;

public:
	// コンストラクタ
	ChatServerFacade();

	// デストラクタ
	~ChatServerFacade();

	// サーバプログラム実行
	void run();
};

#endif // !CHATSERVERFACADE_HPP
