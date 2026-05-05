#ifndef CHATSERVICE_HPP
#define CHATSERVICE_HPP

#include "Utils/common.hpp"
#include "Domain/Entity/Messages/ChatMessage.hpp"
#include "Domain/Entity/Messages/ChatDeliveryMessage.hpp"
#include "Domain/Entity/Messages/ChatAckMessage.hpp"
#include "Domain/ClientRepositoryHandler.hpp"
#include "Infrastructure/MySQLClient.hpp"
#include "Infrastructure/RedisClient.hpp"
#include "Application/Services/IMessageDeliveryService.hpp"

// チャットメッセージ処理結果
struct ChatMessageResult {
    bool success;
    int sent_count;
    std::vector<std::string> unsent_client_ids;
    std::string error_message;
};

class ChatService {
private:
    // 依存関係 - インターフェースのみに依存
    std::shared_ptr<MySQLClient> _mysql_client;
    std::shared_ptr<RedisClient> _redis_client;
    std::shared_ptr<IMessageDeliveryService> _message_delivery_service;
    std::shared_ptr<ClientRepositoryHandler> _client_repository_handler;
    
    // 設定
    static constexpr int MAX_RESEND_COUNT = 3;
    static constexpr int LOAD_MESSAGE_NUM = 20;
    
public:
    // コンストラクタ・デストラクタ
    explicit ChatService(
        std::shared_ptr<MySQLClient> mysql_client,
        std::shared_ptr<RedisClient> redis_client,
        std::shared_ptr<IMessageDeliveryService> message_delivery_service,
        std::shared_ptr<ClientRepositoryHandler> client_repository_handler
    );
    ~ChatService();
    
    // ==================== チャットメッセージ処理 ====================
    
    // チャットメッセージの処理（データベース保存）
    bool process_save_message(const std::unique_ptr<ChatMessage>& chat_message, const json& original_json);
    
    // チャットメッセージの配信処理
    IMessageDeliveryService::DeliveryResult deliver_chat_message(const std::unique_ptr<ChatMessage>& chat_message, socket_t sender_socket);
    
    // チャットメッセージの確認応答処理
    bool send_chat_acknowledgment(
        const std::unique_ptr<ChatMessage>& chat_message,
        const IMessageDeliveryService::DeliveryResult& delivery_result,
        socket_t sender_socket
    );
    
    // チャットメッセージの完全処理（処理→配信→確認応答）
    bool process_chat_message(
        const json& original_json,
        socket_t sender_socket
    );
    
    // ==================== ユーティリティメソッド ====================
    
    // ユーティリティメソッド
    json get_unreceived_messages(const std::string& client_id, uint64_t last_received_message_id);
    
    
private:
    // ==================== 内部ヘルパーメソッド ====================
    
    // メッセージバリデーション
    bool validate_chat_message(const std::unique_ptr<ChatMessage>& chat_message);
    
    // データベース操作ヘルパー
    bool save_message_to_database(const std::unique_ptr<ChatMessage>& chat_message, const json& original_json);
    
    // 再送処理の詳細実装
    bool execute_message_resend(const json& message_json, const std::vector<std::string>& target_client_ids);
    
    // エラーハンドリング
    void handle_processing_error(const std::exception& e, const std::string& operation_context);
};

#endif // CHATSERVICE_HPP