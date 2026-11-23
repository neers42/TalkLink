#ifndef CHATSERVICE_APPLICATION_TEST_FAKES_HPP
#define CHATSERVICE_APPLICATION_TEST_FAKES_HPP

#include "Domain/Interfaces/IChatroomRepository.hpp"
#include "Domain/Interfaces/IUserRepository.hpp"
#include "Domain/Interfaces/IMessageRepository.hpp"
#include "Domain/Interfaces/ISessionRepository.hpp"
#include "Domain/Interfaces/IRoomSequenceRepository.hpp"
#include "Application/Interfaces/IMessageSender.hpp"
#include "MessageBase.hpp"
#include "ServerTime.hpp"
#include <map>
#include <set>
#include <vector>
#include <utility>

// MessageBase::serialize_common_message_fieldsはprivateなので、テストでは
// 自前でcommon fields(message_type/correlation_id/timestamp)を組み立てる。
inline json build_common_fields_json(const std::string& message_type, const std::string& correlation_id) {
    json j;
    j["message_type"] = message_type;
    j["correlation_id"] = correlation_id;
    j["timestamp"] = ServerTime::get_current_time_to_milliseconds();
    return j;
}

// ChatService Application層テスト用の共通フェイク実装。
// 実際のMySQL/Redisに接続せず、ChatroomRepositoryHandler/UserRepositoryHandler/
// ChatroomSequenceHandlerを本物のまま、下層のインターフェースだけ差し替える。

// IChatroomRepositoryのテスト用フェイク実装(メモリ上でルーム/メンバーを管理する)
class FakeChatroomRepository : public IChatroomRepository {
public:
    std::map<std::string, std::set<std::string>> room_members; // room_id -> uids
    std::map<std::string, std::string> room_names;
    std::map<std::string, std::string> room_icons;
    std::set<std::string> deleted_rooms;

    bool insert_chatroom_member(const std::string& room_id, const std::vector<std::string>& uids, const std::string&) override {
        for (const auto& uid : uids) room_members[room_id].insert(uid);
        return true;
    }

    MemberInsertResult insert_chatroom_members_within_limit(const std::string& room_id, const std::vector<std::string>& uids, const std::string&, int max_members) override {
        MemberInsertResult result;
        auto& members = room_members[room_id];
        for (const auto& uid : uids) {
            if (members.find(uid) == members.end()) {
                if (static_cast<int>(members.size()) >= max_members) {
                    result.status = MemberInsertResult::Status::LIMIT_EXCEEDED;
                    return result;
                }
                members.insert(uid);
                result.added_uids.push_back(uid);
            }
        }
        result.status = MemberInsertResult::Status::OK;
        return result;
    }

    bool delete_chatroom_member(const std::string& room_id, const std::vector<std::string>& uids) override {
        for (const auto& uid : uids) room_members[room_id].erase(uid);
        return true;
    }

    std::vector<std::string> get_member_uids(const std::string& room_id) override {
        std::vector<std::string> result;
        auto it = room_members.find(room_id);
        if (it != room_members.end()) {
            result.assign(it->second.begin(), it->second.end());
        }
        return result;
    }

    bool chatroom_exists(const std::string& room_id) override {
        return room_members.find(room_id) != room_members.end() && deleted_rooms.find(room_id) == deleted_rooms.end();
    }

    bool chatroom_exists_by_hex(const std::string& room_id_hex) override {
        return chatroom_exists(room_id_hex);
    }

    std::string find_direct_room_by_members(const std::string& uid_a, const std::string& uid_b) override {
        for (const auto& room_entry : room_members) {
            const auto& members = room_entry.second;
            if (members.size() == 2 && members.count(uid_a) && members.count(uid_b)) {
                return room_entry.first;
            }
        }
        return "";
    }

    bool insert_chatroom(const std::string& room_id, const std::string& room_name, const std::string& icon_url, const std::string&) override {
        room_names[room_id] = room_name;
        room_icons[room_id] = icon_url;
        if (room_members.find(room_id) == room_members.end()) {
            room_members[room_id] = {};
        }
        return true;
    }

    std::string get_chatroom_name(const std::string& room_id) override {
        auto it = room_names.find(room_id);
        return it != room_names.end() ? it->second : "";
    }

    bool delete_chatroom(const std::string& room_id) override {
        deleted_rooms.insert(room_id);
        return true;
    }

    bool update_chatroom_name(const std::string& room_id, const std::string& room_name) override {
        room_names[room_id] = room_name;
        return true;
    }

    bool update_chatroom_icon(const std::string& room_id, const std::string& icon_url) override {
        room_icons[room_id] = icon_url;
        return true;
    }
};

// IUserRepositoryのテスト用フェイク実装
class FakeUserRepository : public IUserRepository {
public:
    bool insert_user_info_data_called = false;
    bool insert_user_info_data_result = true;
    std::string last_inserted_uid;
    // DBに登録済みとして扱うuid(user_existsの判定に使う)
    std::set<std::string> registered_uids;
    // trueの場合、user_existsはDB障害(DB_ERROR)を返す
    bool user_exists_db_error = false;

