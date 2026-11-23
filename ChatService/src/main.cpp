#include "ChatServiceFacade.hpp"
int main()
{
    std::unique_ptr<ChatServiceFacade> chat_service_facade;
    chat_service_facade = std::make_unique<ChatServiceFacade>();
    chat_service_facade->run();
    return 0;
}
