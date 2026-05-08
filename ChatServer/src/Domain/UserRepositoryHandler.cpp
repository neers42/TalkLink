#include "Domain/UserRepositoryHandler.hpp"

// User entity operations
bool UserRepositoryHandler::add_user(socket_t socket, SSL* ssl, const sockaddr_in& addr,
                                        std::shared_ptr<User> user, const std::string& ip)
{
    LOG_TRACE;
    try {
        if (!validate_user_data(user)) {
            LOG_ERROR("Invalid user data provided");
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(_store->get_mutex());
        
        // 既存ユーザーの確認
        if (_store->get_aggregates().find(socket) != _store->get_aggregates().end()) {
            LOG_ERROR("User with socket {} already exists", socket);
            return false;
        }

        // UserAggregateを作成してストアに追加
        UserAggregateStore::UserAggregate aggregate(user, socket, ssl, addr, ip);
        bool result = _store->insert(socket, std::move(aggregate));
        
        if (result) {
            // IDマッピングも更新（ユーザーIDが設定されている場合）
            std::string uid = user->get_id();
            if (!uid.empty()) {
                _store->get_id_to_socket_map()[uid] = socket;
            }
            
            LOG_INFO("[UserRepositoryHandler] User added successfully: socket={}, ip={}, id={}", 
                        socket, ip, uid);
        } else {
            LOG_ERROR("[UserRepositoryHandler] Failed to insert user into store");
        }

        return result;

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in add_user: {}", e.what());
        return false;
    }
}

bool UserRepositoryHandler::add_user(socket_t socket, SSL *ssl, const sockaddr_in &addr, const std::string &ip)
{
    try{
        // ユーザー情報なしで追加するオーバーロード
        UserAggregateStore::UserAggregate aggregate(socket, ssl, addr, ip);
        bool result = _store->insert(socket, std::move(aggregate));
        
        if (result) {
            LOG_INFO("[UserRepositoryHandler] User added without domain info: socket={}, ip={}", 
                        socket, ip);
        } else {
            LOG_ERROR("[UserRepositoryHandler] Failed to insert user into store (no domain info)");
        }

        return result;

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in add_user (no domain info): {}", e.what());
        return false;
    }
    return false;
}
bool UserRepositoryHandler::remove_user(socket_t socket)
{
    LOG_TRACE;
    try {
        // ユーザー情報取得（ログ用）
        std::string uid;
        std::string user_ip_addr;
        
        // 削除前にログ情報を取得
        {
            std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
            auto it = _store->get_aggregates().find(socket);
            if (it == _store->get_aggregates().end()) {
                LOG_WARNING("[UserRepositoryHandler] User not found for socket {}", socket);
                return false;
            }
            
            user_ip_addr = it->second.ipv4_address;
            if (it->second.user) {
                uid = it->second.user->get_id();
            }
        }

        // ストアから削除（IDマッピングも自動的に削除される）
        bool result = _store->erase(socket);
        
        if (result) {
            LOG_INFO("[UserRepositoryHandler] User removed successfully: socket={}, ip={}, id={}", 
                        socket, user_ip_addr, uid);
        } else {
            LOG_ERROR("[UserRepositoryHandler] Failed to remove user from store");
        }

        return result;

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in remove_user: {}", e.what());
        return false;
    }
}

bool UserRepositoryHandler::update_user_info(socket_t socket, const std::string& id,
                                                const std::string& name, const std::string& email)
{
    LOG_TRACE;
    try {
        std::unique_lock<std::shared_mutex> lock(_store->get_mutex());
        
        auto aggregate = get_aggregate(socket);
        if (!aggregate || !aggregate->user) {
            LOG_ERROR("[UserRepositoryHandler] User not found for socket {}", socket);
            return false;
        }

        // クライアント情報更新
        std::string old_id = aggregate->user->get_id();
        
        if (!id.empty()) {
            aggregate->user->set_id(id);
        }
        if (!name.empty()) {
            aggregate->user->set_name(name);
        }
        if (!email.empty()) {
            aggregate->user->set_mail_address(email);
        }
        
        // IDが変更された場合、マッピング更新（空IDは変更として扱わない）
        const std::string current_id = aggregate->user->get_id();
        if (!id.empty() && old_id != current_id) {
            if (!old_id.empty()) {
                _store->get_id_to_socket_map().erase(old_id);
            }
            _store->get_id_to_socket_map()[current_id] = socket;
        }

        LOG_INFO("[UserRepositoryHandler] User info updated: socket={}, id={}, name={}, email={} (empty values are ignored)", 
                socket, aggregate->user->get_id(), aggregate->user->get_name(), aggregate->user->get_mail_address());
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in update_user_info: {}", e.what());
        return false;
    }
}

// 部分更新メソッド（効率的な更新）
bool UserRepositoryHandler::update_user_activity(socket_t socket, time_t last_activity)
{
    LOG_TRACE;
    try {
        std::unique_lock<std::shared_mutex> lock(_store->get_mutex());
        
        auto aggregate = get_aggregate(socket);
        if (!aggregate || !aggregate->user) {
            LOG_ERROR("[UserRepositoryHandler] User not found for socket {}", socket);
            return false;
        }

        // 最終活動時間のみを更新
        aggregate->user->update_last_activity(last_activity);
        
        LOG_DEBUG("[UserRepositoryHandler] User activity updated: socket={}, last_activity={}", 
                    socket, last_activity);
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in update_user_activity: {}", e.what());
        return false;
    }
}

bool UserRepositoryHandler::update_user_heartbeat(socket_t socket)
{
    LOG_TRACE;
    try {
        std::unique_lock<std::shared_mutex> lock(_store->get_mutex());
        
        auto aggregate = get_aggregate(socket);
        if (!aggregate || !aggregate->user) {
            LOG_ERROR("[UserRepositoryHandler] User not found for socket {}", socket);
            return false;
        }

        // ハートビートカウントをリセットし、最終活動時間を現在時刻に更新
        aggregate->user->reset_heart_beat_count();
        aggregate->user->update_last_activity(std::time(nullptr));
        
        LOG_DEBUG("[UserRepositoryHandler] User heartbeat updated: socket={}", socket);
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in update_user_heartbeat: {}", e.what());
        return false;
    }
}

bool UserRepositoryHandler::update_user_auth_status(socket_t socket, bool auth_status)
{
    LOG_TRACE;
    try {
        std::unique_lock<std::shared_mutex> lock(_store->get_mutex());
        
        auto aggregate = get_aggregate(socket);
        if (!aggregate || !aggregate->user) {
            LOG_ERROR("[UserRepositoryHandler] User not found for socket {}", socket);
            return false;
        }

        // 認証状態のみを更新
        aggregate->user->set_auth_status(auth_status);
        
        LOG_DEBUG("[UserRepositoryHandler] User auth status updated: socket={}, auth_status={}", 
                    socket, auth_status);
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in update_user_auth_status: {}", e.what());
        return false;
    }
}

// Query operations
std::shared_ptr<User> UserRepositoryHandler::get_user(socket_t socket)
{
    LOG_TRACE;
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        auto aggregate = get_aggregate(socket);
        if (!aggregate) {
            return nullptr;
        }

        return aggregate->user;

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in get_user: {}", e.what());
        return nullptr;
    }
}

