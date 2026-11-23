#ifndef MESSAGESENDER_HPP
#define MESSAGESENDER_HPP

#include "Application/Interfaces/IMessageSender.hpp"
#include "Application/DTO/DeliveryResult.hpp"

class CommunicationHandler;
class ChatroomRepositoryHandler;
class UserRepositoryHandler;

class MessageSender : public IMessageSender {
public:
    MessageSender(
        std::shared_ptr<CommunicationHandler> communication_handler,
        std::shared_ptr<ChatroomRepositoryHandler> chatroom_repository_handler,
        std::shared_ptr<UserRepositoryHandler> user_repository_handler
    );
    
    ~MessageSender() override = default;
    
    // IMessageSender インターフェース実装
    DeliveryResult deliver_to_room(
        const json& message_json,
        const std::string& room_id,
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
    
    void handle_delivery_error(const std::exception& e, const std::string& operation);
};

#endif // MESSAGESENDER_HPP
