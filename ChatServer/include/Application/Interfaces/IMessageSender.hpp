#ifndef IMESSAGESENDER_HPP
#define IMESSAGESENDER_HPP

#include "Utils/common.hpp"
#include "Application/DTO/DeliveryResult.hpp"

/**
 * @brief メッセージ配信サービスの抽象インターフェース
 * 
 * UseCase間の結合を解消し、配信責務を分離
 */
class IMessageSender {
public:
    virtual ~IMessageSender() = default;
    
    /**
     * @brief ルームへのメッセージ配信
     * @param message_json シリアライズ済みメッセージ
     * @param room_id 対象ルームID
     * @param sender_uid 送信者ID
     * @return 配信結果
     */
    virtual DeliveryResult deliver_to_room(
        const json& message_json,
        const std::string& room_id,
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

#endif // IMESSAGESENDER_HPP