#include "Domain/ClientRepositoryHandler.hpp"

// Client entity operations
bool ClientRepositoryHandler::add_client(socket_t socket, SSL* ssl, const sockaddr_in& addr,
                                        std::shared_ptr<Client> client, const std::string& ip)
{
    LOG_TRACE;
    try {
        if (!validate_client_data(client)) {
            LOG_ERROR("Invalid client data provided");
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(_store->get_mutex());
        
        // 既存クライアントの確認
        if (_store->get_aggregates().find(socket) != _store->get_aggregates().end()) {
            LOG_ERROR("Client with socket {} already exists", socket);
            return false;
        }

        // ClientAggregateを作成してストアに追加
        ClientAggregateStore::ClientAggregate aggregate(client, socket, ssl, addr, ip);
        bool result = _store->insert(socket, std::move(aggregate));
        
        if (result) {
            // IDマッピングも更新（クライアントIDが設定されている場合）
            std::string client_id = client->get_id();
            if (!client_id.empty()) {
                _store->get_id_to_socket_map()[client_id] = socket;
            }
            
            LOG_INFO("[ClientRepositoryHandler] Client added successfully: socket={}, ip={}, id={}", 
                        socket, ip, client_id);
        } else {
            LOG_ERROR("Failed to insert client into store");
        }

        return result;

    } catch (const std::exception& e) {
        LOG_ERROR("Exception in add_client: {}", e.what());
        return false;
    }
}

bool ClientRepositoryHandler::add_client(socket_t socket, SSL *ssl, const sockaddr_in &addr, const std::string &ip)
{
    try{
        // クライアント情報なしで追加するオーバーロード
        ClientAggregateStore::ClientAggregate aggregate(socket, ssl, addr, ip);
        bool result = _store->insert(socket, std::move(aggregate));
        
        if (result) {
            LOG_INFO("[ClientRepositoryHandler] Client added without domain info: socket={}, ip={}", 
                        socket, ip);
        } else {
            LOG_ERROR("Failed to insert client into store (no domain info)");
        }

        return result;

    } catch (const std::exception& e) {
        LOG_ERROR("Exception in add_client (no domain info): {}", e.what());
        return false;
    }
    return false;
}
bool ClientRepositoryHandler::remove_client(socket_t socket)
{
    LOG_TRACE;
    try {
        // クライアント情報取得（ログ用）
        std::string client_id;
        std::string client_ip;
        
        // 削除前にログ情報を取得
        {
            std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
            auto it = _store->get_aggregates().find(socket);
            if (it == _store->get_aggregates().end()) {
                LOG_WARNING("[ClientRepositoryHandler] Client not found for socket {}", socket);
                return false;
            }
            
            client_ip = it->second.ipv4_address;
            if (it->second.client) {
                client_id = it->second.client->get_id();
            }
        }

        // ストアから削除（IDマッピングも自動的に削除される）
        bool result = _store->erase(socket);
        
        if (result) {
            LOG_INFO("[ClientRepositoryHandler] Client removed successfully: socket={}, ip={}, id={}", 
                        socket, client_ip, client_id);
        } else {
            LOG_ERROR("[ClientRepositoryHandler] Failed to remove client from store");
        }

        return result;

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientRepositoryHandler] Exception in remove_client: {}", e.what());
        return false;
    }
}

bool ClientRepositoryHandler::update_client_info(socket_t socket, const std::string& id,
                                                const std::string& name, const std::string& email)
{
    LOG_TRACE;
    try {
        std::unique_lock<std::shared_mutex> lock(_store->get_mutex());
        
        auto aggregate = get_aggregate(socket);
        if (!aggregate || !aggregate->client) {
            LOG_ERROR("[ClientRepositoryHandler] Client not found for socket {}", socket);
            return false;
        }

        // クライアント情報更新
        std::string old_id = aggregate->client->get_id();
        
        aggregate->client->set_id(id);
        aggregate->client->set_name(name);
        aggregate->client->set_mail_address(email);
        
        // IDが変更された場合、マッピング更新
        if (old_id != id) {
            if (!old_id.empty()) {
                _store->get_id_to_socket_map().erase(old_id);
            }
            if (!id.empty()) {
                _store->get_id_to_socket_map()[id] = socket;
            }
        }

        LOG_INFO("[ClientRepositoryHandler] Client info updated: socket={}, id={}, name={}, email={}", 
                    socket, id, name, email);
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientRepositoryHandler] Exception in update_client_info: {}", e.what());
        return false;
    }
}

