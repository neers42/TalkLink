#include "Application/Services/ChatService.hpp"
#include "Domain/ChatroomSequenceHandler.hpp"
#include "MessageBase.hpp"

ChatService::ChatService(
    std::shared_ptr<IMessageDeliveryService> message_delivery_service,
    std::shared_ptr<UserRepositoryHandler> user_repository_handler,
    std::shared_ptr<ChatroomRepositoryHandler> chatroom_repository_handler,
    std::shared_ptr<ChatroomSequenceHandler> chatroom_sequence_handler
) : _message_delivery_service(std::move(message_delivery_service)),
    _user_repository_handler(std::move(user_repository_handler)),
    _chatroom_repository_handler(std::move(chatroom_repository_handler)),
    _chatroom_sequence_handler(std::move(chatroom_sequence_handler))
{
    if (!_message_delivery_service || !_user_repository_handler || !_chatroom_repository_handler || !_chatroom_sequence_handler) {
        throw std::runtime_error("All dependencies must be non-null");
    }
    
    LOG_INFO("[ChatService] ChatService initialized with interface-based dependencies");
}

ChatService::~ChatService() {
    LOG_INFO("[ChatService] Service shutting down!!");
}

// ==================== チャットメッセージ処理 ====================

IMessageDeliveryService::DeliveryResult ChatService::deliver_chat_message(const std::unique_ptr<ChatMessage>& chat_message, socket_t sender_socket) {
    IMessageDeliveryService::DeliveryResult delivery_result = {};
    
    try {
        // シーケンス番号を採番
        std::uint64_t room_sequence = 0;
        if (!_chatroom_sequence_handler->increment_room_sequence(chat_message->get_room_id(), room_sequence)) {
            LOG_ERROR("[ChatService] Failed to get room sequence for room ID: {}", chat_message->get_room_id());
            delivery_result.success = false;
            delivery_result.error_message = "Failed to get room sequence";
            return delivery_result;
        }
        
        // ChatDeliveryMessageを生成
        auto chat_delivery_message = std::make_unique<ChatDeliveryMessage>(
            chat_message->get_common_fields(),
            chat_message->get_message_id(),
            chat_message->get_room_id(),
            chat_message->get_sender_uid(),
            chat_message->get_content(),
            room_sequence
        );
        
        // ルームメンバーに配信
        const json& message_json = MessageBase::serialize_message<ChatDeliveryMessage>(chat_delivery_message);  
        delivery_result = _message_delivery_service->deliver_to_room(
            message_json, 
            chat_message->get_room_id(), 
            sender_socket, 
            chat_message->get_sender_uid()
        );
        delivery_result.room_sequence = room_sequence;
        if(delivery_result.success) {
            LOG_INFO("[ChatService] Chat message delivered successfully. Message ID: {}, Room ID: {}, Sender UID: {}", 
                        chat_message->get_message_id(), chat_message->get_room_id(), chat_message->get_sender_uid());
        } else {
            LOG_ERROR("[ChatService] Failed to deliver chat message. Message ID: {}, Room ID: {}, Sender UID: {}. Error: {}", 
                        chat_message->get_message_id(), chat_message->get_room_id(), chat_message->get_sender_uid(), delivery_result.error_message);
            //メッセージの再配信
            auto resend_result = this->execute_message_resend(message_json, delivery_result.unsent_uids);
                if(resend_result == true)
                {
                    LOG_INFO("[ChatService] Message resend succeeded for message ID: {}", chat_message->get_message_id());
                } else {
                    LOG_ERROR("[ChatService] Message resend failed for message ID: {}", chat_message->get_message_id());
                    delivery_result.error_message += " | Message resend failed";
                    delivery_result.success = false;
                }
        }
        LOG_INFO("[ChatService] Chat message delivered. Sent: {}, Failed: {}", 
                    delivery_result.sent_count, delivery_result.unsent_uids.size());
        
        return delivery_result;
        
    } catch (const std::exception& e) {
        handle_processing_error(e, "deliver_chat_message");
        delivery_result.success = false;
        delivery_result.error_message = e.what();
        return delivery_result;
    }
}

