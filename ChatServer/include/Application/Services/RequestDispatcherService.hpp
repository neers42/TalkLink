#ifndef REQUESTDISPATCHERSERVICE_HPP
#define REQUESTDISPATCHERSERVICE_HPP

#include "Utils/common.hpp"
#include "SocketHandler.hpp"
#include "ThreadPoolHandler.hpp"
#include "Domain/CommunicationHandler.hpp"
#include "MessageBase.hpp"
#include <functional>
#include <memory>

// メッセージ処理結果
struct MessageDispatchResult {
    enum class Type {
        ACCEPT_REQUEST,      // 新しいクライアント接続
        HEARTBEAT,          // ハートビートメッセージ
        SESSION_REQUEST,    // セッションリクエスト
        CHAT_MESSAGE,       // チャットメッセージ
        CHATROOM_CONTROL,   // チャットルーム制御
        USER_SETTING_CONTROL,    // 設定制御
        USER_SEARCH_REQUEST,    // ユーザー検索リクエスト
        DISCONNECT,         // クライアント切断
        ERROR               // エラー
    };
    
    Type type;
    socket_t socket;
    json message_data;
    bool success;
    std::string error_message;
    
    MessageDispatchResult(Type t = Type::ERROR, socket_t s = INVALID_SOCKET_FD) 
        : type(t), socket(s), success(true) {}
};

class RequestDispatcherService {
public:
    // コールバック型定義
    using AcceptCallback = std::function<void()>;
    using MessageHandleCallback = std::function<void(const MessageDispatchResult&)>;

    RequestDispatcherService(
        std::shared_ptr<SocketHandler> socket_handler,
        std::shared_ptr<ThreadPoolHandler> thread_pool_handler,
        std::shared_ptr<CommunicationHandler> communication_handler
    );
    
    ~RequestDispatcherService() = default;

    // コールバック設定
    void set_accept_callback(AcceptCallback callback);
    void set_message_handle_callback(MessageHandleCallback callback);

    // メインのリクエスト振り分け処理
    void allocate_process(socket_t socket);

    // ソケット監視開始（SocketHandler直接アクセスを隠蔽）
    void start_socket_monitoring();

    // クライアント監視用のコールバック取得
    std::function<void(socket_t)> get_allocate_process_callback();

private:
    std::shared_ptr<SocketHandler> _socket_handler;
    std::shared_ptr<ThreadPoolHandler> _thread_pool_handler;
    std::shared_ptr<CommunicationHandler> _communication_handler;
    
    AcceptCallback _accept_callback;
    MessageHandleCallback _message_handle_callback;
    
    // メッセージ受信とタイプ判定
    MessageDispatchResult receive_and_dispatch_message(socket_t socket);
    
    // メッセージタイプの判定
    MessageDispatchResult::Type determine_message_type(const json& message);
    
    // ソケットがリッスンソケットかどうか判定
    bool is_listen_socket(socket_t socket);
};

#endif // REQUESTDISPATCHERSERVICE_HPP