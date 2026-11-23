#ifndef CHATROOMUSECASE_HPP
#define CHATROOMUSECASE_HPP

#include "Utils/common.hpp"
#include "Application/DTO/DeliveryResult.hpp"
#include "Domain/Messages/ChatroomControlMessage.hpp"
#include "Domain/DTO/UserInfo.hpp"

class ChatroomControlMessage;
class ChatroomControlDeliveryMessage;
class ChatroomControlAckMessage;
class IMessageSender;
class ChatroomRepositoryHandler;
class ChatroomSequenceHandler;
class ChatroomIDGenerator;
class UserRepositoryHandler;

// チャットルーム操作結果
struct ChatroomOperationResult {
    bool success;
    std::string error_message;
    int affected_members;
    std::vector<std::string> unsent_notifications;
    std::uint64_t room_sequence = 0;
    std::string room_id;
    bool room_reused = false; // CREATE_ROOMで既存のDirectルームを再利用した場合true
};

// ルーム作成結果
struct RoomCreationResult {
    bool success;
    std::string room_id;
    std::string error_message;
    int initial_members;
    bool reused_existing = false; // 既存のDirectルームを再利用した場合true(新規作成していない)
};

// メンバー管理結果
struct MemberManagementResult {
    bool success;
    std::string error_message;
    std::vector<std::string> affected_member_ids;
    std::vector<std::string> failed_member_ids;
};

class ChatroomUseCase {
private:
    // 依存関係
    std::shared_ptr<IMessageSender> _message_sender;
    std::shared_ptr<ChatroomRepositoryHandler> _chatroom_repository_handler;
    std::shared_ptr<ChatroomSequenceHandler> _chatroom_sequence_handler;
    std::shared_ptr<UserRepositoryHandler> _user_repository_handler;

public:
    // コンストラクタ・デストラクタ
    explicit ChatroomUseCase(
        std::shared_ptr<IMessageSender> message_sender,
        std::shared_ptr<ChatroomRepositoryHandler> chatroom_repository_handler,
        std::shared_ptr<ChatroomSequenceHandler> chatroom_sequence_handler,
        std::shared_ptr<UserRepositoryHandler> user_repository_handler
    );
    ~ChatroomUseCase();

    // ==================== チャットルーム操作 ====================

    // チャットルーム操作処理（統合）
    // sender_socket: payload内のsender_uidが、実際にこのソケットで認証済みのuidと
    // 一致するかを検証するために使用する(なりすまし対策)
    ChatroomOperationResult process_chatroom_operation(const json& control_message_json, socket_t sender_socket);
    
    // チャットルーム作成
    RoomCreationResult create_chatroom(
        const std::string& room_name,
        const std::string& icon_url,
        const std::vector<std::string>& initial_members,
        const std::string& creator_id
    );
    
    // チャットルームメンバー管理
    MemberManagementResult add_members(const std::string& room_id, const std::vector<std::string>& member_ids);
    MemberManagementResult remove_members(const std::string& room_id, const std::vector<std::string>& member_ids);
    
    // チャットルーム削除
    ChatroomOperationResult delete_chatroom(const std::string& room_id, const std::string& requester_id);
    
    // ==================== 通知配信 ====================
    
    // チャットルーム操作通知の配信
    DeliveryResult deliver_chatroom_notification(
        const std::unique_ptr<ChatroomControlMessage>& control_message,
        const ChatroomOperationResult& operation_result
    );
    
    // チャットルーム操作の確認応答
    bool send_chatroom_acknowledgment(
        const std::unique_ptr<ChatroomControlMessage>& control_message,
        const ChatroomOperationResult& operation_result
    );
    
    // ==================== ユーティリティメソッド ====================
    
    // クライアント情報生成
    std::vector<UserInfo> generate_user_infos(const std::vector<std::string>& uids);
    
    // ルーム権限チェック
    bool check_room_permission(const std::string& room_id, const std::string& uid, const std::string& operation);
    
    // ルーム存在チェック
    bool room_exists(const std::string& room_id);
    
    // ルームメンバー取得
    std::vector<std::string> get_room_members(const std::string& room_id);
    
private:
    // ==================== 内部ヘルパーメソッド ====================
    
    // バリデーション
    bool validate_chatroom_control_message(const std::unique_ptr<ChatroomControlMessage>& control_message);
    bool validate_room_name(const std::string& room_name);
    bool validate_member_ids(const std::vector<std::string>& member_ids);
    
    // データベース操作
    bool save_room_operation_to_database(const std::unique_ptr<ChatroomControlMessage> &control_message,
                                         const json &message_json,
                                         const std::string &room_id,
                                         std::uint64_t room_sequence);
    // 特定ユーザー宛の通知（ADD_MEMBER 時の新規メンバーへの CREATE_ROOM 等）をDBへ永続化する
    bool save_targeted_notification_to_database(const std::unique_ptr<ChatroomControlMessage> &control_message,
                                                const json &message_json,
                                                const std::string &room_id,
                                                const std::string &target_uid,
                                                uint64_t message_id,
                                                std::uint64_t room_sequence);
    bool remove_room_sequence(const std::string& room_id);
    
    // エラーハンドリング
    void handle_operation_error(const std::exception& e, const std::string& operation_context);
    
    // 通知メッセージ生成
    std::unique_ptr<ChatroomControlDeliveryMessage> create_notification_message(
        const std::unique_ptr<ChatroomControlMessage>& control_message,
        const ChatroomControlMessage::RoomAction room_action,
        const ChatroomOperationResult& result,
        uint64_t message_id
    );

    // プライベートヘルパーメソッド
    static std::string issue_chatroom_id();
    static std::string issue_chatroom_id_unhex();
};

#endif // CHATROOMUSECASE_HPP