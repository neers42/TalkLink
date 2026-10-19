#include "Application/Services/ChatroomManagementService.hpp"
#include "Domain/Entity/Messages/ChatroomControlDeliveryMessage.hpp"
#include "Domain/Entity/Messages/ChatroomControlAckMessage.hpp"
#include "Application/Interfaces/IMessageDeliveryService.hpp"
#include "Domain/ChatroomRepositoryHandler.hpp"
#include "Domain/ChatroomSequenceHandler.hpp"
#include "Domain/ChatroomIDGenerator.hpp"
#include "Domain/MessageIDGenerator.hpp"
#include "Domain/UserRepositoryHandler.hpp"

ChatroomManagementService::ChatroomManagementService(
    std::shared_ptr<IMessageDeliveryService> message_delivery_service,
    std::shared_ptr<ChatroomRepositoryHandler> chatroom_repository_handler,
    std::shared_ptr<ChatroomSequenceHandler> chatroom_sequence_handler,
    std::shared_ptr<UserRepositoryHandler> user_repository_handler
) : _message_delivery_service(std::move(message_delivery_service)),
    _chatroom_repository_handler(std::move(chatroom_repository_handler)),
    _chatroom_sequence_handler(std::move(chatroom_sequence_handler)),
    _user_repository_handler(std::move(user_repository_handler))
{
    if (!_message_delivery_service || !_chatroom_repository_handler || !_chatroom_sequence_handler || !_user_repository_handler) {
        throw std::runtime_error("ChatroomManagementService dependencies cannot be null");
    }

    LOG_INFO("ChatroomManagementService initialized successfully");
}

ChatroomManagementService::~ChatroomManagementService()
{
    LOG_INFO("ChatroomManagementService shutting down");
}

// ==================== チャットルーム操作 ====================

