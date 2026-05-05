#include "Application/Services/ChatroomManagementService.hpp"

ChatroomManagementService::ChatroomManagementService(
    std::shared_ptr<MySQLClient> mysql_client,
    std::shared_ptr<RedisClient> redis_client,
    std::shared_ptr<IMessageDeliveryService> message_delivery_service,
    std::shared_ptr<ChatroomRepositoryHandler> chatroom_repository_handler
) : _mysql_client(mysql_client),
    _redis_client(redis_client),
    _message_delivery_service(message_delivery_service),
    _chatroom_repository_handler(chatroom_repository_handler)
{
    if (!_mysql_client || !_redis_client || !_message_delivery_service || !_chatroom_repository_handler) {
        throw std::runtime_error("ChatroomManagementService dependencies cannot be null");
    }
    
    LOG_INFO("ChatroomManagementService initialized successfully");
}

ChatroomManagementService::~ChatroomManagementService()
{
    LOG_INFO("ChatroomManagementService shutting down");
}

// ==================== チャットルーム操作 ====================

ChatroomOperationResult ChatroomManagementService::process_chatroom_operation(const json& control_message_json)
{
    ChatroomOperationResult result;
    result.success = false;
    result.affected_members = 0;
    try {
        auto  control_message = MessageBase::deserialize_message<ChatroomControlMessage>(control_message_json);
        // バリデーション
        if (!validate_chatroom_control_message(control_message)) {
            result.error_message = "Invalid chatroom control message";
            return result;
        }
        
        auto operation_type = control_message->get_room_action();
        auto room_id = control_message->get_room_id();
        auto requester_id = control_message->get_sender_cid();
        
        // 権限チェック
        if (!check_room_permission(room_id, requester_id, "manage")) {
            result.error_message = "Insufficient permissions for room operation";
            return result;
        }
        
        switch (operation_type) {
            case ChatroomControlMessage::RoomAction::CREATE_ROOM: {
                auto creation_result = create_chatroom(
                    control_message->get_room_name(),
                    control_message->get_room_image_url(),
                    control_message->get_add_member_list(),
                    requester_id
                );
                result.success = creation_result.success;
                result.error_message = creation_result.error_message;
                result.affected_members = creation_result.initial_members;
                break;
            }

            case ChatroomControlMessage::RoomAction::LEAVE_ROOM:
                // ルームからの退出はメンバー削除と同様の処理で対応
                //ToDo: 退出処理の実装
                break;
            
            case ChatroomControlMessage::RoomAction::ADD_MEMBER: {
                auto member_result = add_members(room_id, control_message->get_add_member_list());
                result.success = member_result.success;
                result.error_message = member_result.error_message;
                result.affected_members = member_result.affected_member_ids.size();
                result.unsent_notifications = member_result.failed_member_ids;
                break;
            }
            
            case ChatroomControlMessage::RoomAction::REMOVE_MEMBER: {
                auto member_result = remove_members(room_id, control_message->get_remove_member_list());
                result.success = member_result.success;
                result.error_message = member_result.error_message;
                result.affected_members = member_result.affected_member_ids.size();
                result.unsent_notifications = member_result.failed_member_ids;
                break;
            }
            
            case ChatroomControlMessage::RoomAction::ROOM_NAME_CHANGE: {
                auto name_change_result = _chatroom_repository_handler->change_room_name(room_id, control_message->get_room_name());
                result.success = name_change_result;
                result.error_message = name_change_result ? "" : "Failed to change room name";
                break;
            }
            
            case ChatroomControlMessage::RoomAction::ROOM_ICON_CHANGE: {
                auto icon_change_result = _chatroom_repository_handler->change_room_icon(room_id, control_message->get_room_image_url());
                result.success = icon_change_result;
                result.error_message = icon_change_result ? "" : "Failed to change room icon";
                break;
            }
            
            case ChatroomControlMessage::RoomAction::DELETE_ROOM: {
                auto deletion_result = delete_chatroom(room_id, requester_id);
                result = deletion_result;
                break;
            }
            
            default:
                result.error_message = "Unsupported chatroom operation";
                return result;
        }
        
        // データベース操作ログを保存
        if (result.success) {
            save_room_operation_to_database(control_message, control_message_json);
            LOG_INFO("Chatroom operation completed successfully: {}, affected members: {}", 
                        static_cast<int>(operation_type), result.affected_members);
        }
        
    } catch (const std::exception& e) {
        handle_operation_error(e, "process_chatroom_operation");
        result.error_message = std::string("Operation failed: ") + e.what();
    }
    
    return result;
}

