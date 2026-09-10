#ifndef CHATSERVICE_HPP
#define CHATSERVICE_HPP

#include "Utils/common.hpp"
#include "Application/Services/DTO/DeliveryResult.hpp"

class ChatMessage;
class ChatDeliveryMessage;
class ChatAckMessage;
class UserRepositoryHandler;
class ChatroomRepositoryHandler;
class IMessageDeliveryService;
class ChatroomSequenceHandler;

// チャットメッセージ処理結果
struct ChatMessageResult {
    bool success;
    int sent_count;
    std::vector<std::string> unsent_uids;
    std::string error_message;
};

class ChatService {
private:
    // 依存関係 - インターフェースのみに依存
    std::shared_ptr<IMessageDeliveryService> _message_delivery_service;
    std::shared_ptr<UserRepositoryHandler> _user_repository_handler;
    std::shared_ptr<ChatroomRepositoryHandler> _chatroom_repository_handler;
    std::shared_ptr<ChatroomSequenceHandler> _chatroom_sequence_handler;
    
    // 設定
    static constexpr int MAX_RESEND_COUNT = 3;
    static constexpr int LOAD_MESSAGE_NUM = 20;
    
public:
    // コンストラクタ・デストラクタ
    explicit ChatService(
        std::shared_ptr<IMessageDeliveryService> message_delivery_service,
        std::shared_ptr<UserRepositoryHandler> user_repository_handler,
        std::shared_ptr<ChatroomRepositoryHandler> chatroom_repository_handler,
        std::shared_ptr<ChatroomSequenceHandler> chatroom_sequence_handler
    );
    ~ChatService();
    
    // ==================== チャットメッセージ処理 ====================
    
    // チャットメッセージの配信処理
    DeliveryResult deliver_chat_message(const std::unique_ptr<ChatMessage>& chat_message);
    
    // チャットメッセージの確認応答処理
    bool send_chat_acknowledgment(
        const std::unique_ptr<ChatMessage>& chat_message,
        const DeliveryResult& delivery_result,
        socket_t sender_socket
    );
    
    // チャットメッセージの完全処理（処理→配信→確認応答）
    bool process_chat_message(
        const json& original_json,
        socket_t sender_socket
    );
    
    // ==================== ユーティリティメソッド ====================
    
    // ユーティリティメソッド
    json get_unreceived_messages(const std::string& uid, uint64_t last_received_message_id);
    
    
private:
    // ==================== 内部ヘルパーメソッド ====================
    
    // データベース操作ヘルパー
    bool save_message_to_database(const std::unique_ptr<ChatMessage>& chat_message, const json& original_json, std::uint64_t room_sequence = 0);
    
    // 再送処理
    bool execute_message_resend(const json& message_json, const std::vector<std::string>& target_uids);

    bool advance_room_sequence(const std::string& room_id);
    
    // エラーハンドリング
    void handle_processing_error(const std::exception& e, const std::string& operation_context);
};

#endif // CHATSERVICE_HPP