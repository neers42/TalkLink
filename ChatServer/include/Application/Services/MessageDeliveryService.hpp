#ifndef MESSAGEDELIVERYSERVICE_HPP
#define MESSAGEDELIVERYSERVICE_HPP

#include "Application/Services/IMessageDeliveryService.hpp"
#include "Domain/CommunicationHandler.hpp"
#include "Domain/ChatroomRepositoryHandler.hpp"
#include "Domain/UserRepositoryHandler.hpp"
#include "Infrastructure/MySQLClient.hpp"
#include <memory>

class MessageDeliveryService : public IMessageDeliveryService {
public:
    MessageDeliveryService(
        std::shared_ptr<CommunicationHandler> communication_handler,
        std::shared_ptr<ChatroomRepositoryHandler> chatroom_repository_handler,
        std::shared_ptr<UserRepositoryHandler> user_repository_handler,
        std::shared_ptr<MySQLClient> mysql_client
    );
    
    ~MessageDeliveryService() override = default;
    
    // IMessageDeliveryService インターフェース実装
    DeliveryResult deliver_to_room(
        const json& message_json,
        const std::string& room_id,
        socket_t sender_socket,
        const std::string& sender_uid
    ) override;
    
    DeliveryResult deliver_p2p(
        const json& message_json,
        const std::string& target_uid
    ) override;
    
    DeliveryResult broadcast(
        const json& message_json,
        const std::string& sender_uid,
        bool exclude_sender = true
    ) override;
    
private:
    std::shared_ptr<CommunicationHandler> _communication_handler;
    std::shared_ptr<ChatroomRepositoryHandler> _chatroom_repository_handler;
    std::shared_ptr<UserRepositoryHandler> _user_repository_handler;
    std::shared_ptr<MySQLClient> _mysql_client;
    
    void handle_delivery_error(const std::exception& e, const std::string& operation);
};

#endif // MESSAGEDELIVERYSERVICE_HPP
