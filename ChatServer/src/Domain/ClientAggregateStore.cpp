#include "Domain/ClientAggregateStore.hpp"

// 外部公開API
size_t ClientAggregateStore::size() const {
    std::shared_lock<std::shared_mutex> lock(_shared_mutex);
    return _aggregates.size();
}

bool ClientAggregateStore::empty() const {
    std::shared_lock<std::shared_mutex> lock(_shared_mutex);
    return _aggregates.empty();
}

// フレンドクラス専用の内部操作
bool ClientAggregateStore::insert(socket_t socket, ClientAggregate&& aggregate) {
    std::unique_lock<std::shared_mutex> lock(_shared_mutex);
    
    // 既存のソケットをチェック
    auto existing_socket_iter = _aggregates.find(socket);
    if (existing_socket_iter != _aggregates.end()) {
        LOG_WARNING("Socket {} already exists in ClientAggregateStore, removing old entry before inserting new one", socket);
        
        // 既存のクライアントIDマッピングも削除
        if (existing_socket_iter->second.client) {
            const std::string& old_client_id = existing_socket_iter->second.client->get_id();
            if (!old_client_id.empty()) {
                _client_id_to_socket.erase(old_client_id);
                LOG_DEBUG("Removed old client ID mapping: {} -> {}", old_client_id, socket);
            }
        }
        
        // 古いソケットエントリを削除
        _aggregates.erase(existing_socket_iter);
        LOG_DEBUG("Removed old socket entry for socket: {}", socket);
    }
    
    // クライアントIDが有効かチェック
    if (aggregate.client && aggregate.client->get_id().empty()) {
        LOG_WARNING("Client has empty ID in aggregate, allowing insertion anyway");
    }
    
    // クライアントオブジェクトが存在する場合のみIDチェック
    if (aggregate.client) {
        const std::string& client_id = aggregate.client->get_id();
        
        // 空でないIDの場合のみ重複チェック
        if (!client_id.empty()) {
            // 既存のクライアントIDをチェック
            auto existing_id_iter = _client_id_to_socket.find(client_id);
            if (existing_id_iter != _client_id_to_socket.end()) {
                LOG_WARNING("Client ID {} already exists, removing old mapping", client_id);
                
                // 古いマッピングを削除
                auto old_socket = existing_id_iter->second;
                _aggregates.erase(old_socket);
                _client_id_to_socket.erase(existing_id_iter);
            }
            
            // 新しいマッピングを追加
            _client_id_to_socket[client_id] = socket;
        }
    }
    
    // Aggregateをストアに追加
    _aggregates.emplace(socket, std::move(aggregate));
    
    LOG_DEBUG("ClientAggregate inserted: socket={}", socket);
    return true;
}

bool ClientAggregateStore::erase(socket_t socket) {
    std::unique_lock<std::shared_mutex> lock(_shared_mutex);
    
    auto iter = _aggregates.find(socket);
    if (iter == _aggregates.end()) {
        LOG_DEBUG("Socket {} not found in ClientAggregateStore", socket);
        return false;
    }
    
    // クライアントIDからのマッピングも削除
    if (iter->second.client && !iter->second.client->get_id().empty()) {
        const std::string& client_id = iter->second.client->get_id();
        _client_id_to_socket.erase(client_id);
        LOG_DEBUG("ClientAggregate erased: socket={}, client_id={}", socket, client_id);
    }
    
    _aggregates.erase(iter);
    return true;
}

ClientAggregateStore::ClientAggregate* ClientAggregateStore::find(socket_t socket) {
    std::shared_lock<std::shared_mutex> lock(_shared_mutex);
    
    auto iter = _aggregates.find(socket);
    return (iter != _aggregates.end()) ? &iter->second : nullptr;
}

const ClientAggregateStore::ClientAggregate* ClientAggregateStore::find(socket_t socket) const {
    std::shared_lock<std::shared_mutex> lock(_shared_mutex);
    
    auto iter = _aggregates.find(socket);
    return (iter != _aggregates.end()) ? &iter->second : nullptr;
}
