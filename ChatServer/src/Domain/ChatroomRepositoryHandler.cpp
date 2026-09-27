#include "Domain/ChatroomRepositoryHandler.hpp"

ChatroomRepositoryHandler::ChatroomRepositoryHandler(
    std::shared_ptr<IChatroomRepository> chatroom_repository,
    std::shared_ptr<IUserRepository> user_repository,
    std::shared_ptr<IMessageRepository> message_repository)
    : _chatroom_repository(std::move(chatroom_repository)),
      _user_repository(std::move(user_repository)),
      _message_repository(std::move(message_repository))
{
    if (!_chatroom_repository || !_user_repository || !_message_repository)
    {
        throw std::invalid_argument("ChatroomRepositoryHandler dependencies cannot be null");
    }
    LOG_INFO("[ChatroomRepositoryHandler] ChatroomRepositoryHandler initialized");
}

ChatroomRepositoryHandler::~ChatroomRepositoryHandler() {
    LOG_INFO("[ChatroomRepositoryHandler] ChatroomRepositoryHandler destroyed");
}

// ==================== メンバーシップ操作 ====================

int ChatroomRepositoryHandler::add_members(const std::string& room_id, const std::vector<std::string>& uids) {
    if (!_chatroom_repository) {
        LOG_ERROR("[ChatroomRepositoryHandler] ChatroomRepository is not initialized");
        return -1;
    }

    // 上限チェック込みで追加（現在メンバー取得→判定→INSERT を原子的に実行）
    const std::string now_mysql = ServerTime::iso8601_to_mysql_datetime(ServerTime::get_current_time_to_milliseconds());
    auto db_result = _chatroom_repository->insert_chatroom_members_within_limit(
        room_id, uids, now_mysql, MAX_USER_NUM_IN_CHATROOM);

    switch (db_result.status) {
        case MemberInsertResult::Status::DB_ERROR:
            LOG_ERROR("[ChatroomRepositoryHandler] Failed to persist chatroom members to DB for room {}", room_id);
            return -1;
        case MemberInsertResult::Status::LIMIT_EXCEEDED:
            return -2;
        case MemberInsertResult::Status::OK:
            break;
    }

    LOG_INFO("[ChatroomRepositoryHandler] Added {} new member(s) to chatroom {}", db_result.added_uids.size(), room_id);
    return static_cast<int>(db_result.added_uids.size());
}

bool ChatroomRepositoryHandler::remove_members(const std::string& room_id, const std::vector<std::string>& uids) {
    if (!_chatroom_repository) {
        LOG_ERROR("[ChatroomRepositoryHandler] ChatroomRepository is not initialized");
        return false;
    }

    if (!_chatroom_repository->delete_chatroom_member(room_id, uids)) {
        LOG_ERROR("[ChatroomRepositoryHandler] Failed to remove chatroom members from DB for room {}", room_id);
        return false;
    }

    LOG_INFO("[ChatroomRepositoryHandler] Removed {} member(s) from chatroom {}", uids.size(), room_id);
    return true;
}

// ==================== ルーム属性更新 ====================

bool ChatroomRepositoryHandler::change_room_name(const std::string& room_id, const std::string& room_name) {
    if (room_name.empty()) {
        LOG_ERROR("[ChatroomRepositoryHandler] The room name cannot be empty.");
        return false;
    }
    if (!_chatroom_repository || !_chatroom_repository->update_chatroom_name(room_id, room_name)) {
        LOG_ERROR("[ChatroomRepositoryHandler] Failed to persist room name for chatroom {}", room_id);
        return false;
    }
    LOG_INFO("[ChatroomRepositoryHandler] Room name changed to '{}' for chatroom {}", room_name, room_id);
    return true;
}

bool ChatroomRepositoryHandler::change_room_icon(const std::string& room_id, const std::string& icon_url) {
    if (icon_url.empty()) {
        LOG_ERROR("[ChatroomRepositoryHandler] The room icon URL cannot be empty.");
        return false;
    }
    if (!_chatroom_repository || !_chatroom_repository->update_chatroom_icon(room_id, icon_url)) {
        LOG_ERROR("[ChatroomRepositoryHandler] Failed to persist room icon for chatroom {}", room_id);
        return false;
    }
    LOG_INFO("[ChatroomRepositoryHandler] Room icon changed to '{}' for chatroom {}", icon_url, room_id);
    return true;
}

// ==================== ルームCRUD ====================

bool ChatroomRepositoryHandler::insert_chatroom_with_members(const std::string& room_id,
                                                             const std::string& room_name,
                                                             const std::string& icon_url,
                                                             const std::vector<std::string>& member_uids) {
    try {
        if (!_chatroom_repository) {
            LOG_ERROR("[ChatroomRepositoryHandler] ChatroomRepository is not initialized");
            return false;
        }

        if (member_uids.empty()) {
            LOG_ERROR("[ChatroomRepositoryHandler] member_uids is empty for room {}", room_id);
            return false;
        }

        const std::string now_mysql = ServerTime::iso8601_to_mysql_datetime(ServerTime::get_current_time_to_milliseconds());

        if (!_chatroom_repository->insert_chatroom(room_id, room_name, icon_url, now_mysql)) {
            LOG_ERROR("[ChatroomRepositoryHandler] Failed to insert chatroom {}", room_id);
            return false;
        }

        if (!_chatroom_repository->insert_chatroom_member(room_id, member_uids, now_mysql)) {
            LOG_ERROR("[ChatroomRepositoryHandler] Failed to insert chatroom members for room {}", room_id);
            _chatroom_repository->delete_chatroom(room_id);
            return false;
        }

        LOG_INFO("[ChatroomRepositoryHandler] Chatroom {} persisted with {} members", room_id, member_uids.size());
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("[ChatroomRepositoryHandler] Error persisting chatroom {}: {}", room_id, e.what());
        return false;
    }
}

