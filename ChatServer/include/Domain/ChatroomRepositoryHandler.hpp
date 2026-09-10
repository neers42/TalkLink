#ifndef CHATROOMREPOSITORYHANDLER_HPP
#define CHATROOMREPOSITORYHANDLER_HPP

#include "Utils/common.hpp"
#include "Infrastructure/MySQLAdapter.hpp"
#include "Domain/DTO/UserInfo.hpp"
#include <vector>
#include <memory>
#include <string>

#define MAX_USER_NUM_IN_CHATROOM 100 // チャットルーム内の最大ユーザー数

/**
 * @brief チャットルームのデータアクセスを担当する Repository 層
 *
 * DB(chat_members / chatrooms / messages)を唯一の正とする。
 * インメモリキャッシュは持たない。
 */
class ChatroomRepositoryHandler {
private:
    std::shared_ptr<IChatroomRepository> _chatroom_repository;
    std::shared_ptr<IUserRepository> _user_repository;
    std::shared_ptr<IMessageRepository> _message_repository;

public:
    explicit ChatroomRepositoryHandler(
        std::shared_ptr<IChatroomRepository> chatroom_repository,
        std::shared_ptr<IUserRepository> user_repository,
        std::shared_ptr<IMessageRepository> message_repository);
    ~ChatroomRepositoryHandler();

    // コピー・移動を禁止
    ChatroomRepositoryHandler(const ChatroomRepositoryHandler&) = delete;
    ChatroomRepositoryHandler& operator=(const ChatroomRepositoryHandler&) = delete;
    ChatroomRepositoryHandler(ChatroomRepositoryHandler&&) = delete;
    ChatroomRepositoryHandler& operator=(ChatroomRepositoryHandler&&) = delete;

    // ==================== メンバーシップ操作 ====================

    /**
     * @brief ルームメンバーを追加（上限チェック込み）
     * @return 新規に追加したメンバー数（0以上）。-1=DBエラー、-2=メンバー数上限超過（未変更）
     */
    int add_members(const std::string& room_id, const std::vector<std::string>& uids);

    /**
     * @brief ルームメンバーを削除
     * @return 削除成功時true
     */
    bool remove_members(const std::string& room_id, const std::vector<std::string>& uids);

    // ==================== ルーム属性更新 ====================

    bool change_room_name(const std::string& room_id, const std::string& room_name);
    bool change_room_icon(const std::string& room_id, const std::string& icon_url);

    // ==================== ルームCRUD ====================

    /**
     * @brief チャットルームとメンバーをDBに保存
     */
    bool insert_chatroom_with_members(const std::string& room_id,
                                      const std::string& room_name,
                                      const std::string& icon_url,
                                      const std::vector<std::string>& member_uids);

    /**
     * @brief チャットルームを削除（chat_members / messages は FK CASCADE で連動）
     */
    bool delete_chatroom_and_persist(const std::string& room_id);

    /**
     * @brief ルームIDが存在するか（16進数形式）
     */
    bool chatroom_exists(const std::string& room_id_hex);

    // ==================== メッセージ ====================

    bool insert_message(const uint64_t message_id, const std::string& room_id,
                        const std::string& sender_uid, const std::string& send_time,
                        const std::string& message_content, std::uint64_t room_sequence = 0,
                        const std::string& target_uid = "");

    json get_unreceived_messages(const std::string& uid, const uint64_t last_received_message_id);

    // ==================== 取得 ====================

    /**
     * @brief 複数ユーザーの情報を取得
     */
    std::vector<UserInfo> get_user_infos(const std::vector<std::string>& uids);

    /**
     * @brief ルームのメンバー UID 一覧を取得
     */
    std::vector<std::string> get_member_uids(const std::string& room_id);
};

#endif // CHATROOMREPOSITORYHANDLER_HPP
