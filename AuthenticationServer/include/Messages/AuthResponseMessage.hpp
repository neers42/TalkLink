#ifndef AUTH_RESPONSE_MESSAGE_HPP
#define AUTH_RESPONSE_MESSAGE_HPP
#include "common.hpp"
#include "MessageBase.hpp"
#include "ISendMessage.hpp"

class AuthResponseMessage : public MessageBase, public ISendMessage {
public:
    AuthResponseMessage();
    AuthResponseMessage(bool _verify_status, const std::uint64_t ttl);
    bool get_verify_status() const;
    std::uint64_t get_ttl() const;

    // ISendMessageの純粋仮想関数の実装
    json serialize_specific_message_fields() const override;
    bool validate_specific_message_fields() const override;

private:
    bool _verify_status;
    std::uint64_t ttl;
};

#endif // AUTH_RESPONSE_MESSAGE_HPP