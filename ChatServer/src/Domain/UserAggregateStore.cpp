#include "Domain/UserAggregateStore.hpp"

// 外部公開API
size_t UserAggregateStore::size() const {
    std::shared_lock<std::shared_mutex> lock(_shared_mutex);
    return _aggregates.size();
}

bool UserAggregateStore::empty() const {
    std::shared_lock<std::shared_mutex> lock(_shared_mutex);
    return _aggregates.empty();
}

// フレンドクラス専用の内部操作
bool UserAggregateStore::insert(socket_t socket, UserAggregate&& aggregate) {
    std::unique_lock<std::shared_mutex> lock(_shared_mutex);
    
    // 既存のソケットをチェック
    auto existing_socket_iter = _aggregates.find(socket);
    if (existing_socket_iter != _aggregates.end()) {
        LOG_WARNING("Socket {} already exists in UserAggregateStore, removing old entry before inserting new one", socket);
        
        // 既存のユーザーIDマッピングも削除
        if (existing_socket_iter->second.user) {
            const std::string& old_uid = existing_socket_iter->second.user->get_id();
            if (!old_uid.empty()) {
                _user_id_to_socket.erase(old_uid);
                LOG_DEBUG("Removed old user ID mapping: {} -> {}", old_uid, socket);
            }
        }
        
        // 古いソケットエントリを削除
        _aggregates.erase(existing_socket_iter);
        LOG_DEBUG("Removed old socket entry for socket: {}", socket);
    }
    
    // ユーザーIDが有効かチェック
    if (aggregate.user && aggregate.user->get_id().empty()) {
        LOG_WARNING("User has empty ID in aggregate, allowing insertion anyway");
    }
    
    // ユーザーオブジェクトが存在する場合のみIDチェック
    if (aggregate.user) {
        const std::string& uid = aggregate.user->get_id();
        
        // 空でないIDの場合のみ重複チェック
        if (!uid.empty()) {
            // 既存のユーザーIDをチェック
            auto existing_id_iter = _user_id_to_socket.find(uid);
            if (existing_id_iter != _user_id_to_socket.end()) {
                LOG_WARNING("User ID {} already exists, removing old mapping", uid);
                
                // 古いマッピングを削除
                auto old_socket = existing_id_iter->second;
                _aggregates.erase(old_socket);
                _user_id_to_socket.erase(existing_id_iter);
            }
            
            // 新しいマッピングを追加
            _user_id_to_socket[uid] = socket;
        }
    }
    
    // Aggregateをストアに追加
    _aggregates.emplace(socket, std::move(aggregate));
    
    LOG_DEBUG("UserAggregate inserted: socket={}", socket);
    return true;
}

bool UserAggregateStore::erase(socket_t socket) {
    std::unique_lock<std::shared_mutex> lock(_shared_mutex);
    
    auto iter = _aggregates.find(socket);
    if (iter == _aggregates.end()) {
        LOG_DEBUG("Socket {} not found in UserAggregateStore", socket);
        return false;
    }
    
    // クライアントIDからのマッピングも削除
    if (iter->second.user && !iter->second.user->get_id().empty()) {
        const std::string& uid = iter->second.user->get_id();
        _user_id_to_socket.erase(uid);
        LOG_DEBUG("UserAggregate erased: socket={}, uid={}", socket, uid);
    }
    
    _aggregates.erase(iter);
    return true;
}

UserAggregateStore::UserAggregate* UserAggregateStore::find(socket_t socket) {
    std::shared_lock<std::shared_mutex> lock(_shared_mutex);
    
    auto iter = _aggregates.find(socket);
    return (iter != _aggregates.end()) ? &iter->second : nullptr;
}

const UserAggregateStore::UserAggregate* UserAggregateStore::find(socket_t socket) const {
    std::shared_lock<std::shared_mutex> lock(_shared_mutex);
    
    auto iter = _aggregates.find(socket);
    return (iter != _aggregates.end()) ? &iter->second : nullptr;
}