ChatroomOperationResult ChatroomManagementService::process_chatroom_operation(const json& control_message_json, socket_t sender_socket)
{
    using RoomAction = ChatroomControlMessage::RoomAction;

    ChatroomOperationResult result;
    result.success = false;
    result.affected_members = 0;
    result.room_id = "";
    std::unique_ptr<ChatroomControlMessage> control_message = nullptr;

    try {
        control_message = MessageBase::deserialize_message<ChatroomControlMessage>(control_message_json);

        if (!validate_chatroom_control_message(control_message)) {
            result.error_message = "Invalid chatroom control message";
        } else {
            // 操作本体。ここでの return はラムダを抜けるだけで、必ず末尾の
            // ACK 送信を経由する（ACK 送信の重複を避けるため単一箇所に集約）。
            [&] {
                const RoomAction operation_type = control_message->get_room_action();
                const std::string requester_id = control_message->get_sender_uid();
                result.room_id = control_message->get_room_id();

                // なりすまし対策: payloadのsender_uidが、このソケットで実際に
                // 認証済みのuidと一致することを確認する
                const std::string authenticated_uid = _user_repository_handler->get_uid_by_socket(sender_socket);
                if (authenticated_uid.empty() || authenticated_uid != requester_id) {
                    LOG_WARNING("[ChatroomManagementService] sender_uid mismatch for socket {}: claimed='{}', authenticated='{}'",
                                sender_socket, requester_id, authenticated_uid);
                    result.error_message = "sender_uid does not match the authenticated user";
                    return;
                }

                // 権限チェック（CREATE_ROOM は未作成ルームのため事前チェック対象外）
                if (operation_type != RoomAction::CREATE_ROOM &&
                    !check_room_permission(result.room_id, requester_id, "manage")) {
                    result.error_message = "Insufficient permissions for room operation";
                    return;
                }

                // CREATE_ROOM / DELETE_ROOM 以外は mutation の前に room_sequence を採番する。
                // 採番に失敗したら mutation を実行しない（中途半端な状態を作らない）。
                // 採番済み・mutation 失敗の場合はシーケンスに欠番が出るだけで、
                // 番号の使い回しより安全（クライアントは欠番検知で再同期できる）。
                const bool increment_before_mutation =
                    operation_type != RoomAction::CREATE_ROOM &&
                    operation_type != RoomAction::DELETE_ROOM;
                if (increment_before_mutation &&
                    !_chatroom_sequence_handler->increment_room_sequence(result.room_id, result.room_sequence)) {
                    result.error_message = "Failed to update room sequence";
                    return;
                }

                switch (operation_type) {
                    case RoomAction::CREATE_ROOM: {
                        auto creation_result = create_chatroom(
                            control_message->get_room_name(),
                            control_message->get_icon_url(),
                            control_message->get_member_list(),
                            requester_id
                        );
                        result.success = creation_result.success;
                        result.error_message = creation_result.error_message;
                        result.affected_members = creation_result.initial_members;
                        result.room_reused = creation_result.reused_existing;
                        if (!creation_result.room_id.empty()) {
                            result.room_id = creation_result.room_id;
                        }
                        break;
                    }

                    case RoomAction::LEAVE_ROOM:
                        // ToDo: 退出処理の実装（メンバー削除と同様の処理で対応予定）
                        result.error_message = "LEAVE_ROOM operation is not implemented yet";
                        break;

                    case RoomAction::ADD_MEMBER: {
                        auto member_result = add_members(result.room_id, control_message->get_member_list());
                        result.success = member_result.success;
                        result.error_message = member_result.error_message;
                        result.affected_members = member_result.affected_member_ids.size();
                        result.unsent_notifications = member_result.failed_member_ids;
                        break;
                    }

                    case RoomAction::REMOVE_MEMBER: {
                        auto member_result = remove_members(result.room_id, control_message->get_member_list());
                        result.success = member_result.success;
                        result.error_message = member_result.error_message;
                        result.affected_members = member_result.affected_member_ids.size();
                        result.unsent_notifications = member_result.failed_member_ids;
                        break;
                    }

                    case RoomAction::ROOM_NAME_CHANGE: {
                        result.success = _chatroom_repository_handler->change_room_name(result.room_id, control_message->get_room_name());
                        if (!result.success) {
                            result.error_message = "Failed to change room name";
                        }
                        break;
                    }

                    case RoomAction::ROOM_ICON_CHANGE: {
                        result.success = _chatroom_repository_handler->change_room_icon(result.room_id, control_message->get_icon_url());
                        if (!result.success) {
                            result.error_message = "Failed to change room icon";
                        }
                        break;
                    }

                    case RoomAction::DELETE_ROOM: {
                        const std::string room_id = result.room_id;
                        result = delete_chatroom(room_id, requester_id);
                        result.room_id = room_id;
                        break;
                    }

                    default:
                        result.error_message = "Unsupported chatroom operation";
                        return;
                }

                // mutation が失敗した場合はここで終了（error_message は switch 内で設定済み）。
                if (!result.success) {
                    return;
                }

                // CREATE_ROOMが既存のDirectルームを再利用した場合、そのルームは既に
                // シーケンス初期化・メッセージ永続化・通知配信を済ませているので、
                // ここで二重に行わない（他メンバーへの重複CREATE_ROOM通知も防げる）。
                // ACKには既存のroom_idが載るので、requester側は正しいルームに正規化できる。
                if (result.room_reused) {
                    return;
                }

                // CREATE_ROOM / DELETE_ROOM のシーケンス処理は mutation と不可分なので後段で行う。
                // mutation は成功しているため、失敗しても操作全体は失敗扱いにしない（警告のみ）。
                if (operation_type == RoomAction::DELETE_ROOM) {
                    if (!remove_room_sequence(result.room_id)) {
                        LOG_WARNING("[ChatroomManagementService] Failed to delete room sequence for room {}", result.room_id);
                    }
                } else if (operation_type == RoomAction::CREATE_ROOM) {
                    if (!_chatroom_sequence_handler->increment_room_sequence(result.room_id, result.room_sequence)) {
                        LOG_WARNING("[ChatroomManagementService] Failed to initialize room sequence for room {}", result.room_id);
                    }
                }

                // ルーム制御メッセージをDBに保存（配信より前 = DBを正とする）
                if (!save_room_operation_to_database(control_message, control_message_json, result.room_id, result.room_sequence)) {
                    LOG_WARNING("[ChatroomManagementService] Failed to persist chatroom operation message. room_id={}, message_id={}",
                                result.room_id, control_message->get_message_id());
                }

                // 通知メッセージを送信（制御メッセージ送信元以外に送信）
                const auto delivery_result = deliver_chatroom_notification(control_message, result);
                if (!delivery_result.success) {
                    LOG_WARNING("[ChatroomManagementService] Notification delivery failed: {}", delivery_result.error_message);
                    result.unsent_notifications = delivery_result.unsent_uids;
                }
            }();
        }
    } catch (const std::exception& e) {
        handle_operation_error(e, "process_chatroom_operation");
        result.success = false;
        result.error_message = std::string("Operation failed: ") + e.what();
    }

    // 制御応答メッセージを送信（全経路で必ずここを1回だけ通る）
    if (control_message) {
        if (!send_chatroom_acknowledgment(control_message, result)) {
            LOG_WARNING("[ChatroomManagementService] Failed to send chatroom operation ACK");
        }
    } else {
        LOG_ERROR("[ChatroomManagementService] Cannot send ACK: control message could not be parsed");
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

        // Directルーム(2人)の場合、同じ組み合わせの既存ルームがあれば重複作成せず再利用する
        // (クライアント側のローカル状態が失われた場合などにCREATE_ROOMが重複送信されることへの対策)
        if (all_members.size() == 2) {
            const std::string existing_room_id = _chatroom_repository_handler->find_direct_room_by_members(
                all_members[0], all_members[1]);
            if (!existing_room_id.empty()) {
                result.room_id = existing_room_id;
                result.initial_members = 2;
                result.success = true;
                result.reused_existing = true;
                LOG_INFO("Direct room already exists between '{}' and '{}'. Reusing room_id={}",
                         all_members[0], all_members[1], existing_room_id);
                return result;
            }
        }

        // 新しいチャットルームIDを生成
        const std::string room_id = ChatroomIDGenerator::issue_chatroom_id_unhex();

        // DBへ永続化（chatrooms, chat_members）
        bool persist_result = _chatroom_repository_handler->insert_chatroom_with_members(
            room_id,
            room_name,
            icon_url,
            all_members
        );
        if (!persist_result) {
            result.error_message = "Failed to persist chatroom in database";
            return result;
        }

        result.room_id = room_id;
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

        // 上限チェックを全体で行うため、まとめて 1 回で追加する
        const int added = _chatroom_repository_handler->add_members(room_id, member_ids);
        if (added == -2) {
            result.error_message = "Room member limit exceeded";
            result.failed_member_ids = member_ids;
            return result;
        }
        if (added < 0) {
            result.error_message = "Failed to add members";
            result.failed_member_ids = member_ids;
            return result;
        }

        // 要求されたメンバーは（新規・既存を問わず）ルームに在籍している状態
        result.affected_member_ids = member_ids;
        result.success = true;
        spdlog::info("Added {} new member(s) to room {} (requested {})", added, room_id, member_ids.size());

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

        if (!_chatroom_repository_handler->remove_members(room_id, member_ids)) {
            result.error_message = "Failed to remove members";
            result.failed_member_ids = member_ids;
            return result;
        }

        result.affected_member_ids = member_ids;
        result.success = true;
        spdlog::info("Removed {} member(s) from room {}", member_ids.size(), room_id);

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
        bool deleted = _chatroom_repository_handler->delete_chatroom_and_persist(room_id);
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

 DeliveryResult ChatroomManagementService::deliver_chatroom_notification(
    const std::unique_ptr<ChatroomControlMessage>& control_message,
    const ChatroomOperationResult& operation_result)
{
    DeliveryResult result;
    try {
        const std::string delivery_room_id = operation_result.room_id.empty() ? control_message->get_room_id() : operation_result.room_id;

        // ADD_MEMBER 時は、新規追加メンバーごとに全メンバー情報付きの CREATE_ROOM 通知を配信する。
        // オフライン相手にも届くよう、宛先指定でDBへ永続化してからリアルタイム配信（ベストエフォート）する。
        if(control_message->get_room_action() == ChatroomControlMessage::RoomAction::ADD_MEMBER){
            for (const auto &uid : control_message->get_member_list())
            {
                const uint64_t new_member_mid = MessageIDGenerator::generate_message_id();
                auto new_member_message = create_notification_message(
                    control_message, ChatroomControlMessage::RoomAction::CREATE_ROOM, operation_result, new_member_mid);
                if(!new_member_message){
                    result.success = false;
                    result.error_message = "Failed to create notification message";
                    return result;
                }
                const json &new_member_json = MessageBase::serialize_message(new_member_message);

                if (!save_targeted_notification_to_database(
                        control_message, new_member_json, delivery_room_id, uid, new_member_mid, operation_result.room_sequence))
                {
                    LOG_WARNING("[ChatroomManagementService] Failed to persist CREATE_ROOM notification for new member {}", uid);
                }

                auto p2p_result = _message_delivery_service->deliver_p2p(new_member_json, uid);
                if (p2p_result.success)
                {
                    result.sent_count += p2p_result.sent_count;
                }
                else
                {
                    LOG_DEBUG("[ChatroomManagementService] New member {} offline/unreachable; CREATE_ROOM will be delivered via DB sync", uid);
                }
            }
        }

        // 既存メンバー向けの通知メッセージを作成（ADD_MEMBER の場合は差分のみ）
        auto delivery_message = create_notification_message(
            control_message, control_message->get_room_action(), operation_result, control_message->get_message_id());
        if (!delivery_message) {
            result.success = false;
            result.error_message = "Failed to create notification message";
            return result;
        }
        
        // ルームメンバーに配信
        const json& delivery_json = MessageBase::serialize_message(delivery_message);
        result = _message_delivery_service->deliver_to_room(delivery_json, delivery_room_id, control_message->get_sender_uid());

        if(result.success) {
            LOG_INFO("[ChatroomManagementService] Chatroom notification delivered successfully to room {}, sender: {}, affected members: {}", 
                delivery_room_id, control_message->get_sender_uid(), operation_result.affected_members);
        } else {
            LOG_ERROR("[ChatroomManagementService] Failed to deliver chatroom notification to room {}, sender: {}, error: {}", 
                delivery_room_id, control_message->get_sender_uid(), result.error_message);
            //メッセージの再配信
            // TODO: Implement message retry logic
        }
        LOG_INFO("[ChatroomManagementService] Notification delivery result - success: {}, sent_count: {}, error_message: {}", 
                    result.success, result.sent_count, result.error_message);

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
        const std::string ack_room_id = operation_result.room_id.empty() ? control_message->get_room_id() : operation_result.room_id;
        if (operation_result.success) {
            if (operation_result.unsent_notifications.empty()) {
                status = ChatroomControlAckMessage::MessageSendStatus::SEND_SUCCESS;
            } else {
                status = ChatroomControlAckMessage::MessageSendStatus::SEND_PARTIAL_SUCCESS;
            }
        } else {
            status = ChatroomControlAckMessage::MessageSendStatus::SEND_FAILED;
        }
        
        // ACKメッセージを作成（operation_result から room_sequence を取得）
        auto ack_message = std::make_unique<ChatroomControlAckMessage>(
            control_message->get_common_fields(),
            status,
            ack_room_id,
            control_message->get_message_id(),
            operation_result.unsent_notifications,
            operation_result.room_sequence
        );
        
        // メッセージを送信
        auto ack_json = MessageBase::serialize_message(ack_message);
        auto delivery_result = _message_delivery_service->deliver_p2p(
            ack_json, 
            control_message->get_sender_uid()
        );
        
        if (delivery_result.success && delivery_result.sent_count > 0) {
            LOG_DEBUG("Chatroom operation acknowledgment sent to user: {}", control_message->get_sender_uid());
            return true;
        } else {
            LOG_ERROR("Failed to send chatroom operation acknowledgment to user: {}", control_message->get_sender_uid());
            return false;
        }
        
    } catch (const std::exception& e) {
        handle_operation_error(e, "send_chatroom_acknowledgment");
        return false;
    }
}

// ==================== ユーティリティメソッド ====================

std::vector<UserInfo> ChatroomManagementService::generate_user_infos(const std::vector<std::string>& uids)
{
    std::vector<UserInfo> user_infos;
    
    try {
        user_infos = _chatroom_repository_handler->get_user_infos(uids);

    } catch (const std::exception& e) {
        handle_operation_error(e, "generate_user_infos");
    }
    
    return user_infos;
}

bool ChatroomManagementService::check_room_permission(const std::string& room_id, const std::string& uid, const std::string& operation)
{
    try {
        // 基本的な権限チェック: ルームメンバーかどうか
        auto members = get_room_members(room_id);
        return std::find(members.begin(), members.end(), uid) != members.end();
        
    } catch (const std::exception& e) {
        handle_operation_error(e, "check_room_permission");
        return false;
    }
}

bool ChatroomManagementService::room_exists(const std::string& room_id)
{
    try {
        // インメモリキャッシュではなく DB を参照する。
        // （再起動後はキャッシュが空なので、既存ルームでも miss する）
        return _chatroom_repository_handler->chatroom_exists(room_id);

    } catch (const std::exception& e) {
        handle_operation_error(e, "room_exists");
        return false;
    }
}

std::vector<std::string> ChatroomManagementService::get_room_members(const std::string& room_id)
{
    try {
        return _chatroom_repository_handler->get_member_uids(room_id);
        
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

bool ChatroomManagementService::save_room_operation_to_database(const std::unique_ptr<ChatroomControlMessage>& control_message,
                                                                const json& message_json,
                                                                const std::string& room_id,
                                                                std::uint64_t room_sequence)
{
    try {
        // ISO 8601からMySQL形式に変換
        std::string mysql_timestamp = ServerTime::iso8601_to_mysql_datetime(control_message->get_time_stamp());
        // チャットルーム操作をメッセージテーブルに保存
        return _chatroom_repository_handler->insert_message(
            control_message->get_message_id(),
            room_id,
            control_message->get_sender_uid(),
            mysql_timestamp,
            message_json.dump(),
            room_sequence
        );
        
    } catch (const std::exception& e) {
        handle_operation_error(e, "save_room_operation_to_database");
        return false;
    }
}

bool ChatroomManagementService::save_targeted_notification_to_database(
    const std::unique_ptr<ChatroomControlMessage>& control_message,
    const json& message_json,
    const std::string& room_id,
    const std::string& target_uid,
    uint64_t message_id,
    std::uint64_t room_sequence)
{
    try {
        std::string mysql_timestamp = ServerTime::iso8601_to_mysql_datetime(control_message->get_time_stamp());
        return _chatroom_repository_handler->insert_message(
            message_id,
            room_id,
            control_message->get_sender_uid(),
            mysql_timestamp,
            message_json.dump(),
            room_sequence,
            target_uid
        );

    } catch (const std::exception& e) {
        handle_operation_error(e, "save_targeted_notification_to_database");
        return false;
    }
}


bool ChatroomManagementService::remove_room_sequence(const std::string& room_id)
{
    if (!_chatroom_sequence_handler->delete_room_sequence(room_id)) {
        LOG_ERROR("[ChatroomManagementService] Failed to delete room sequence. Room ID: {}", room_id);
        return false;
    }

    LOG_INFO("[ChatroomManagementService] Room sequence deleted. Room ID: {}", room_id);
    return true;
}

void ChatroomManagementService::handle_operation_error(const std::exception& e, const std::string& operation_context)
{
    LOG_ERROR("ChatroomManagementService error in {}: {}", operation_context, e.what());
}

std::unique_ptr<ChatroomControlDeliveryMessage> ChatroomManagementService::create_notification_message(
    const std::unique_ptr<ChatroomControlMessage>& control_message,
    const ChatroomControlMessage::RoomAction room_action,
    const ChatroomOperationResult& result,
    uint64_t message_id)
{
    try {
        const std::string delivery_room_id = result.room_id.empty() ? control_message->get_room_id() : result.room_id;
        std::vector<UserInfo> user_infos;
        if (room_action == ChatroomControlMessage::RoomAction::CREATE_ROOM)
        {
            // 新規メンバー向け: DBからルーム全員
            auto member_list = _chatroom_repository_handler->get_member_uids(delivery_room_id);
            user_infos = generate_user_infos(member_list);
        }
        else
        {
            // 既存メンバー向け (ADD_MEMBER): 追加分の差分のみ
            user_infos = generate_user_infos(control_message->get_member_list());
        }

        return std::make_unique<ChatroomControlDeliveryMessage>(
            control_message->get_common_fields(),
            message_id,
            control_message->get_sender_uid(),
            delivery_room_id,
            control_message->get_room_name(),
            control_message->get_icon_url(),
            user_infos,
            room_action,
            result.room_sequence
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
