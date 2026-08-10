#ifndef IMESSAGEDELIVERYSERVICE_HPP
#define IMESSAGEDELIVERYSERVICE_HPP

#include "Utils/common.hpp"

/**
 * @brief メッセージ配信サービスの抽象インターフェース
 * 
 * Service間の結合を解消し、配信責務を分離
 */
class IMessageDeliveryService {
public:
    virtual ~IMessageDeliveryService() = default;
    
    // 配信結果
    struct DeliveryResult {
        bool success;
        int sent_count;
        std::vector<std::string> unsent_uids;
        std::string error_message;
        std::uint64_t room_sequence = 0;
    };
    
    /**
     * @brief ルームへのメッセージ配信
     * @param message_json シリアライズ済みメッセージ
     * @param room_id 対象ルームID
     * @param sender_socket 送信者ソケット
     * @param sender_uid 送信者ID
     * @return 配信結果
     */
    virtual DeliveryResult deliver_to_room(
        const json& message_json,
        const std::string& room_id,
        socket_t sender_socket,
        const std::string& uid
    ) = 0;
    
    /**
     * @brief P2Pメッセージ配信
     * @param message_json シリアライズ済みメッセージ
     * @param target_uid 受信者ID
     * @return 配信結果
     */
    virtual DeliveryResult deliver_p2p(
        const json& message_json,
        const std::string& target_uid
    ) = 0;
    
    /**
     * @brief ブロードキャスト配信
     * @param message_json シリアライズ済みメッセージ
     * @param uid 送信者ID
     * @param exclude_sender 送信者除外フラグ
     * @return 配信結果
     */
    virtual DeliveryResult broadcast(
        const json& message_json,
        const std::string& uid,
        bool exclude_sender = true
    ) = 0;
};

#endif // IMESSAGEDELIVERYSERVICE_HPP