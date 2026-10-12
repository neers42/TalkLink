#ifndef SYNCSERVICE_HPP
#define SYNCSERVICE_HPP

#include "Utils/common.hpp"

class SyncRequestMessage;
class ChatroomRepositoryHandler;
class IMessageDeliveryService;

class SyncService {
private:
    // 依存関係 - インターフェースのみに依存
    std::shared_ptr<IMessageDeliveryService> _message_delivery_service;
    std::shared_ptr<ChatroomRepositoryHandler> _chatroom_repository_handler;

public:
    // コンストラクタ・デストラクタ
    explicit SyncService(
        std::shared_ptr<IMessageDeliveryService> message_delivery_service,
        std::shared_ptr<ChatroomRepositoryHandler> chatroom_repository_handler
    );
    ~SyncService();

    // ==================== 同期処理 ====================

    // SyncRequestの完全処理（JSON受信→欠番検索→SyncResponse送信）
    bool process_sync_request(
        const json& original_json,
        socket_t sender_socket
    );

private:
    // ==================== 内部ヘルパーメソッド ====================

    // リクエスト元がそのルームのメンバーかどうかを確認する
    bool is_room_member(const std::string& room_id, const std::string& uid);

    // エラーハンドリング
    void handle_processing_error(const std::exception& e, const std::string& operation_context);
};

#endif // SYNCSERVICE_HPP
