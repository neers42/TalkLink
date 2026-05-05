#ifndef MESSAGEDELIVERYSERVICE_HPP
#define MESSAGEDELIVERYSERVICE_HPP

#include "Application/Services/IMessageDeliveryService.hpp"
#include "Domain/ClientCommunicationHandler.hpp"
#include "Domain/ChatroomRepositoryHandler.hpp"
#include "Domain/ClientRepositoryHandler.hpp"
#include "Infrastructure/MySQLClient.hpp"
#include <memory>

class MessageDeliveryService : public IMessageDeliveryService {
public:
    MessageDeliveryService(
        std::shared_ptr<ClientCommunicationHandler> client_communication_handler,
        std::shared_ptr<ChatroomRepositoryHandler> chatroom_repository_handler,
        std::shared_ptr<ClientRepositoryHandler> client_repository_handler,
        std::shared_ptr<MySQLClient> mysql_client
    );
    
    ~MessageDeliveryService() override = default;
    
    // IMessageDeliveryService インターフェース実装
    DeliveryResult deliver_to_room(
        const json& message_json,
        const std::string& room_id,
        socket_t sender_socket,
        const std::string& sender_client_id
    ) override;
    
    DeliveryResult deliver_p2p(
        const json& message_json,
        const std::string& target_client_id
    ) override;
    
    DeliveryResult broadcast(
        const json& message_json,
        const std::string& sender_client_id,
        bool exclude_sender = true
    ) override;
    
private:
    std::shared_ptr<ClientCommunicationHandler> _client_communication_handler;
    std::shared_ptr<ChatroomRepositoryHandler> _chatroom_repository_handler;
    std::shared_ptr<ClientRepositoryHandler> _client_repository_handler;
    std::shared_ptr<MySQLClient> _mysql_client;
    
    void handle_delivery_error(const std::exception& e, const std::string& operation);
};

#endif // MESSAGEDELIVERYSERVICE_HPP
