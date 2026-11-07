#include "Application/Services/RequestDispatcherService.hpp"
#include "Domain/CommunicationHandler.hpp"
#include "MessageBase.hpp"

RequestDispatcherService::RequestDispatcherService(
    std::shared_ptr<SocketHandler> socket_handler,
    std::shared_ptr<CommunicationHandler> communication_handler
) : _socket_handler(std::move(socket_handler)),
    _communication_handler(std::move(communication_handler))
{
    if (!_socket_handler || !_communication_handler) {
        throw std::runtime_error("RequestDispatcherService dependencies cannot be null");
    }
    
    LOG_INFO("RequestDispatcherService initialized successfully");
}

void RequestDispatcherService::set_accept_callback(AcceptCallback callback) {
    _accept_callback = std::move(callback);
}

void RequestDispatcherService::set_message_handle_callback(MessageHandleCallback callback) {
    _message_handle_callback = std::move(callback);
}

void RequestDispatcherService::allocate_process(socket_t socket) {
    // socketがINVALIDの場合は処理をしない
    if (socket == INVALID_SOCKET_FD) {
        LOG_ERROR("[RequestDispatcherService] Invalid socket received: {}", socket);
        return;
    }
    
    // リッスンソケットかどうかで処理を振り分け
    if (is_listen_socket(socket)) {
        // 新しいクライアントの接続リクエスト
        LOG_DEBUG("[RequestDispatcherService] Listen socket detected - processing accept");
        if (_accept_callback) {
            _accept_callback();
        } else {
            LOG_WARNING("[RequestDispatcherService] Accept callback not set");
        }
    } else {
        // 既存クライアントからのメッセージ
        if (_message_handle_callback) {
            auto result = receive_and_dispatch_message(socket);
            // 結果の内容はこの直後に呼ばれるmessage_handle_callback側
            // ("Message dispatch callback triggered")で型名付きで出力されるため、
            // ここで同じ内容を重複して出力しない
            _message_handle_callback(result);
        } else {
            LOG_WARNING("[RequestDispatcherService] Message handle callback not set");
        }
    }
}

MessageDispatchResult RequestDispatcherService::receive_and_dispatch_message(socket_t socket) {
    MessageDispatchResult result(MessageDispatchResult::Type::ERROR, socket);
    
    try {
        // ソケット状態確認
        if (_socket_handler->is_socket_closed(socket)) {
            LOG_WARNING("[RequestDispatcherService] Socket {} is already closed - returning DISCONNECT", socket);
            result.type = MessageDispatchResult::Type::DISCONNECT;
            result.success = true;
            return result;
        }
        
        // メッセージの受信
        json receive_json = json::object();
        bool disconnect_flag = false;
        bool recv_flag = true;
        
        //オリジナルのJSONを受信
        std::tie(receive_json, disconnect_flag, recv_flag) = _communication_handler->receive_message(socket);
        
        LOG_DEBUG("[RequestDispatcherService] receive_message result for socket: {} - recv_flag: {}, disconnect_flag: {}, json_empty: {}", 
                  socket, recv_flag, disconnect_flag, receive_json.empty());
        
        if (disconnect_flag) {
            LOG_WARNING("[RequestDispatcherService] disconnect_flag=true from communication_handler for socket: {} - returning DISCONNECT", socket);
            result.type = MessageDispatchResult::Type::DISCONNECT;
            result.success = true;
            return result;
        }
        
        if (receive_json.empty() || !recv_flag) {
            LOG_ERROR("[RequestDispatcherService] Empty or invalid message for socket: {} - recv_flag: {}, json_empty: {}", 
                     socket, recv_flag, receive_json.empty());
            result.success = false;
            result.error_message = "Empty or invalid message received";
            return result;
        }
        
#ifdef USE_POLL
        // epollのONESHOTを再有効化
        _socket_handler->reactivate_epoll_ctrl(socket);
#endif
        
        // メッセージタイプの判定
        result.type = determine_message_type(receive_json);
        result.message_data = receive_json;
        result.success = true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("[RequestDispatcherService] Exception in receive_and_dispatch_message for socket {}: {}", socket, e.what());
        result.type = MessageDispatchResult::Type::ERROR;
        result.success = false;
        result.error_message = e.what();
    }
    
    return result;
}

MessageDispatchResult::Type RequestDispatcherService::determine_message_type(const json& message) {
    try {
        if (!message.contains("message_type")) {
            return MessageDispatchResult::Type::ERROR;
        }
        
        auto mt = MessageBase::parse_message_type(message.at("message_type").get<std::string>());
        
        switch (mt) {
            case MessageBase::MessageType::HEARTBEAT_MESSAGE:
                return MessageDispatchResult::Type::HEARTBEAT;
            case MessageBase::MessageType::SESSION_REQUEST:
                return MessageDispatchResult::Type::SESSION_REQUEST;
            case MessageBase::MessageType::CHAT_MESSAGE:
                return MessageDispatchResult::Type::CHAT_MESSAGE;
            case MessageBase::MessageType::CHATROOM_CONTROL:
                return MessageDispatchResult::Type::CHATROOM_CONTROL;
            case MessageBase::MessageType::USER_SETTING_CONTROL:
                return MessageDispatchResult::Type::USER_SETTING_CONTROL;
            case MessageBase::MessageType::USER_SEARCH_REQUEST:
                return MessageDispatchResult::Type::USER_SEARCH_REQUEST;
            case MessageBase::MessageType::SYNC_REQUEST:
                return MessageDispatchResult::Type::SYNC_REQUEST;
            default:
                return MessageDispatchResult::Type::ERROR;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("[RequestDispatcherService] Error determining message type: {}", e.what());
        return MessageDispatchResult::Type::ERROR;
    }
}

bool RequestDispatcherService::is_listen_socket(socket_t socket) {
    // TODO: リッスンソケットかどうかの判定ロジックを実装
    // 現在はシンプルな実装
    if (!_socket_handler) {
        LOG_ERROR("[RequestDispatcherService] SocketHandler is null");
        return false;
    }
    
    socket_t listen_socket = _socket_handler->get_listen_socket();
    return socket == listen_socket;
}

void RequestDispatcherService::start_socket_monitoring() {
#ifdef USE_POLL
    _socket_handler->monitor_socket_descriptor(
        get_allocate_process_callback());
#elif defined(USE_SELECT)
    // SELECT用の処理は別途実装が必要
    LOG_WARNING("[RequestDispatcherService] SELECT mode socket monitoring not fully implemented");
#endif
}

std::function<void(socket_t)> RequestDispatcherService::get_allocate_process_callback() {
    return [this](socket_t socket) {
        allocate_process(socket);
    };
}