RoomCreationResult ChatroomManagementService::create_chatroom(
    const std::string& room_name,
    const std::string& icon_url,
    const std::vector<std::string>& initial_members,
    const std::string& creator_id)
{
    RoomCreationResult result;
    result.success = false;
    result.initial_members = 0;
    
    try {
        // バリデーション
        if (!validate_room_name(room_name)) {
            result.error_message = "Invalid room name";
            return result;
        }
        
        if (!validate_member_ids(initial_members)) {
            result.error_message = "Invalid member IDs";
            return result;
        }
        
        // メンバーリストに作成者を追加
        std::vector<std::string> all_members = initial_members;
        if (std::find(all_members.begin(), all_members.end(), creator_id) == all_members.end()) {
            all_members.push_back(creator_id);
        }
        
        // 新しいチャットルームIDを生成
        std::string room_id = ChatroomIDGenerator::issue_chatroom_id_unhex();
        
        // チャットルームオブジェクトを作成
        auto new_room = std::make_shared<Chatroom>(room_id, room_name, icon_url, all_members);
        
        // Repositoryに追加
        bool add_result = _chatroom_repository_handler->add_chatroom(new_room);
        if (!add_result) {
            result.error_message = "Failed to create chatroom in repository";
            return result;
        }
        
        result.room_id = new_room->get_chatroom_id();
        result.initial_members = all_members.size();
        result.success = true;
        
        LOG_INFO("Chatroom '{}' created successfully with {} members", room_name, all_members.size());
        
    } catch (const std::exception& e) {
        handle_operation_error(e, "create_chatroom");
        result.error_message = std::string("Room creation failed: ") + e.what();
    }
    
    return result;
}

MemberManagementResult ChatroomManagementService::add_members(const std::string& room_id, const std::vector<std::string>& member_ids)
{
    MemberManagementResult result;
    result.success = false;
    
    try {
        if (!room_exists(room_id)) {
            result.error_message = "Room does not exist";
            return result;
        }
        
        if (!validate_member_ids(member_ids)) {
            result.error_message = "Invalid member IDs";
            return result;
        }
        
        // メンバーを追加
        for (const auto& member_id : member_ids) {
            try {
                bool added = _mysql_client->insert_chatroom_member(room_id, {member_id}, 
                    std::to_string(std::time(nullptr)));
                
                if (added) {
                    result.affected_member_ids.push_back(member_id);
                } else {
                    result.failed_member_ids.push_back(member_id);
                }
            } catch (const std::exception& e) {
                result.failed_member_ids.push_back(member_id);
                spdlog::warn("Failed to add member {} to room {}: {}", member_id, room_id, e.what());
            }
        }
        
        result.success = !result.affected_member_ids.empty();
        if (result.success) {
            spdlog::info("Added {} members to room {}", result.affected_member_ids.size(), room_id);
        }
        
    } catch (const std::exception& e) {
        handle_operation_error(e, "add_members");
        result.error_message = std::string("Add members failed: ") + e.what();
    }
    
    return result;
}

MemberManagementResult ChatroomManagementService::remove_members(const std::string& room_id, const std::vector<std::string>& member_ids)
{
    MemberManagementResult result;
    result.success = false;
    
    try {
        if (!room_exists(room_id)) {
            result.error_message = "Room does not exist";
            return result;
        }
        
        if (!validate_member_ids(member_ids)) {
            result.error_message = "Invalid member IDs";
            return result;
        }
        
        // メンバーを削除
        for (const auto& member_id : member_ids) {
            try {
                bool removed = _mysql_client->delete_chatroom_member(room_id, {member_id});
                
                if (removed) {
                    result.affected_member_ids.push_back(member_id);
                } else {
                    result.failed_member_ids.push_back(member_id);
                }
            } catch (const std::exception& e) {
                result.failed_member_ids.push_back(member_id);
                spdlog::warn("Failed to remove member {} from room {}: {}", member_id, room_id, e.what());
            }
        }
        
        result.success = !result.affected_member_ids.empty();
        if (result.success) {
            spdlog::info("Removed {} members from room {}", result.affected_member_ids.size(), room_id);
        }
        
    } catch (const std::exception& e) {
        handle_operation_error(e, "remove_members");
        result.error_message = std::string("Remove members failed: ") + e.what();
    }
    
    return result;
}

ChatroomOperationResult ChatroomManagementService::delete_chatroom(const std::string& room_id, const std::string& requester_id)
{
    ChatroomOperationResult result;
    result.success = false;
    result.affected_members = 0;
    
    try {
        if (!room_exists(room_id)) {
            result.error_message = "Room does not exist";
            return result;
        }
        
        // 削除前にメンバー数を取得
        auto members = get_room_members(room_id);
        result.affected_members = members.size();
        
        // チャットルームを削除
        bool deleted = _mysql_client->delete_chatroom_fromDB(room_id);
        if (deleted) {
            result.success = true;
            spdlog::info("Chatroom {} deleted by {}, {} members affected", room_id, requester_id, result.affected_members);
        } else {
            result.error_message = "Failed to delete chatroom from database";
        }
        
    } catch (const std::exception& e) {
        handle_operation_error(e, "delete_chatroom");
        result.error_message = std::string("Delete chatroom failed: ") + e.what();
    }
    
    return result;
}

