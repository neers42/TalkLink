#ifndef ICHATROOMREPOSITORY_HPP
#define ICHATROOMREPOSITORY_HPP

#include <string>
#include <vector>

// メンバー追加（上限チェック込み）の結果
struct MemberInsertResult
{
    enum class Status { OK, DB_ERROR, LIMIT_EXCEEDED };
    Status status = Status::DB_ERROR;
    std::vector<std::string> added_uids;   // 実際に新規追加された uid（既存メンバーは含まない）
};

class IChatroomRepository
{
public:
    virtual ~IChatroomRepository() = default;

    virtual bool insert_chatroom_member(
        const std::string& room_id,
        const std::vector<std::string>& uids,
        const std::string& joined_at
    ) = 0;

    // 追加後の総メンバー数が max_members 以下に収まる範囲で、まだメンバーでない
    // uid のみを追加する。カウントと INSERT を単一の処理として行うため、
    // 上限判定は並行呼び出しに対しても一貫する。
    virtual MemberInsertResult insert_chatroom_members_within_limit(
        const std::string& room_id,
        const std::vector<std::string>& uids,
        const std::string& joined_at,
        int max_members
    ) = 0;

    virtual bool delete_chatroom_member(
        const std::string& room_id,
        const std::vector<std::string>& uids
    ) = 0;

    virtual std::vector<std::string> get_member_uids(
        const std::string& room_id
    ) = 0;

    virtual bool chatroom_exists(
        const std::string& room_id
    ) = 0;

    virtual bool chatroom_exists_by_hex(
        const std::string& room_id_hex
    ) = 0;

    // uid_a, uid_b の2人だけで構成される(Direct)ルームを検索する。
    // 見つかった場合はそのroom_idを16進数文字列で返し、無ければ空文字列を返す。
    virtual std::string find_direct_room_by_members(
        const std::string& uid_a,
        const std::string& uid_b
    ) = 0;

    virtual bool insert_chatroom(
        const std::string& room_id,
        const std::string& room_name,
        const std::string& icon_url,
        const std::string& created_at
    ) = 0;

    virtual std::string get_chatroom_name(
        const std::string& room_id
    ) = 0;

    virtual bool delete_chatroom(
        const std::string& room_id
    ) = 0;

    virtual bool update_chatroom_name(
        const std::string& room_id,
        const std::string& room_name
    ) = 0;

    virtual bool update_chatroom_icon(
        const std::string& room_id,
        const std::string& icon_url
    ) = 0;
};

#endif