std::shared_ptr<User> UserRepositoryHandler::get_user_by_id(const std::string& uid)
{
    LOG_TRACE;
    try {
        if (uid.empty()) {
            LOG_ERROR("[UserRepositoryHandler] User ID is empty");
            return nullptr;
        }

        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        // IDマッピングから検索
        auto id_it = _store->get_id_to_socket_map().find(uid);
        if (id_it != _store->get_id_to_socket_map().end()) {
            auto aggregate = get_aggregate(id_it->second);
            if (aggregate) {
                return aggregate->user;
            }
        }

        LOG_WARNING("[UserRepositoryHandler] User not found for id: {}", uid);
        return nullptr;

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in get_user_by_id: {}", e.what());
        return nullptr;
    }
}

socket_t UserRepositoryHandler::get_socket_by_uid(const std::string& uid)
{
    LOG_TRACE;
    try {
        if (uid.empty()) {
            LOG_ERROR("[UserRepositoryHandler] User ID is empty");
            return INVALID_SOCKET_FD;
        }

        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        auto it = _store->get_id_to_socket_map().find(uid);
        if (it != _store->get_id_to_socket_map().end()) {
            return it->second;
        }

        LOG_WARNING("[UserRepositoryHandler] Socket not found for user id: {}", uid);
        return INVALID_SOCKET_FD;

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in get_socket_by_uid: {}", e.what());
        return INVALID_SOCKET_FD;
    }
}

std::shared_ptr<User> UserRepositoryHandler::get_user_by_socket(socket_t socket)
{
    LOG_TRACE;
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        auto aggregate = get_aggregate(socket);
        if (!aggregate) {
            return nullptr;
        }

        return aggregate->user;

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in get_user_by_socket: {}", e.what());
        return nullptr;
    }
}
std::string UserRepositoryHandler::get_uid_by_socket(socket_t socket)
{
    LOG_TRACE;
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        auto aggregate = get_aggregate(socket);
        if (!aggregate || !aggregate->user) {
            LOG_WARNING("[UserRepositoryHandler] User not found for socket {}", socket);
            return "";
        }

        return aggregate->user->get_id();

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in get_uid_by_socket: {}", e.what());
        return "";
    }
}

