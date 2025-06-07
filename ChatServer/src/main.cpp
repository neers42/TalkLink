#include <iostream>
#include "ChatServerFacade.h"
#include "spdlog/spdlog.h"
int main()
{
    std::unique_ptr<ChatServerFacade> chat_server_facade;
    chat_server_facade = std::make_unique<ChatServerFacade>();
    chat_server_facade->run();
    
}
