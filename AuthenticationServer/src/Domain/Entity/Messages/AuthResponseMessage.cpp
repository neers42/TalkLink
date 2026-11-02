#include "Domain/Entity/Messages/AuthResponseMessage.hpp"

AuthResponseMessage::AuthResponseMessage()
    :MessageBase(MessageBase::MessageType::AUTHENTICATION_RESPONSE),_verify_status(false), ttl(0){
}
AuthResponseMessage::AuthResponseMessage(const MessageBase::CommonField& common_fields, bool _verify_status, const std::uint64_t ttl, const std::string session_token)
    :MessageBase(common_fields, MessageBase::MessageType::AUTHENTICATION_RESPONSE), _verify_status(_verify_status), ttl(ttl), _session_token(session_token)
{
}

bool AuthResponseMessage::get_verify_status() const
{
    return this->_verify_status;
}

std::uint64_t AuthResponseMessage::get_ttl() const
{
    return this->ttl;
}

json AuthResponseMessage::serialize_payload() const
{
    json payload, specific_fields;
    try
    {
        if(validate_payload() == false)
        {
            throw std::runtime_error("AuthResponseMessage specific fields validation failed during serialization.");
        }
        specific_fields["verify_result"] = this->_verify_status;
        specific_fields["ttl"] = this->ttl;
        specific_fields["session_token"] = this->_session_token;
        payload["payload"] = specific_fields;
        return payload;
    }
    catch(const std::runtime_error& re)
    {
        LOG_ERROR("Runtime error in AuthResponseMessage serialization: {}", re.what());
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("Exception in AuthResponseMessage serialization: {}", e.what());
    }
    return json();
}

bool AuthResponseMessage::validate_payload() const
{
    try
    {
        if (ttl < 0)
        {
            throw std::runtime_error("TTL cannot be negative: " + std::to_string(ttl));
        }
        return true;
    }
    catch(const std::runtime_error& re)
    {
        LOG_ERROR("Runtime error in AuthResponseMessage validation: {}", re.what());
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("Exception in AuthResponseMessage validation: {}", e.what());
    }
    
    return false;
}