// ==================== 通知配信 ====================

ChatroomOperationResult ChatroomManagementService::deliver_chatroom_notification(
    const std::unique_ptr<ChatroomControlMessage>& control_message,
    socket_t sender_socket)
{
    ChatroomOperationResult result;
    result.success = false;
    result.affected_members = 0;
    
    try {
        // 通知メッセージを作成
        auto notification_message = create_notification_message(control_message, result);
        if (!notification_message) {
            result.error_message = "Failed to create notification message";
            return result;
        }
        
        // ルームメンバーに配信
        auto room_members = get_room_members(control_message->get_room_id());
        auto sender_cid = control_message->get_sender_cid();
        
        // 送信者以外のメンバーをフィルタリング
        std::vector<std::string> target_members;
        for (const auto& member_id : room_members) {
            if (member_id != sender_cid) {
                target_members.push_back(member_id);
            }
        }
        
        if (!target_members.empty()) {
            json notification_json = MessageBase::serialize_message(notification_message);
            auto delivery_result = _message_delivery_service->deliver_to_room(notification_json, control_message->get_room_id(), sender_socket, control_message->get_sender_cid());
            
            result.affected_members = delivery_result.sent_count;
            result.unsent_notifications = std::move(delivery_result.unsent_client_ids);
            result.success = delivery_result.success;
        } else {
            result.success = true; // 配信対象がいない場合は成功とする
        }
        
        LOG_INFO("Chatroom notification delivered to {} members, {} failed", 
                    result.affected_members, result.unsent_notifications.size());
        
    } catch (const std::exception& e) {
        handle_operation_error(e, "deliver_chatroom_notification");
        result.error_message = std::string("Notification delivery failed: ") + e.what();
    }
    
    return result;
}

bool ChatroomManagementService::send_chatroom_acknowledgment(
    const std::unique_ptr<ChatroomControlMessage>& control_message,
    const ChatroomOperationResult& operation_result)
{
    ChatroomControlAckMessage::MessageSendStatus status = ChatroomControlAckMessage::MessageSendStatus::SEND_FAILED;
    try {
        if (operation_result.success) {
            if (operation_result.unsent_notifications.empty()) {
                status = ChatroomControlAckMessage::MessageSendStatus::SEND_SUCCESS;
            } else {
                status = ChatroomControlAckMessage::MessageSendStatus::SEND_PARTICAL_SUCCESS;
            }
        } else {
            status = ChatroomControlAckMessage::MessageSendStatus::SEND_FAILED;
        }
        
        // ACKメッセージを作成
        auto ack_message = std::make_unique<ChatroomControlAckMessage>(
            control_message->get_common_fields(),
            status,
            control_message->get_room_id(),
            control_message->get_message_id(),
            operation_result.unsent_notifications
        );
        
        // メッセージを送信
        auto ack_json = MessageBase::serialize_message(ack_message);
        auto delivery_result = _message_delivery_service->deliver_p2p(
            ack_json, 
            control_message->get_sender_cid()
        );
        
        if (delivery_result.success && delivery_result.sent_count > 0) {
            LOG_DEBUG("Chatroom operation acknowledgment sent to client: {}", control_message->get_sender_cid());
            return true;
        } else {
            LOG_ERROR("Failed to send chatroom operation acknowledgment to client: {}", control_message->get_sender_cid());
            return false;
        }
        
    } catch (const std::exception& e) {
        handle_operation_error(e, "send_chatroom_acknowledgment");
        return false;
    }
}

// ==================== ユーティリティメソッド ====================

std::vector<ChatroomControlDeliveryMessage::ClientInfo> ChatroomManagementService::generate_client_infos(const std::vector<std::string>& client_ids)
{
    std::vector<ChatroomControlDeliveryMessage::ClientInfo> client_infos;
    
    try {
        for (const auto& cid : client_ids) {
            std::string client_name, client_mail, client_icon;
            std::tie(client_name, client_mail, client_icon) = _mysql_client->get_client_info_fromDB(cid);
            
            ChatroomControlDeliveryMessage::ClientInfo info(cid, client_name, client_mail, client_icon);
            client_infos.push_back(info);
        }
        
    } catch (const std::exception& e) {
        handle_operation_error(e, "generate_client_infos");
    }
    
    return client_infos;
}

