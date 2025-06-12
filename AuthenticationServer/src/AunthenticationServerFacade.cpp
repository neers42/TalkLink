#include "AunthenticationServerFacade.h"

AunthenticationServerFacade::AunthenticationServerFacade() {

	////ログのフォーマット設定
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread id : %t] [%^%l%$] %v");
	spdlog::info("Starting MyChat server version 1.0.0!!");
	_socket_handler = std::make_unique<SocketHandler>("conf/socket_setting.ini");
}

AunthenticationServerFacade::~AunthenticationServerFacade()
{
}

void AunthenticationServerFacade::run() {
	//リッスンソケットの生成
	if (_socket_handler->open_tcp_listen_socket() == false) {
		LOG_ERROR("The TCP listening socket for message could not be opened properly, so the program is forcibly terminated. Please set it again.");
		exit(1);
	}

	if (_socket_handler->generate_udp_multicast_tx_socket() == false) {
		LOG_ERROR("The program terminates because it failed to create a socket for multicast transmission. Please reset the settings.");
		exit(1);
	}
}
