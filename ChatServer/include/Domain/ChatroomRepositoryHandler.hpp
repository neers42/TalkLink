#ifndef CHATROOMREPOSITORYHANDLER_HPP
#define CHATROOMREPOSITORYHANDLER_HPP

#include "Utils/common.hpp"
#include "Domain/Entity/Chatroom.hpp"
#include <shared_mutex>
#include <vector>
#include <memory>
#include <string>

#define MAX_ACTIVE_CHATROOM_NUM 1000  // activeなチャットルームの最大数
#define MAX_CLIENT_NUM_IN_CHATROOM 100 // チャットルーム内の最大クライアント数

/**
 * @brief チャットルームのデータアクセス・ストレージ管理を担当するRepository層
 * 
 * 責務:
 * - チャットルームの永続化・取得・検索
 * - スレッドセーフなデータアクセス 
 * - メモリ管理・容量制限
 */
class ChatroomRepositoryHandler {
private:
    // チャットルームのインメモリストレージ
    std::vector<std::shared_ptr<Chatroom>> _chatroom_list;
    
    // 読み書き用の共有ミューテックス
    mutable std::shared_mutex _shared_mtx;
    
    // プライベートヘルパーメソッド
    std::vector<std::shared_ptr<Chatroom>>::iterator find_chatroom_iterator(const std::string& room_id);
    int find_chatroom_index(const std::string& room_id);

public:
    // コンストラクタ・デストラクタ
    ChatroomRepositoryHandler();
    ~ChatroomRepositoryHandler();
    
    // コピー・移動を禁止
    ChatroomRepositoryHandler(const ChatroomRepositoryHandler&) = delete;
    ChatroomRepositoryHandler& operator=(const ChatroomRepositoryHandler&) = delete;
    ChatroomRepositoryHandler(ChatroomRepositoryHandler&&) = delete;
    ChatroomRepositoryHandler& operator=(ChatroomRepositoryHandler&&) = delete;
    
    // ==================== 基本CRUD操作 ====================
    
    /**
     * @brief チャットルームを保存
     * @param chatroom 保存するチャットルーム
     * @return 保存成功時true
     */
    bool add_chatroom(std::shared_ptr<Chatroom> chatroom);
    
    /**
     * @brief チャットルームを削除
     * @param room_id 削除するルームID
     * @return 削除成功時true
     */
    bool delete_chatroom(const std::string& room_id);
    
    /**
     * @brief チャットルームをIDで取得
     * @param room_id 取得するルームID
     * @return 見つかった場合チャットルーム、見つからない場合nullptr
     */
    std::shared_ptr<Chatroom> find_chatroom_by_id(const std::string& room_id);
    
    /**
     * @brief すべてのチャットルームを取得（スナップショット）
     * @return チャットルームのコピーリスト
     */
    std::vector<std::shared_ptr<Chatroom>> get_chatrooms_snapshot();
    
    // ==================== 検索・取得操作（既存API互換） ====================
    
    /**
     * @brief インデックスでクライアント情報を取得
     * @param list_num リストインデックス
     * @return クライアントIDリスト
     */
    std::vector<std::string> get_client_infos_by_chatroom_id(int list_num);
    
    /**
     * @brief ルームIDでクライアント情報を取得
     * @param room_id ルームID
     * @return クライアントIDリスト
     */
    std::vector<std::string> get_client_infos_by_chatroom_id(const std::string& room_id);
    
    /**
     * @brief チャットルームIDを検索
     * @param chatroom_id 検索するルームID
     * @return 見つかったインデックス、見つからない場合-1
     */
    int search_chatroom_id_from_list(const std::string& chatroom_id);
    
    /**
     * @brief チャットルームのイテレータを検索
     * @param room_id 検索するルームID
     * @return イテレータ
     */
    std::vector<std::shared_ptr<Chatroom>>::iterator find_chatroom_id_from_list(const std::string& room_id);
    
    /**
     * @brief チャットルームのクライアントリストを取得
     * @param room_id ルームID
     * @return クライアントIDリスト
     */
    std::vector<std::string> get_client_list_from_chatroom(const std::string& room_id);
    
    // ==================== メンバーシップ操作 ====================
    
    /**
     * @brief ルームメンバーを追加
     * @param room_id 対象ルームID
     * @param cids 追加するクライアントIDリスト
     * @return 追加成功時true
     */
    bool add_member_to_chatroom(const std::string& room_id, const std::vector<std::string>& cids);
    
    /**
     * @brief ルームメンバーを削除
     * @param room_id 対象ルームID
     * @param cids 削除するクライアントIDリスト
     * @return 削除成功時true
     */
    bool delete_member_from_chatroom(const std::string& room_id, const std::vector<std::string>& cids);
    
    // ==================== ルーム属性更新 ====================
    
    /**
     * @brief ルーム名を更新
     * @param room_id 対象ルームID
     * @param room_name 新しいルーム名
     * @return 更新成功時true
     */
    bool change_room_name(const std::string& room_id, const std::string& room_name);
    
    /**
     * @brief ルームアイコンを更新
     * @param room_id 対象ルームID
     * @param icon_url 新しいアイコンURL
     * @return 更新成功時true
     */
    bool change_room_icon(const std::string& room_id, const std::string& icon_url);
    
    // ==================== friend クラス ====================
    // ChatroomManagementHandlerに特権アクセスを許可
    friend class ChatroomHandler;
    
public:
    // 互換性のため、既存のpublic変数を維持
    std::shared_mutex& get_shared_mutex() { return _shared_mtx; }
};

#endif // CHATROOMREPOSITORYHANDLER_HPP