bool ChatroomRepositoryHandler::delete_chatroom_and_persist(const std::string& room_id) {
    try {
        if (!_chatroom_repository) {
            LOG_ERROR("[ChatroomRepositoryHandler] ChatroomRepository is not initialized");
            return false;
        }

        const bool db_deleted = _chatroom_repository->delete_chatroom(room_id);
        if (!db_deleted) {
            LOG_WARNING("[ChatroomRepositoryHandler] Failed to delete chatroom from database: {}", room_id);
        }
        return db_deleted;
    } catch (const std::exception& e) {
        LOG_ERROR("[ChatroomRepositoryHandler] Error deleting chatroom {}: {}", room_id, e.what());
        return false;
    }
}

bool ChatroomRepositoryHandler::chatroom_exists(const std::string& room_id_hex) {
    try {
        if (!_chatroom_repository) {
            LOG_ERROR("[ChatroomRepositoryHandler] ChatroomRepository is not initialized");
            return false;
        }
        return _chatroom_repository->chatroom_exists_by_hex(room_id_hex);
    } catch (const std::exception& e) {
        LOG_ERROR("[ChatroomRepositoryHandler] Error checking room existence {}: {}", room_id_hex, e.what());
        return false;
    }
}

std::string ChatroomRepositoryHandler::find_direct_room_by_members(const std::string& uid_a, const std::string& uid_b) {
    try {
        if (!_chatroom_repository) {
            LOG_ERROR("[ChatroomRepositoryHandler] ChatroomRepository is not initialized");
            return "";
        }
        if (uid_a.empty() || uid_b.empty() || uid_a == uid_b) {
            return "";
        }
        return _chatroom_repository->find_direct_room_by_members(uid_a, uid_b);
    } catch (const std::exception& e) {
        LOG_ERROR("[ChatroomRepositoryHandler] Error finding direct room between {} and {}: {}", uid_a, uid_b, e.what());
        return "";
    }
}

// ==================== メッセージ ====================

bool ChatroomRepositoryHandler::insert_message(const uint64_t message_id, const std::string& room_id,
                                              const std::string& sender_uid, const std::string& send_time,
                                              const std::string& message_content, std::uint64_t room_sequence,
                                              const std::string& target_uid) {
    try {
        if (!_message_repository) {
            LOG_ERROR("[ChatroomRepositoryHandler] MessageRepository is not initialized");
            return false;
        }

        bool result = _message_repository->insert_message(message_id, room_id, sender_uid, send_time, message_content, room_sequence, target_uid);
        if (!result) {
            LOG_ERROR("[ChatroomRepositoryHandler] Failed to insert message {} to room {}", message_id, room_id);
        } else {
            LOG_DEBUG("[ChatroomRepositoryHandler] Message {} inserted successfully to room {}", message_id, room_id);
        }
        return result;
    } catch (const std::exception& e) {
        LOG_ERROR("[ChatroomRepositoryHandler] Error inserting message {}: {}", message_id, e.what());
        return false;
    }
}

json ChatroomRepositoryHandler::get_unreceived_messages(const std::string& uid, const uint64_t last_received_message_id) {
    try {
        if (!_message_repository) {
            LOG_ERROR("[ChatroomRepositoryHandler] MessageRepository is not initialized");
            return json::object();
        }
        return _message_repository->get_unreceived_messages(uid, last_received_message_id);
    } catch (const std::exception& e) {
        LOG_ERROR("[ChatroomRepositoryHandler] Error getting unreceived messages for user {}: {}", uid, e.what());
        return json::object();
    }
}

// ==================== 取得 ====================

std::vector<UserInfo> ChatroomRepositoryHandler::get_user_infos(const std::vector<std::string>& uids) {
    std::vector<UserInfo> result;
    try {
        if (!_user_repository) {
            LOG_ERROR("[ChatroomRepositoryHandler] UserRepository is not initialized");
            return result;
        }

        for (const auto& uid : uids) {
            try {
                auto user_info = _user_repository->get_user_info(uid);
                result.emplace_back(UserInfo(uid, user_info.name, user_info.mail_address, user_info.icon_url));
            } catch (const std::exception& e) {
                LOG_WARNING("[ChatroomRepositoryHandler] Failed to get user info for {}: {}", uid, e.what());
            }
        }
        return result;
    } catch (const std::exception& e) {
        LOG_ERROR("[ChatroomRepositoryHandler] Error getting user infos: {}", e.what());
        return result;
    }
}

std::vector<std::string> ChatroomRepositoryHandler::get_member_uids(const std::string& room_id) {
    try {
        if (!_chatroom_repository) {
            LOG_ERROR("[ChatroomRepositoryHandler] ChatroomRepository is not initialized");
            return {};
        }
        return _chatroom_repository->get_member_uids(room_id);
    } catch (const std::exception& e) {
        LOG_ERROR("[ChatroomRepositoryHandler] Error getting member uids for room {}: {}", room_id, e.what());
        return {};
    }
}
