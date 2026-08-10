#include "Application/Services/MessageDeliveryService.hpp"

MessageDeliveryService::MessageDeliveryService(
    std::shared_ptr<CommunicationHandler> communication_handler,
    std::shared_ptr<ChatroomRepositoryHandler> chatroom_repository_handler,
    std::shared_ptr<UserRepositoryHandler> user_repository_handler
) :
    _communication_handler(std::move(communication_handler)),
    _chatroom_repository_handler(std::move(chatroom_repository_handler)),
    _user_repository_handler(std::move(user_repository_handler))
{
    if (!_communication_handler || !_chatroom_repository_handler || 
        !_user_repository_handler) {
        throw std::invalid_argument("All dependencies must be non-null for MessageDeliveryService");
    }
    
    LOG_INFO("[MessageDeliveryService] MessageDeliveryService initialized successfully");
}

IMessageDeliveryService::DeliveryResult MessageDeliveryService::deliver_to_room(
    const json& message_json,
    const std::string& room_id,
    socket_t sender_socket,
    const std::string& sender_uid
) {
    DeliveryResult result;
    
    try {
        if (sender_socket == INVALID_SOCKET_FD) {
            LOG_ERROR("[MessageDeliveryService] Sender with UID '{}' not found. Cannot send message.", sender_uid);
            result.success = false;
            result.error_message = "Sender not found";
            return result;
        }
        
        // ルームメンバー取得
        auto receivers_uids = _chatroom_repository_handler->get_user_infos_by_chatroom_id(room_id);
        
        //現在接続中であるメンバーがいない場合はエラーではないため、成功とする
        if (receivers_uids.empty()) {
            LOG_WARNING("[MessageDeliveryService] No members for connection found in room: {}", room_id);
            result.success = true; // エラーではない
            return result;
        }
        
        // 各メンバーにメッセージ送信
        for (const auto& uid : receivers_uids) {
            auto search_user = _user_repository_handler->get_user_by_id(uid);
            
            if (search_user == nullptr) {
                // 接続していないクライアントはスキップ
                result.unsent_uids.push_back(uid);
                continue;
            }
            
            if (uid == sender_uid) {
                // 送信者自身はスキップ
                continue;
            }
            
            // メッセージ送信
            bool send_success = _communication_handler->send_message(_user_repository_handler->get_socket_by_uid(uid), message_json);
            
            if (send_success) {
                result.sent_count++;
                LOG_DEBUG("[MessageDeliveryService] Message sent successfully to user: {}", uid);
            } else {
                result.unsent_uids.push_back(uid);
                LOG_WARNING("[MessageDeliveryService] Failed to send message to user: {}", uid);
            }
        }
        if(result.unsent_uids.size() > 0){
            result.success = false;
            result.error_message = "Failed to send message to some users, unsent_uids: " + std::to_string(result.unsent_uids.size());
            LOG_WARNING("[MessageDeliveryService] Message delivery completed with some failures. Sent: {}, Unsent: {}", result.sent_count, result.unsent_uids.size());
        } else {
            result.success = true;
            LOG_INFO("[MessageDeliveryService] Message delivery completed. Sent: {}, Unsent: {}", result.sent_count, result.unsent_uids.size());
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
    const std::string& target_uid
) {
    DeliveryResult result;
    
    try {
        // ターゲットユーザー取得
        auto target_user = _user_repository_handler->get_user_by_id(target_uid);
        
        if (target_user == nullptr) {
            LOG_WARNING("[MessageDeliveryService] Target user '{}' not found for P2P message", target_uid);
            result.unsent_uids.push_back(target_uid);
            result.success = false;
            return result;
        }
        
        // メッセージ送信
        bool send_success = _communication_handler->send_message(_user_repository_handler->get_socket_by_uid(target_uid), message_json);
        
        if (send_success) {
            result.sent_count = 1;
            LOG_DEBUG("[MessageDeliveryService] P2P message sent successfully to user: {}", target_uid);
        } else {
            result.unsent_uids.push_back(target_uid);
            LOG_WARNING("[MessageDeliveryService] Failed to send P2P message to user: {}", target_uid);
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
    const std::string& sender_uid,
    bool exclude_sender
) {
    DeliveryResult result;
    
    try {
        // 全ユーザー取得
        auto all_users = _user_repository_handler->get_all_users();
        
        // 各ユーザーにメッセージ送信
        for (const auto& user : all_users) {
            const auto& uid = _user_repository_handler->get_uid_by_socket(user.first);
            
            // 送信者除外チェック
            if (exclude_sender && uid == sender_uid) {
                continue;
            }
            
            // メッセージ送信
            bool send_success = _communication_handler->send_message(_user_repository_handler->get_socket_by_uid(uid), message_json);
            
            if (send_success) {
                result.sent_count++;
                LOG_DEBUG("[MessageDeliveryService] Broadcast message sent successfully to user: {}", uid);
            } else {
                result.unsent_uids.push_back(uid);
                LOG_WARNING("[MessageDeliveryService] Failed to send broadcast message to user: {}", uid);
            }
        }
        
        result.success = (result.unsent_uids.empty());
        LOG_INFO("[MessageDeliveryService] Broadcast delivery completed. Sent: {}, Unsent: {}", result.sent_count, result.unsent_uids.size());
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