// チャットメッセージの確認応答処理
bool ChatService::send_chat_acknowledgment(
    const std::unique_ptr<ChatMessage>& chat_message,
    const IMessageDeliveryService::DeliveryResult& delivery_result,
    socket_t sender_socket
) {
    try {
        // 送信状態を決定
        ChatAckMessage::MessageSendStatus send_status;
        
        if (delivery_result.sent_count == 0 && delivery_result.unsent_uids.empty()) {
            send_status = ChatAckMessage::MessageSendStatus::SEND_FAILED;
        } else if (delivery_result.sent_count > 0 && delivery_result.unsent_uids.empty()) {
            send_status = ChatAckMessage::MessageSendStatus::SEND_SUCCESS;
        } else {
            send_status = ChatAckMessage::MessageSendStatus::SEND_PARTIAL_SUCCESS;
        }
        
        // ChatAckMessageを生成
        auto chat_ack_message = std::make_unique<ChatAckMessage>(
            chat_message->get_common_fields(),
            chat_message->get_message_id(),
            send_status,
            chat_message->get_room_id(),
            delivery_result.unsent_uids,
            delivery_result.room_sequence
        );
        
        // シリアライズして送信
        json chat_ack_json = MessageBase::serialize_message(chat_ack_message);
        auto send_result = _message_delivery_service->deliver_p2p(chat_ack_json, _user_repository_handler->get_uid_by_socket(sender_socket));
        
        if (send_result.success) {
            LOG_INFO("[ChatService] Chat acknowledgment sent to sender (socket: {})", sender_socket);
        } else {
            LOG_ERROR("[ChatService] Failed to send chat acknowledgment to sender (socket: {})", sender_socket);
        }
        
        return send_result.success;
        
    } catch (const std::exception& e) {
        handle_processing_error(e, "send_chat_acknowledgment");
        return false;
    }
}

// チャットメッセージの完全処理（処理→配信→確認応答）
bool ChatService::process_chat_message(
    const json& original_json,
    socket_t sender_socket
) 
{
    
    try {
        LOG_INFO("[ChatService] Processing chat message from socket: {}", sender_socket);
        // 1. JSONからChatMessageを生成
        auto chat_message = MessageBase::deserialize_message<ChatMessage>(original_json);
        if (!chat_message) {
            LOG_ERROR("[ChatService] Failed to create ChatMessage instance from JSON");
            return false;
        }

        // 2. メッセージを配信
        auto delivery_result = deliver_chat_message(chat_message, sender_socket);
        
        // 3. 確認応答を送信
        bool ack_sent = send_chat_acknowledgment(chat_message, delivery_result, sender_socket);
            if (!ack_sent) {
                //メッセージの再送信
                if(this->execute_message_resend(original_json, delivery_result.unsent_uids) == false)
                {
                    LOG_ERROR("[ChatService] Message resend failed during acknowledgment step for message ID: {}", chat_message->get_message_id());
                    return false;
                }
                LOG_INFO("[ChatService] Message resend succeeded during acknowledgment step for message ID: {}", chat_message->get_message_id());
            }
        
        // 4. メッセージをデータベースに保存（シーケンス採番不要、すでに deliver_chat_message で採番済み）
        if (!save_message_to_database(chat_message, original_json, delivery_result.room_sequence)) {
            return false;
        }
        return true;
        
    } catch (const std::exception& e) {
        handle_processing_error(e, "process_chat_message");
        return false;
    }
}


// ==================== ユーティリティメソッド ====================