bool ChatroomManagementService::check_room_permission(const std::string& room_id, const std::string& client_id, const std::string& operation)
{
    try {
        // 基本的な権限チェック: ルームメンバーかどうか
        auto members = get_room_members(room_id);
        return std::find(members.begin(), members.end(), client_id) != members.end();
        
    } catch (const std::exception& e) {
        handle_operation_error(e, "check_room_permission");
        return false;
    }
}

bool ChatroomManagementService::room_exists(const std::string& room_id)
{
    try {
        return _chatroom_repository_handler->search_chatroom_id_from_list(room_id) >= 0;
        
    } catch (const std::exception& e) {
        handle_operation_error(e, "room_exists");
        return false;
    }
}

std::vector<std::string> ChatroomManagementService::get_room_members(const std::string& room_id)
{
    try {
        return _chatroom_repository_handler->get_client_infos_by_chatroom_id(room_id);
        
    } catch (const std::exception& e) {
        handle_operation_error(e, "get_room_members");
        return {};
    }
}

// ==================== 内部ヘルパーメソッド ====================

bool ChatroomManagementService::validate_chatroom_control_message(const std::unique_ptr<ChatroomControlMessage>& control_message)
{
    if (!control_message) {
        spdlog::error("Control message is null");
        return false;
    }
    
    if (!control_message->validate_value()) {
        spdlog::error("Invalid control message values");
        return false;
    }
    
    return true;
}

bool ChatroomManagementService::validate_room_name(const std::string& room_name)
{
    return !room_name.empty() && room_name.length() <= 100;
}

bool ChatroomManagementService::validate_member_ids(const std::vector<std::string>& member_ids)
{
    if (member_ids.empty() || member_ids.size() > 100) {
        return false;
    }
    
    for (const auto& id : member_ids) {
        if (id.empty()) {
            return false;
        }
    }
    
    return true;
}

bool ChatroomManagementService::save_room_operation_to_database(const std::unique_ptr<ChatroomControlMessage>& control_message, const json& message_json)
{
    try {
        // ISO 8601からMySQL形式に変換
        std::string mysql_timestamp = ServerTime::iso8601_to_mysql_datetime(control_message->get_time_stamp());
        // チャットルーム操作をメッセージテーブルに保存
        return _mysql_client->insert_message(
            control_message->get_message_id(),
            control_message->get_room_id(),
            control_message->get_sender_cid(),
            mysql_timestamp,
            message_json.dump()
        );
        
    } catch (const std::exception& e) {
        handle_operation_error(e, "save_room_operation_to_database");
        return false;
    }
}

void ChatroomManagementService::handle_operation_error(const std::exception& e, const std::string& operation_context)
{
    LOG_ERROR("ChatroomManagementService error in {}: {}", operation_context, e.what());
}

std::unique_ptr<ChatroomControlDeliveryMessage> ChatroomManagementService::create_notification_message(
    const std::unique_ptr<ChatroomControlMessage>& control_message,
    const ChatroomOperationResult& result)
{
    try {
        auto client_infos = generate_client_infos(control_message->get_add_member_list());
        
        return std::make_unique<ChatroomControlDeliveryMessage>(
            control_message->get_common_fields(),
            control_message->get_message_id(),
            control_message->get_sender_cid(),
            control_message->get_room_id(),
            control_message->get_room_name(),
            control_message->get_room_image_url(),
            client_infos,
            control_message->get_room_action()
        );
        
    } catch (const std::exception& e) {
        handle_operation_error(e, "create_notification_message");
        return nullptr;
    }
}

std::string ChatroomManagementService::issue_chatroom_id()
{
    // ランダムバイトジェネレータを作成
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);

    // ミリ秒単位の現在時刻
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    // ランダムバイト生成関数
    auto random_func = [&]() -> uint8_t {
        return static_cast<uint8_t>(dist(gen));
    };

    // ULIDを作成
    ulid::ULID ulid = ulid::Create(now, random_func);

    // 文字列に変換して返す
    return ulid::Marshal(ulid);
}

std::string ChatroomManagementService::issue_chatroom_id_unhex()
{
    // ランダムバイトジェネレータを作成
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);

    // ミリ秒単位の現在時刻
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    // ランダムバイト生成関数
    auto random_func = [&]() -> uint8_t {
        return static_cast<uint8_t>(dist(gen));
    };

    // ULIDを作成
    ulid::ULID ulid = ulid::Create(now, random_func);

    // 文字列に変換して返す
    auto ulid_str = ulid::Marshal(ulid);
    auto decode_ulid = base32::decode(ulid_str);
    std::ostringstream oss;
    for (auto b : decode_ulid) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    LOG_INFO("Create Chatroom ID: {}", oss.str());
    return oss.str();
}
