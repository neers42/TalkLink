#include "Messages/AuthResponseMessage.hpp"

AuthResponseMessage::AuthResponseMessage()
    :MessageBase(MessageBase::ContentType::AUTHENTICATION, "AuthResponse"),
     _verify_status(false), ttl(0){
}
AuthResponseMessage::AuthResponseMessage(bool _verify_status, const std::uint64_t ttl)
    :MessageBase(MessageBase::ContentType::AUTHENTICATION, "AuthResponse"),
      _verify_status(_verify_status),
      ttl(ttl)
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

json AuthResponseMessage::serialize_specific_message_fields() const
{
    json specific_fields;
    try
    {
        if(validate_specific_message_fields() == false)
        {
            throw std::runtime_error("AuthResponseMessage specific fields validation failed during serialization.");
        }
        specific_fields["verify_result"] = this->_verify_status;
        specific_fields["ttl"] = this->ttl;
        return specific_fields;
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

bool AuthResponseMessage::validate_specific_message_fields() const
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
