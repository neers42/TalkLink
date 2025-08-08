#include "ChatServerFacade.hpp"
int main()
{
    std::unique_ptr<ChatServerFacade> chat_server_facade;
    chat_server_facade = std::make_unique<ChatServerFacade>();
    chat_server_facade->run();
    return 0;
}
