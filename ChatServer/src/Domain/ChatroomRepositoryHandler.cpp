#include "Domain/ChatroomRepositoryHandler.hpp"

ChatroomRepositoryHandler::ChatroomRepositoryHandler() {
    LOG_INFO("ChatroomRepositoryHandler initialized");
}

ChatroomRepositoryHandler::~ChatroomRepositoryHandler() {
    std::unique_lock<std::shared_mutex> lock(_shared_mtx);
    _chatroom_list.clear();
    LOG_INFO("ChatroomRepositoryHandler destroyed");
}

// ==================== プライベートヘルパーメソッド ====================

std::vector<std::shared_ptr<Chatroom>>::iterator 
ChatroomRepositoryHandler::find_chatroom_iterator(const std::string& room_id) {
    return std::find_if(_chatroom_list.begin(), _chatroom_list.end(),
        [&room_id](const std::shared_ptr<Chatroom>& room) {
            return room && room->get_chatroom_id() == room_id;
        });
}

int ChatroomRepositoryHandler::find_chatroom_index(const std::string& room_id) {
    for (size_t i = 0; i < _chatroom_list.size(); ++i) {
        if (_chatroom_list[i] && _chatroom_list[i]->get_chatroom_id() == room_id) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// ==================== 基本CRUD操作 ====================

bool ChatroomRepositoryHandler::add_chatroom(std::shared_ptr<Chatroom> chatroom) {
    try {
        std::unique_lock<std::shared_mutex> lock(_shared_mtx);
        if (!chatroom || chatroom->get_chatroom_name().empty() || chatroom->get_users().empty()) {
            LOG_ERROR("Chatroom name or user list is empty.");
            return false;
        }
        if (_chatroom_list.size() == MAX_ACTIVE_CHATROOM_NUM) {
            _chatroom_list.erase(_chatroom_list.begin());
        }

        _chatroom_list.push_back(std::move(chatroom));
        LOG_INFO("Chatroom added successfully");
        return true;
    }
    catch (const std::bad_alloc& e) {
        LOG_ERROR("Memory allocation failed: {}", e.what());
        return false;
    }
    catch (const std::system_error& e) {
        LOG_ERROR("System error: {}", e.what());
        return false;
    }
    catch (const std::runtime_error& e) {
        LOG_ERROR("Runtime error: {}", e.what());
        return false;
    }
    catch (const std::logic_error& e) {
        LOG_ERROR("Logic error: {}", e.what());
        return false;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception: {}", e.what());
        return false;
    }
}

bool ChatroomRepositoryHandler::delete_chatroom(const std::string& room_id) {
    std::unique_lock<std::shared_mutex> lock(_shared_mtx);
    auto it = find_chatroom_iterator(room_id);
    if (it == _chatroom_list.end()) {
        LOG_ERROR("The chatroom ID({}) you tried to delete could not be found.", room_id);
        return false;
    }
    _chatroom_list.erase(it);
    LOG_INFO("Chatroom ID:{} deleted from active room list successfully.", room_id);
    return true;
}

std::shared_ptr<Chatroom> ChatroomRepositoryHandler::find_chatroom_by_id(const std::string& room_id) {
    std::shared_lock<std::shared_mutex> lock(_shared_mtx);
    auto it = find_chatroom_iterator(room_id);
    return (it != _chatroom_list.end()) ? *it : nullptr;
}

std::vector<std::shared_ptr<Chatroom>> ChatroomRepositoryHandler::get_chatrooms_snapshot() {
    std::shared_lock<std::shared_mutex> lock(_shared_mtx);
    return _chatroom_list; // コピーを返す
}

// ==================== 検索・取得操作（既存API互換） ====================

std::vector<std::string> ChatroomRepositoryHandler::get_user_infos_by_chatroom_id(int list_num) {
    std::shared_lock<std::shared_mutex> lock(_shared_mtx);
    if (list_num >= 0 && static_cast<size_t>(list_num) < _chatroom_list.size()) {
        return _chatroom_list[list_num]->get_users();
    }
    return std::vector<std::string>();
}

std::vector<std::string> ChatroomRepositoryHandler::get_user_infos_by_chatroom_id(const std::string& room_id) {
    std::shared_lock<std::shared_mutex> lock(_shared_mtx);
    int chatroom_list_num = find_chatroom_index(room_id);
    if (chatroom_list_num != -1) {
        return _chatroom_list[chatroom_list_num]->get_users();
    }
    else {
        return std::vector<std::string>();
    }
}

int ChatroomRepositoryHandler::search_chatroom_id_from_list(const std::string& room_id) {
    std::shared_lock<std::shared_mutex> lock(_shared_mtx);
    return find_chatroom_index(room_id);
}

std::vector<std::shared_ptr<Chatroom>>::iterator ChatroomRepositoryHandler::find_chatroom_id_from_list(const std::string& room_id) {
    std::shared_lock<std::shared_mutex> lock(_shared_mtx);
    return find_chatroom_iterator(room_id);
}

std::vector<std::string> ChatroomRepositoryHandler::get_users_from_chatroom(const std::string& chatroom_id) {
    std::shared_lock<std::shared_mutex> lock(_shared_mtx);
    int chatroom_list_num = find_chatroom_index(chatroom_id);
    if (chatroom_list_num != -1) {
        return _chatroom_list[chatroom_list_num]->get_users();
    }
    else {
        LOG_ERROR("Chatroom id is not found.");
        return std::vector<std::string>();
    }
}

// ==================== メンバーシップ操作 ====================

bool ChatroomRepositoryHandler::add_member_to_chatroom(const std::string& room_id, const std::vector<std::string>& uids) {
    std::shared_lock<std::shared_mutex> lock(_shared_mtx);
    auto it = find_chatroom_iterator(room_id);
    if (it == _chatroom_list.end()) {
        LOG_WARNING("The chatroom ID({}) you tried to add a member to could not be found.", room_id);
        return false;
    }
    for (const auto& uid : uids) {
        if ((*it)->get_users().size() == MAX_USER_NUM_IN_CHATROOM) {
            LOG_ERROR("The chatroom ID({}) has reached the maximum number of users.", room_id);
            return false;
        }
        if (!(*it)->add_user_chatroom(uid)) {
            LOG_ERROR("Failed to add user ID({}) to chatroom ID({}).", uid, room_id);
            return false;
        }
    }
    LOG_INFO("{}Users added to chatroom ID({}) successfully.", uids.size(), room_id);
    return true;
}

bool ChatroomRepositoryHandler::delete_member_from_chatroom(const std::string& room_id, const std::vector<std::string>& uids) {
    std::shared_lock<std::shared_mutex> lock(_shared_mtx);
    auto it = find_chatroom_iterator(room_id);
    if (it == _chatroom_list.end()) {
        LOG_WARNING("The chatroom ID({}) you tried to remove a member from could not be found.", room_id);
        return false;
    }
    for (const auto& uid : uids) {
        if ((*it)->get_users().size() == 0) {
            LOG_WARNING("The chatroom ID({}) has no users.", room_id);
            return false;
        }
        if (!(*it)->delete_user_chatroom(uid)) {
            LOG_ERROR("Failed to delete user ID({}) from chatroom ID({}).", uid, room_id);
            return false;
        }
    }
    LOG_INFO("{}Users removed from chatroom ID({}) successfully.", uids.size(), room_id);
    return true;
}

// ==================== ルーム属性更新 ====================

bool ChatroomRepositoryHandler::change_room_name(const std::string& room_id, const std::string& room_name) {
    std::shared_lock<std::shared_mutex> lock(_shared_mtx);
    auto it = find_chatroom_iterator(room_id);
    if (it == _chatroom_list.end()) {
        LOG_ERROR("The chatroom ID({}) you tried to change name could not be found.", room_id);
        return false;
    }
    if (room_name.empty()) {
        LOG_ERROR("The room name cannot be empty.");
        return false;
    }
    (*it)->set_chatroom_name(room_name);
    LOG_INFO("Room name changed to '{}' for chatroom ID: {}", room_name, room_id);
    return true;
}

bool ChatroomRepositoryHandler::change_room_icon(const std::string& room_id, const std::string& icon_url) {
    std::shared_lock<std::shared_mutex> lock(_shared_mtx);
    auto it = find_chatroom_iterator(room_id);
    if (it == _chatroom_list.end()) {
        LOG_ERROR("The chatroom ID({}) you tried to change icon could not be found.", room_id);
        return false;
    }
    if (icon_url.empty()) {
        LOG_ERROR("The room icon URL cannot be empty.");
        return false;
    }
    (*it)->set_chatroom_icon(icon_url);
    LOG_INFO("Room icon changed to '{}' for chatroom ID: {}", icon_url, room_id);
    return true;
}