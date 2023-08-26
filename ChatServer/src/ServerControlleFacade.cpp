#include "ServerControlleFacade.h"

ServerControlleFacade::ServerControlleFacade()
{

}

ServerControlleFacade::~ServerControlleFacade()
{
	delete m_socket_handler;
}

//サーバプログラムの実行
void ServerControlleFacade::run()
{
	m_socket_handler->generate_listen_socket();
}
