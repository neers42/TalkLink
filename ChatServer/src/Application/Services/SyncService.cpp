#include "Application/Services/SyncService.hpp"
#include "Domain/Entity/Messages/SyncRequestMessage.hpp"
#include "Domain/Entity/Messages/SyncResponseMessage.hpp"
#include "Domain/ChatroomRepositoryHandler.hpp"
#include "Domain/UserRepositoryHandler.hpp"
#include "Application/Interfaces/IMessageDeliveryService.hpp"

SyncService::SyncService(
    std::shared_ptr<IMessageDeliveryService> message_delivery_service,
    std::shared_ptr<ChatroomRepositoryHandler> chatroom_repository_handler,
    std::shared_ptr<UserRepositoryHandler> user_repository_handler
) : _message_delivery_service(std::move(message_delivery_service)),
    _chatroom_repository_handler(std::move(chatroom_repository_handler)),
    _user_repository_handler(std::move(user_repository_handler))
{
    if (!_message_delivery_service || !_chatroom_repository_handler || !_user_repository_handler) {
        throw std::runtime_error("SyncService dependencies cannot be null");
    }

    LOG_INFO("[SyncService] SyncService initialized successfully");
}

SyncService::~SyncService() {
    LOG_INFO("[SyncService] Service shutting down!!");
}

bool SyncService::process_sync_request(
    const json& original_json,
    socket_t sender_socket
) {
    try {
        LOG_INFO("[SyncService] Processing sync request from socket: {}", sender_socket);

        // 1. JSONからSyncRequestMessageを生成
        auto sync_request = MessageBase::deserialize_message<SyncRequestMessage>(original_json);
        if (!sync_request) {
            LOG_ERROR("[SyncService] Failed to create SyncRequestMessage instance from JSON");
            return false;
        }

        const std::string room_id = sync_request->get_room_id();
        const std::string target_uid = sync_request->get_target_uid();
        const std::uint64_t from_sequence = sync_request->get_from_sequence();

        // なりすまし対策: payloadのtarget_uidが、このソケットで実際に
        // 認証済みのuidと一致することを確認する
        const std::string authenticated_uid = _user_repository_handler->get_uid_by_socket(sender_socket);
        if (authenticated_uid.empty() || authenticated_uid != target_uid) {
            LOG_WARNING("[SyncService] target_uid mismatch for socket {}: claimed='{}', authenticated='{}'",
                        sender_socket, target_uid, authenticated_uid);
            return false;
        }

        // 2. リクエスト元がそのルームのメンバーであることを確認
        //    (他人のルームの内容を覗けないようにするための権限チェック)
        if (!is_room_member(room_id, target_uid)) {
            LOG_WARNING("[SyncService] uid '{}' is not a member of room '{}'. Rejecting sync request.",
                        target_uid, room_id);
            return false;
        }

        // 3. 欠番分のメッセージを検索
        //    (送信者自身のメッセージも除外しない。room_sequenceは送受信者問わず
        //     連番としてクライアント側で追跡されるため)
        json gap_result = _chatroom_repository_handler->get_room_messages_after_sequence(
            room_id, from_sequence, target_uid);
        json gap_messages = gap_result.contains("gap_messages") ? gap_result["gap_messages"] : json::array();

        // 4. SyncResponseを生成して送信元に返す
        auto sync_response = std::make_unique<SyncResponseMessage>(
            sync_request->get_common_fields(),
            room_id,
            target_uid,
            gap_messages
        );

        const json response_json = MessageBase::serialize_message<SyncResponseMessage>(sync_response);
        auto send_result = _message_delivery_service->deliver_p2p(response_json, target_uid);

        if (!send_result.success) {
            LOG_ERROR("[SyncService] Failed to send sync response to uid: {}", target_uid);
            return false;
        }

        LOG_INFO("[SyncService] Sync response sent to uid: {} ({} messages, room: {})",
                 target_uid, gap_messages.size(), room_id);
        return true;

    } catch (const std::exception& e) {
        handle_processing_error(e, "process_sync_request");
        return false;
    }
}

bool SyncService::is_room_member(const std::string& room_id, const std::string& uid) {
    if (room_id.empty() || uid.empty()) {
        return false;
    }
    auto members = _chatroom_repository_handler->get_member_uids(room_id);
    return std::find(members.begin(), members.end(), uid) != members.end();
}

void SyncService::handle_processing_error(const std::exception& e, const std::string& operation_context) {
    LOG_ERROR("[SyncService] Error in {}: {}", operation_context, e.what());
}
