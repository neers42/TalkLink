#include "Domain/Entity/Messages/AuthRequestMessage.hpp"


// コンストラクタ
AuthRequestMessage::AuthRequestMessage()
    : MessageBase(MessageBase::MessageType::AUTHENTICATION_REQUEST), jwt("")
{
}
AuthRequestMessage::AuthRequestMessage(const MessageBase::CommonField& common_fields, const std::string &jwt)
    : MessageBase(common_fields, MessageBase::MessageType::AUTHENTICATION_REQUEST), jwt(jwt)
{
}
AuthRequestMessage::AuthRequestMessage(const std::string &jwt, const std::string& correlation_id)
    : MessageBase(MessageBase::MessageType::AUTHENTICATION_REQUEST, correlation_id), jwt(jwt)
{
}

std::string AuthRequestMessage::get_jwt() const {
    return jwt;
}


bool AuthRequestMessage::validate_payload(const json &message) const
{
    try
    {
        // payloadキーの存在確認
        if (!message.contains("payload")) {
            throw std::runtime_error("Missing key: payload");
        }
        
        if (!message["payload"].is_object()) {
            throw std::runtime_error("Key 'payload' is not an object.");
        }
        
        // payload内のjwtキーの存在確認
        if (!message["payload"].contains("jwt")) {
            throw std::runtime_error("Missing key: jwt in payload");
        }

        if(!message["payload"]["jwt"].is_string()) {
            throw std::runtime_error("Key 'jwt' in payload has invalid type.");
        }
        
        return true;
    }
    catch(const std::runtime_error& re)
    {
        LOG_ERROR("Runtime error : {}", re.what());
        return false;
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("Exception : {}", e.what());
        return false;
    }
}

std::unique_ptr<MessageBase> AuthRequestMessage::create_instance(const json &message) const
{
    try
    {
        MessageBase::CommonField common_fields = parse_common_fields(message);
        std::string jwt = message.at("payload").at("jwt").get<std::string>();
        return std::make_unique<AuthRequestMessage>(common_fields, jwt);
    }
    catch(const std::runtime_error& re)
    {
        LOG_ERROR("Runtime error : {}", re.what());
        return nullptr;
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("Error creating AuthRequestMessage instance: {}", e.what());
        return nullptr;
    }
}

bool AuthRequestMessage::validate_value() const
{
    try
    {
        if(is_valid_jwt(this->jwt) == false){
            throw std::runtime_error("Invalid JWT value.");
        }
        LOG_DEBUG("AuthRequestMessage value validation succeeded.");
        return true;
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("Value validation failed: {}", e.what());
        return false;
    }
}

bool AuthRequestMessage::is_valid_jwt(const std::string &jwt) const
{
    try
    {
        // 1. JWTが空でないかチェック
        if (jwt.empty()) {
            LOG_ERROR("JWT is empty");
            return false;
        }
        
        // 2. JWTの長さチェック（異常に短い・長いトークンを排除）
        if (jwt.length() < 50 || jwt.length() > 2000) {
            LOG_ERROR("JWT length is invalid: {}", jwt.length());
            return false;
        }
        
        // 3. JWTの基本的なフォーマットチェック（header.payload.signature）
        size_t first_dot = jwt.find('.');
        size_t second_dot = jwt.find('.', first_dot + 1);
        
        if (first_dot == std::string::npos || second_dot == std::string::npos) {
            LOG_ERROR("Invalid JWT format: missing dots");
            return false;
        }
        
        // 4. JWTの各部分が空でないかチェック
        if (first_dot == 0 || 
            second_dot == first_dot + 1 || 
            second_dot == jwt.length() - 1) {
            LOG_ERROR("Invalid JWT format: empty sections");
            return false;
        }
        
        // 5. ヘッダー、ペイロード、シグネチャ部分を分割
        std::string header = jwt.substr(0, first_dot);
        std::string payload = jwt.substr(first_dot + 1, second_dot - first_dot - 1);
        std::string signature = jwt.substr(second_dot + 1);
        
        // 6. Base64文字セットのチェック（JWT用のURL-safe Base64）
        std::string allowed_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_=";
        
        if (header.find_first_not_of(allowed_chars) != std::string::npos ||
            payload.find_first_not_of(allowed_chars) != std::string::npos ||
            signature.find_first_not_of(allowed_chars) != std::string::npos) {
            LOG_ERROR("JWT contains invalid characters");
            return false;
        }
        
        // 7. ヘッダーとペイロードのBase64デコードチェック（簡易）
        if (!is_valid_base64(header) || !is_valid_base64(payload)) {
            LOG_ERROR("JWT header or payload is not valid Base64");
            return false;
        }
        
        LOG_DEBUG("JWT format validation passed");
        return true;
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Error validating JWT: {}", e.what());
        return false;
    }
}

bool AuthRequestMessage::is_valid_base64(const std::string &str) const
{
    // Base64の長さは4の倍数である必要がある（パディングを考慮）
    if (str.empty()) {
        return false;
    }
    
    // URL-safe Base64文字セット
    const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    
    for (char c : str) {
        if (c != '=' && base64_chars.find(c) == std::string::npos) {
            return false;
        }
    }
    
    return true;
}