// 部分更新メソッド（効率的な更新）
bool ClientRepositoryHandler::update_client_activity(socket_t socket, time_t last_activity)
{
    LOG_TRACE;
    try {
        std::unique_lock<std::shared_mutex> lock(_store->get_mutex());
        
        auto aggregate = get_aggregate(socket);
        if (!aggregate || !aggregate->client) {
            LOG_ERROR("[ClientRepositoryHandler] Client not found for socket {}", socket);
            return false;
        }

        // 最終活動時間のみを更新
        aggregate->client->update_last_activity(last_activity);
        
        LOG_DEBUG("[ClientRepositoryHandler] Client activity updated: socket={}, last_activity={}", 
                    socket, last_activity);
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientRepositoryHandler] Exception in update_client_activity: {}", e.what());
        return false;
    }
}

bool ClientRepositoryHandler::update_client_heartbeat(socket_t socket)
{
    LOG_TRACE;
    try {
        std::unique_lock<std::shared_mutex> lock(_store->get_mutex());
        
        auto aggregate = get_aggregate(socket);
        if (!aggregate || !aggregate->client) {
            LOG_ERROR("[ClientRepositoryHandler] Client not found for socket {}", socket);
            return false;
        }

        // ハートビートカウントをリセットし、最終活動時間を現在時刻に更新
        aggregate->client->reset_heart_beat_count();
        aggregate->client->update_last_activity(std::time(nullptr));
        
        LOG_DEBUG("[ClientRepositoryHandler] Client heartbeat updated: socket={}", socket);
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientRepositoryHandler] Exception in update_client_heartbeat: {}", e.what());
        return false;
    }
}

bool ClientRepositoryHandler::update_client_auth_status(socket_t socket, bool auth_status)
{
    LOG_TRACE;
    try {
        std::unique_lock<std::shared_mutex> lock(_store->get_mutex());
        
        auto aggregate = get_aggregate(socket);
        if (!aggregate || !aggregate->client) {
            LOG_ERROR("[ClientRepositoryHandler] Client not found for socket {}", socket);
            return false;
        }

        // 認証状態のみを更新
        aggregate->client->set_auth_status(auth_status);
        
        LOG_DEBUG("[ClientRepositoryHandler] Client auth status updated: socket={}, auth_status={}", 
                    socket, auth_status);
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientRepositoryHandler] Exception in update_client_auth_status: {}", e.what());
        return false;
    }
}

// Query operations
std::shared_ptr<Client> ClientRepositoryHandler::get_client(socket_t socket)
{
    LOG_TRACE;
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        auto aggregate = get_aggregate(socket);
        if (!aggregate) {
            return nullptr;
        }

        return aggregate->client;

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientRepositoryHandler] Exception in get_client: {}", e.what());
        return nullptr;
    }
}

std::shared_ptr<Client> ClientRepositoryHandler::get_client_by_id(const std::string& client_id)
{
    LOG_TRACE;
    try {
        if (client_id.empty()) {
            LOG_ERROR("[ClientRepositoryHandler] Client ID is empty");
            return nullptr;
        }

        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        // IDマッピングから検索
        auto id_it = _store->get_id_to_socket_map().find(client_id);
        if (id_it != _store->get_id_to_socket_map().end()) {
            auto aggregate = get_aggregate(id_it->second);
            if (aggregate) {
                return aggregate->client;
            }
        }

        LOG_WARNING("[ClientRepositoryHandler] Client not found for id: {}", client_id);
        return nullptr;

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientRepositoryHandler] Exception in get_client_by_id: {}", e.what());
        return nullptr;
    }
}

socket_t ClientRepositoryHandler::get_socket_by_client_id(const std::string& client_id)
{
    LOG_TRACE;
    try {
        if (client_id.empty()) {
            LOG_ERROR("[ClientRepositoryHandler] Client ID is empty");
            return INVALID_SOCKET_FD;
        }

        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        auto it = _store->get_id_to_socket_map().find(client_id);
        if (it != _store->get_id_to_socket_map().end()) {
            return it->second;
        }

        LOG_WARNING("[ClientRepositoryHandler] Socket not found for client id: {}", client_id);
        return INVALID_SOCKET_FD;

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientRepositoryHandler] Exception in get_socket_by_client_id: {}", e.what());
        return INVALID_SOCKET_FD;
    }
}