json ChatService::get_unreceived_messages(const std::string& uid, uint64_t last_received_message_id) {
    try {
        return _chatroom_repository_handler->get_unreceived_messages(uid, last_received_message_id);
    } catch (const std::exception& e) {
        handle_processing_error(e, "get_unreceived_messages");
        return json::object();
    }
}

// ==================== 内部ヘルパーメソッド ====================

bool ChatService::save_message_to_database(const std::unique_ptr<ChatMessage>& chat_message, const json& original_json, std::uint64_t room_sequence) {
    try {
        // ルームIDの存在確認
        if (!_chatroom_repository_handler->is_room_id_exist(chat_message->get_room_id())) {
            LOG_ERROR("[ChatService] Room ID not found: {}", chat_message->get_room_id());
            return false;
        }
        
        // ISO 8601からMySQL形式に変換
        std::string mysql_timestamp = ServerTime::iso8601_to_mysql_datetime(chat_message->get_time_stamp());
        
        // データベースに保存
        bool save_success = _chatroom_repository_handler->insert_message(
            chat_message->get_message_id(),
            chat_message->get_room_id(),
            chat_message->get_sender_uid(),
            mysql_timestamp,
            original_json.dump(),
            room_sequence
        );
        
        if (save_success) {
            LOG_INFO("[ChatService] Message successfully saved to database. ID: {}", chat_message->get_message_id());
        } else {
            LOG_ERROR("[ChatService] Failed to save message to database. ID: {}", chat_message->get_message_id());
        }
        
        return save_success;
        
    } catch (const std::exception& e) {
        handle_processing_error(e, "save_message_to_database");
        return false;
    }
}

// メッセージの再送処理
bool ChatService::execute_message_resend(const json& message_json, const std::vector<std::string>& target_uids) {
    try {
        int unsent_count = 0;
        // 1秒待機
        std::this_thread::sleep_for(std::chrono::seconds(1));

        for (auto &uid : target_uids)
        {
            LOG_INFO("[ChatService] Attempting to resend message to user: {}", uid);
            // 再送試行
            for (int resend_count = 1; resend_count < MAX_RESEND_COUNT; resend_count++)
            {
                if (resend_count == 2)
                {
                    std::this_thread::sleep_for(std::chrono::seconds(3));
                }
                if (resend_count == 3)
                {
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                }
                auto resend_result = _message_delivery_service->deliver_p2p(message_json, uid);
                if (resend_result.success)
                {
                    //再送処理完了したらループを抜けて次のクライアントの処理を実施
                    LOG_DEBUG("[ChatService] Resend message to user {} succeeded on attempt {}", uid, resend_count);
                    break;
                }
                if(resend_result.success == false && resend_count == MAX_RESEND_COUNT - 1)
                {
                    LOG_WARNING("[ChatService] Resend message to user {} failed after {} attempts", uid, resend_count);
                    unsent_count++;
                }
            }
        }
        if(unsent_count > 0){
            LOG_ERROR("[ChatService] Message resend completed with some failures. Unsent: {}", unsent_count);
            return false;
        } else {
            LOG_INFO("[ChatService] Message resend completed successfully for all users.");
            return true;
        }
                     
    } catch (const std::exception& e) {
        handle_processing_error(e, "execute_message_resend");
        return false;
    }
}

bool ChatService::advance_room_sequence(const std::string& room_id)
{
    std::uint64_t next_sequence = 0;
    if (!_chatroom_sequence_handler->increment_room_sequence(room_id, next_sequence)) {
        LOG_ERROR("[ChatService] Failed to update room sequence. Room ID: {}", room_id);
        return false;
    }

    LOG_INFO("[ChatService] Room sequence updated. Room ID: {}, Sequence: {}", room_id, next_sequence);
    return true;
}

void ChatService::handle_processing_error(const std::exception& e, const std::string& operation_context) {
    LOG_ERROR("[ChatService] Error in {}: {}", operation_context, e.what());
    
    // TODO: エラーメトリクスの収集、アラート送信等
}