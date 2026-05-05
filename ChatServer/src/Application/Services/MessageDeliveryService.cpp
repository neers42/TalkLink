#include "Application/Services/MessageDeliveryService.hpp"

MessageDeliveryService::MessageDeliveryService(
    std::shared_ptr<ClientCommunicationHandler> client_communication_handler,
    std::shared_ptr<ChatroomRepositoryHandler> chatroom_repository_handler,
    std::shared_ptr<ClientRepositoryHandler> client_repository_handler,
    std::shared_ptr<MySQLClient> mysql_client
) :
    _client_communication_handler(std::move(client_communication_handler)),
    _chatroom_repository_handler(std::move(chatroom_repository_handler)),
    _client_repository_handler(std::move(client_repository_handler)),
    _mysql_client(std::move(mysql_client))
{
    if (!_client_communication_handler || !_chatroom_repository_handler || 
        !_client_repository_handler || !_mysql_client) {
        throw std::invalid_argument("All dependencies must be non-null for MessageDeliveryService");
    }
    
    LOG_INFO("MessageDeliveryService initialized successfully");
}

IMessageDeliveryService::DeliveryResult MessageDeliveryService::deliver_to_room(
    const json& message_json,
    const std::string& room_id,
    socket_t sender_socket,
    const std::string& sender_client_id
) {
    DeliveryResult result;
    
    try {
        if (sender_socket == INVALID_SOCKET_FD) {
            LOG_ERROR("Sender with ID '{}' not found. Cannot send message.", sender_client_id);
            result.success = false;
            result.error_message = "Sender not found";
            return result;
        }
        
        // ルームメンバー取得
        auto receivers_cids = _chatroom_repository_handler->get_client_infos_by_chatroom_id(room_id);
        
        if (receivers_cids.empty()) {
            LOG_WARNING("No members found in room: {}", room_id);
            result.success = true; // エラーではない
            return result;
        }
        
        // 各メンバーにメッセージ送信
        for (const auto& cid : receivers_cids) {
            auto search_client = _client_repository_handler->get_client_by_id(cid);
            
            if (search_client == nullptr) {
                // 接続していないクライアントはスキップ
                result.unsent_client_ids.push_back(cid);
                continue;
            }
            
            if (cid == sender_client_id) {
                // 送信者自身はスキップ
                continue;
            }
            
            // メッセージ送信
            bool send_success = _client_communication_handler->send_message(_client_repository_handler->get_socket_by_client_id(cid), message_json);
            
            if (send_success) {
                result.sent_count++;
                LOG_DEBUG("[ChatService] Message sent successfully to client: {}", cid);
            } else {
                result.unsent_client_ids.push_back(cid);
                LOG_WARNING("[ChatService] Failed to send message to client: {}", cid);
            }
        }
        if(result.unsent_client_ids.size() > 0){
            result.success = false;
            result.error_message = "Failed to send message to some clients, unsent_client_ids: " + std::to_string(result.unsent_client_ids.size());
            LOG_WARNING("[ChatService] Message delivery completed with some failures. Sent: {}, Unsent: {}", result.sent_count, result.unsent_client_ids.size());
        } else {
            result.success = true;
            LOG_INFO("[ChatService] Message delivery completed. Sent: {}, Unsent: {}", result.sent_count, result.unsent_client_ids.size());
        }
        
        
        return result;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[MessageDeliveryService] Exception in deliver_to_room: {}", e.what());
        handle_delivery_error(e, "deliver_to_room");
        result.success = false;
        result.error_message = e.what();
        return result;
    }
}

IMessageDeliveryService::DeliveryResult MessageDeliveryService::deliver_p2p(
    const json& message_json,
    const std::string& target_client_id
) {
    DeliveryResult result;
    
    try {
        // ターゲットクライアント取得
        auto target_client = _client_repository_handler->get_client_by_id(target_client_id);
        
        if (target_client == nullptr) {
            LOG_WARNING("Target client '{}' not found for P2P message", target_client_id);
            result.unsent_client_ids.push_back(target_client_id);
            result.success = false;
            return result;
        }
        
        // メッセージ送信
        bool send_success = _client_communication_handler->send_message(_client_repository_handler->get_socket_by_client_id(target_client_id), message_json);
        
        if (send_success) {
            result.sent_count = 1;
            LOG_DEBUG("P2P message sent successfully to client: {}", target_client_id);
        } else {
            result.unsent_client_ids.push_back(target_client_id);
            LOG_WARNING("Failed to send P2P message to client: {}", target_client_id);
        }
        
        result.success = send_success;
        return result;
        
    } catch (const std::exception& e) {
        handle_delivery_error(e, "deliver_p2p");
        result.success = false;
        result.error_message = e.what();
        return result;
    }
}

IMessageDeliveryService::DeliveryResult MessageDeliveryService::broadcast(
    const json& message_json,
    const std::string& sender_client_id,
    bool exclude_sender
) {
    DeliveryResult result;
    
    try {
        // 全クライアント取得
        auto all_clients = _client_repository_handler->get_all_clients();
        
        // 各クライアントにメッセージ送信
        for (const auto& client : all_clients) {
            const auto& cid = _client_repository_handler->get_client_id(client.first);
            
            // 送信者除外チェック
            if (exclude_sender && cid == sender_client_id) {
                continue;
            }
            
            // メッセージ送信
            bool send_success = _client_communication_handler->send_message(_client_repository_handler->get_socket_by_client_id(cid), message_json);
            
            if (send_success) {
                result.sent_count++;
                LOG_DEBUG("Broadcast message sent successfully to client: {}", cid);
            } else {
                result.unsent_client_ids.push_back(cid);
                LOG_WARNING("Failed to send broadcast message to client: {}", cid);
            }
        }
        
        result.success = (result.unsent_client_ids.empty());
        LOG_INFO("Broadcast delivery completed. Sent: {}, Unsent: {}", result.sent_count, result.unsent_client_ids.size());
        return result;
        
    } catch (const std::exception& e) {
        handle_delivery_error(e, "broadcast");
        result.success = false;
        result.error_message = e.what();
        return result;
    }
}

void MessageDeliveryService::handle_delivery_error(const std::exception& e, const std::string& operation) {
    LOG_ERROR("[MessageDeliveryService] Exception in {}: {}", operation, e.what());
}