std::string UserRepositoryHandler::get_uid_by_socket(const std::shared_ptr<User> &user)
{
    LOG_TRACE;
    try {
        if (!user) {
            LOG_ERROR("[UserRepositoryHandler] User pointer is null");
            return "";
        }

        return user->get_id();

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in get_uid_by_socket (by user): {}", e.what());
        return "";
    }
}

// Collection queries
size_t UserRepositoryHandler::get_connected_user_count() const
{
    LOG_TRACE;
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        return _store->get_aggregates().size();

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in get_connected_user_count: {}", e.what());
        return 0;
    }
}

std::vector<socket_t> UserRepositoryHandler::get_all_sockets()
{
    LOG_TRACE;
    std::vector<socket_t> sockets;
    
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        const auto& aggregates = _store->get_aggregates();
        sockets.reserve(aggregates.size());
        
        for (const auto& [socket, aggregate] : aggregates) {
            sockets.push_back(socket);
        }

        LOG_INFO("[UserRepositoryHandler] Retrieved {} sockets", sockets.size());

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in get_all_sockets: {}", e.what());
    }
    
    return sockets;
}

std::vector<socket_t> UserRepositoryHandler::get_inactive_users(std::chrono::seconds timeout)
{
    LOG_TRACE;
    std::vector<socket_t> inactive_sockets;
    
    try {
        auto cutoff_time = std::chrono::steady_clock::now() - timeout;
        
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        const auto& aggregates = _store->get_aggregates();
        for (const auto& [socket, aggregate] : aggregates) {
            if (aggregate.last_activity < cutoff_time) {
                inactive_sockets.push_back(socket);
            }
        }

        LOG_INFO("[UserRepositoryHandler] Found {} inactive users (timeout: {}s)", 
                 inactive_sockets.size(), timeout.count());

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in get_inactive_users: {}", e.what());
    }
    
    return inactive_sockets;
}

// Bulk operations
void UserRepositoryHandler::update_all_last_activity()
{
    LOG_TRACE;
    try {
        auto now = std::chrono::steady_clock::now();
        
        std::unique_lock<std::shared_mutex> lock(_store->get_mutex());
        
        auto& aggregates = _store->get_aggregates();
        for (auto& [socket, aggregate] : aggregates) {
            aggregate.last_activity = now;
        }

        LOG_INFO("[UserRepositoryHandler] Updated last activity for {} users", aggregates.size());

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in update_all_last_activity: {}", e.what());
    }
}

void UserRepositoryHandler::clear_all_heartbeats()
{
    LOG_TRACE;
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        const auto& aggregates = _store->get_aggregates();
        for (const auto& [socket, aggregate] : aggregates) {
            if (aggregate.user) {
                aggregate.user->clear_heart_beat_count();
            }
        }

        LOG_DEBUG("[UserRepositoryHandler] Cleared heartbeat counts for {} users", aggregates.size());

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in clear_all_heartbeats: {}", e.what());
    }
}

// Private helper methods
UserAggregateStore::UserAggregate* UserRepositoryHandler::get_aggregate(socket_t socket)
{
    auto it = _store->get_aggregates().find(socket);
    if (it != _store->get_aggregates().end()) {
        return &(it->second);
    }
    return nullptr;
}

bool UserRepositoryHandler::validate_user_data(const std::shared_ptr<User>& user)
{
    if (!user) {
        LOG_ERROR("[UserRepositoryHandler] User pointer is null");
        return false;
    }

    // 基本的なバリデーション
    if (user->get_name().empty()) {
        LOG_WARNING("[UserRepositoryHandler] User name is empty");
    }

    return true;
}


std::map<socket_t, std::shared_ptr<User>> UserRepositoryHandler::get_all_users() const
{
    LOG_TRACE;
    std::map<socket_t, std::shared_ptr<User>> result;
    
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        const auto& connections = _store->get_aggregates();
        
        for (const auto& [socket, aggregate] : connections) {
            if (aggregate.user != nullptr) {
                result[socket] = aggregate.user;
            }
        }
        
        LOG_DEBUG("[UserRepositoryHandler] Retrieved {} users", result.size());
        return result;

    } catch (const std::exception& e) {
        LOG_ERROR("[UserRepositoryHandler] Exception in get_all_users: {}", e.what());
        return result; // 空のマップを返す
    }
}
