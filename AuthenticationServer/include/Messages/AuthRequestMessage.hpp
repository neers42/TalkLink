#ifndef AUTHREQUESTMESSAGE_HPP
#define AUTHREQUESTMESSAGE_HPP
#include "common.hpp"
#include "MessageBase.hpp"
#include "IReceiveMessage.hpp"

class AuthRequestMessage : public MessageBase, public IReceiveMessage {
private:
    std::string jwt;

public:
    AuthRequestMessage();
    AuthRequestMessage(const MessageBase::CommonField& common_fields, const std::string& jwt);
    AuthRequestMessage(const std::string& jwt, const std::string& correlation_id);
    std::string get_jwt() const;

    // IReceiveMessageの実装
    bool validate_payload(const json& message) const override;
    std::unique_ptr<MessageBase> create_instance(const json& message) const override;
    bool validate_value() const override;

private:
    bool is_valid_jwt(const std::string& jwt) const;
    bool is_valid_base64(const std::string& str) const;

};

#endif // AUTHREQUESTMESSAGE_HPP