std::shared_ptr<Client> ClientRepositoryHandler::get_client_by_socket(socket_t socket)
{
    LOG_TRACE;
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        auto aggregate = get_aggregate(socket);
        if (!aggregate) {
            return nullptr;
        }

        return aggregate->client;

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientRepositoryHandler] Exception in get_client_by_socket: {}", e.what());
        return nullptr;
    }
}
std::string ClientRepositoryHandler::get_client_id(socket_t socket)
{
    LOG_TRACE;
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        auto aggregate = get_aggregate(socket);
        if (!aggregate || !aggregate->client) {
            LOG_WARNING("[ClientRepositoryHandler] Client not found for socket {}", socket);
            return "";
        }

        return aggregate->client->get_id();

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientRepositoryHandler] Exception in get_client_id: {}", e.what());
        return "";
    }
}

std::string ClientRepositoryHandler::get_client_id(const std::shared_ptr<Client> &client)
{
    LOG_TRACE;
    try {
        if (!client) {
            LOG_ERROR("[ClientRepositoryHandler] Client pointer is null");
            return "";
        }

        return client->get_id();

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientRepositoryHandler] Exception in get_client_id (by client): {}", e.what());
        return "";
    }
}

// Collection queries
size_t ClientRepositoryHandler::get_connected_count() const
{
    LOG_TRACE;
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        return _store->get_aggregates().size();

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientRepositoryHandler] Exception in get_connected_count: {}", e.what());
        return 0;
    }
}

std::vector<socket_t> ClientRepositoryHandler::get_all_sockets()
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

        LOG_INFO("[ClientRepositoryHandler] Retrieved {} sockets", sockets.size());

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientRepositoryHandler] Exception in get_all_sockets: {}", e.what());
    }
    
    return sockets;
}

std::vector<socket_t> ClientRepositoryHandler::get_inactive_clients(std::chrono::seconds timeout)
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

        LOG_INFO("[ClientRepositoryHandler] Found {} inactive clients (timeout: {}s)", 
                 inactive_sockets.size(), timeout.count());

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientRepositoryHandler] Exception in get_inactive_clients: {}", e.what());
    }
    
    return inactive_sockets;
}

// Bulk operations
void ClientRepositoryHandler::update_all_last_activity()
{
    LOG_TRACE;
    try {
        auto now = std::chrono::steady_clock::now();
        
        std::unique_lock<std::shared_mutex> lock(_store->get_mutex());
        
        auto& aggregates = _store->get_aggregates();
        for (auto& [socket, aggregate] : aggregates) {
            aggregate.last_activity = now;
        }

        LOG_INFO("[ClientRepositoryHandler] Updated last activity for {} clients", aggregates.size());

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientRepositoryHandler] Exception in update_all_last_activity: {}", e.what());
    }
}

void ClientRepositoryHandler::clear_all_heartbeats()
{
    LOG_TRACE;
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        const auto& aggregates = _store->get_aggregates();
        for (const auto& [socket, aggregate] : aggregates) {
            if (aggregate.client) {
                aggregate.client->clear_heart_beat_count();
            }
        }

        LOG_DEBUG("[ClientRepositoryHandler] Cleared heartbeat counts for {} clients", aggregates.size());

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientRepositoryHandler] Exception in clear_all_heartbeats: {}", e.what());
    }
}

// Private helper methods
ClientAggregateStore::ClientAggregate* ClientRepositoryHandler::get_aggregate(socket_t socket)
{
    auto it = _store->get_aggregates().find(socket);
    if (it != _store->get_aggregates().end()) {
        return &(it->second);
    }
    return nullptr;
}

bool ClientRepositoryHandler::validate_client_data(const std::shared_ptr<Client>& client)
{
    if (!client) {
        LOG_ERROR("[ClientRepositoryHandler] Client pointer is null");
        return false;
    }

    // 基本的なバリデーション
    if (client->get_name().empty()) {
        LOG_WARNING("[ClientRepositoryHandler] Client name is empty");
    }

    return true;
}


std::map<socket_t, std::shared_ptr<Client>> ClientRepositoryHandler::get_all_clients() const
{
    LOG_TRACE;
    std::map<socket_t, std::shared_ptr<Client>> result;
    
    try {
        std::shared_lock<std::shared_mutex> lock(_store->get_mutex());
        
        const auto& connections = _store->get_aggregates();
        
        for (const auto& [socket, aggregate] : connections) {
            if (aggregate.client != nullptr) {
                result[socket] = aggregate.client;
            }
        }
        
        LOG_DEBUG("[ClientRepositoryHandler] Retrieved {} clients", result.size());
        return result;

    } catch (const std::exception& e) {
        LOG_ERROR("[ClientRepositoryHandler] Exception in get_all_clients: {}", e.what());
        return result; // 空のマップを返す
    }
}
