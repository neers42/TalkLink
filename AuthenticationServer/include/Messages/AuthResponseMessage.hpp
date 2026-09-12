#ifndef AUTH_RESPONSE_MESSAGE_HPP
#define AUTH_RESPONSE_MESSAGE_HPP
#include "common.hpp"
#include "MessageBase.hpp"
#include "ISendMessage.hpp"

class AuthResponseMessage : public MessageBase, public ISendMessage {
public:
    AuthResponseMessage();
    AuthResponseMessage(const MessageBase::CommonField& common_fields, bool _verify_status, const std::uint64_t ttl, const std::string session_token);
    bool get_verify_status() const;
    std::uint64_t get_ttl() const;

    // ISendMessageの純粋仮想関数の実装
    json serialize_payload() const override;
    bool validate_payload() const override;

private:
    bool _verify_status;
    std::uint64_t ttl;
    std::string _session_token;
};

#endif // AUTH_RESPONSE_MESSAGE_HPP