    bool insert_user_info_data(const std::string& uid, const std::string&, const std::string&,
                                const std::string&, const std::string&, const std::string&) override {
        insert_user_info_data_called = true;
        last_inserted_uid = uid;
        if (insert_user_info_data_result) {
            registered_uids.insert(uid);
        }
        return insert_user_info_data_result;
    }
    UserInfo get_user_info(const std::string& uid) override {
        return UserInfo(uid, "", "", "");
    }
    bool update_last_login(const std::string&, const std::string&) override { return true; }
    UserExistence user_exists(const std::string& uid) override {
        if (user_exists_db_error) {
            return UserExistence::DB_ERROR;
        }
        return registered_uids.count(uid) > 0 ? UserExistence::EXISTS : UserExistence::NOT_FOUND;
    }
    std::string get_user_last_login(const std::string&) override { return ""; }
    std::vector<UserInfo> search_user_infos(const std::string&) override { return {}; }
};

// IMessageRepositoryのテスト用フェイク実装
class FakeMessageRepository : public IMessageRepository {
public:
    bool last_insert_called = false;
    json gap_messages_to_return = json::array();

    bool insert_message(uint64_t, const std::string&, const std::string&, const std::string&,
                         const std::string&, uint64_t, const std::string&) override {
        last_insert_called = true;
        return true;
    }
    json get_unreceived_messages(const std::string&, uint64_t) override { return json::array(); }
    json get_room_messages_after_sequence(const std::string&, uint64_t, const std::string&) override {
        json result;
        result["gap_messages"] = gap_messages_to_return;
        return result;
    }
};

// IMessageSenderのテスト用フェイク実装
class FakeMessageSender : public IMessageSender {
public:
    bool should_succeed = true;
    int sent_count_to_return = 1;
    // should_succeed=false の場合にDeliveryResult::unsent_uidsへ入れる値。
    // 実際のMessageSenderは配信失敗した宛先をunsent_uidsに積むため、
    // それを模倣しないと呼び出し元の再送ロジックが「対象0件で成功扱い」に
    // なってしまい、失敗シナリオを正しく再現できない。
    std::vector<std::string> unsent_uids_to_return;
    std::vector<json> delivered_to_room;
    std::vector<std::pair<json, std::string>> delivered_p2p; // {message, target_uid}
    std::vector<json> broadcasted;

    DeliveryResult deliver_to_room(const json& message, const std::string& /*room_id*/, const std::string& /*uid*/) override {
        delivered_to_room.push_back(message);
        return make_result();
    }

    DeliveryResult deliver_p2p(const json& message, const std::string& target_uid) override {
        delivered_p2p.emplace_back(message, target_uid);
        return make_result();
    }

    DeliveryResult broadcast(const json& message, const std::string& /*uid*/, bool /*exclude_sender*/ = true) override {
        broadcasted.push_back(message);
        return make_result();
    }

private:
    DeliveryResult make_result() const {
        DeliveryResult result;
        result.success = should_succeed;
        result.sent_count = sent_count_to_return;
        if (!should_succeed) {
            result.unsent_uids = unsent_uids_to_return;
        }
        return result;
    }
};

// IRedisAdapterのテスト用フェイク実装(ChatroomSequenceHandler/SessionHandler用)
class FakeRedisAdapter : public ISessionRepository, public IRoomSequenceRepository {
public:
    std::map<std::string, std::uint64_t> counters;
    std::map<std::string, std::string> sessions; // token_hash -> uid相当のセッション情報
    bool user_session_exists_result = false;

    // 注: どちらのインターフェースにも属さない(RedisAdapter側もインターフェース外)
    bool user_session_exists(const std::string& session_key) {
        return sessions.find(session_key) != sessions.end() || user_session_exists_result;
    }

    bool initialize_counter_key(const std::string& key, std::uint64_t initial_value = 0) override {
        // 実装のensure_key_initialized相当: 既に存在するキーは上書きしない
        counters.emplace(key, initial_value);
        return true;
    }

    bool increment_counter_key(const std::string& key, std::uint64_t& next_value) override {
        // 実際のRedisのINCRと同様、存在しないキーは0から開始する
        next_value = ++counters[key];
        return true;
    }

    std::uint64_t get_counter_value(const std::string& key) override {
        auto it = counters.find(key);
        return it != counters.end() ? it->second : 0;
    }

    bool delete_key(const std::string& key) override {
        counters.erase(key);
        sessions.erase(key);
        return true;
    }

    std::optional<std::string> get_session(const std::string& session_token_hash) override {
        // 実際のRedisAdapter::get_sessionは"session:"+ハッシュのキーで保存しているため、
        // フェイクも同じキー形式(sessionsマップには"session:"プレフィックス付きで格納)を再現する。
        auto it = sessions.find("session:" + session_token_hash);
        if (it == sessions.end()) {
            return std::nullopt;
        }
        return it->second;
    }
};

#endif // CHATSERVICE_APPLICATION_TEST_FAKES_HPP
