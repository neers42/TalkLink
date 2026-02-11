#include "MessageBase.hpp"

MessageBase::MessageBase()
    :_common_fields()
{
}

MessageBase::MessageBase(const MessageBase::MessageType mt)
    :_common_fields{mt, ""}
{
}

MessageBase::MessageBase(const MessageBase::MessageType mt, const std::string &id)
    :_common_fields{mt, id}
{
}

MessageBase::MessageBase(const CommonField& common_fields)
    :_common_fields(common_fields)
{
}

MessageBase::MessageBase(const CommonField& common_fields, const MessageBase::MessageType mt)
    :_common_fields(common_fields)
{
    // メッセージタイプを上書き
    this->_common_fields._message_type = mt;
}

MessageBase::~MessageBase()
{
}

// メッセージタイプ文字列をMessageType列挙型に変換する関数
MessageBase::MessageType MessageBase::parse_message_type(const std::string& message_type_str) {
        if (message_type_str == "CHAT_MESSAGE") {
            return MessageBase::MessageType::CHAT_MESSAGE;
        } else if (message_type_str == "CHAT_ACK") {
            return MessageBase::MessageType::CHAT_ACK;
        } else if (message_type_str == "CHAT_DELIVERY") {
            return MessageBase::MessageType::CHAT_DELIVERY;
        } else if (message_type_str == "CHATROOM_CONTROL") {
            return MessageBase::MessageType::CHATROOM_CONTROL;
        } else if (message_type_str == "CHATROOM_CONTROL_ACK") {
            return MessageBase::MessageType::CHATROOM_CONTROL_ACK;
        } else if (message_type_str == "CHATROOM_CONTROL_DELIVERY") {
            return MessageBase::MessageType::CHATROOM_CONTROL_DELIVERY;
        } else if (message_type_str == "HEARTBEAT_MESSAGE") {
            return MessageBase::MessageType::HEARTBEAT_MESSAGE;
        } else if (message_type_str == "AUTHENTICATION_REQUEST") {
            return MessageBase::MessageType::AUTHENTICATION_REQUEST;
        } else if (message_type_str == "AUTHENTICATION_RESPONSE") {
            return MessageBase::MessageType::AUTHENTICATION_RESPONSE;
        } else if (message_type_str == "SESSION_REQUEST") {
            return MessageBase::MessageType::SESSION_REQUEST;
        } else if (message_type_str == "SESSION_RESPONSE") {
            return MessageBase::MessageType::SESSION_RESPONSE;
        } else if (message_type_str == "INCLIMENTAL_SEARCH") {
            return MessageBase::MessageType::INCLIMENTAL_SEARCH;
        } else if (message_type_str == "SETTING_CONTROL") {
            return MessageBase::MessageType::SETTING_CONTROL;
        } else {
            throw std::invalid_argument("Unknown message type string: " + message_type_str);
        }
    }

std::string MessageBase::message_type_to_string(const MessageBase::MessageType &message_type) const
{
    switch (message_type) {
        case MessageBase::MessageType::CHAT_MESSAGE:
            return "CHAT_MESSAGE";
        case MessageBase::MessageType::CHAT_ACK:
            return "CHAT_ACK";
        case MessageBase::MessageType::CHAT_DELIVERY:
            return "CHAT_DELIVERY";
        case MessageBase::MessageType::CHATROOM_CONTROL:
            return "CHATROOM_CONTROL";
        case MessageBase::MessageType::CHATROOM_CONTROL_ACK:
            return "CHATROOM_CONTROL_ACK";
        case MessageBase::MessageType::CHATROOM_CONTROL_DELIVERY:
            return "CHATROOM_CONTROL_DELIVERY";
        case MessageBase::MessageType::HEARTBEAT_MESSAGE:
            return "HEARTBEAT_MESSAGE";
        case MessageBase::MessageType::AUTHENTICATION_REQUEST:
            return "AUTHENTICATION_REQUEST";
        case MessageBase::MessageType::AUTHENTICATION_RESPONSE:
            return "AUTHENTICATION_RESPONSE";
        case MessageBase::MessageType::SESSION_REQUEST:
            return "SESSION_REQUEST";
        case MessageBase::MessageType::SESSION_RESPONSE:
            return "SESSION_RESPONSE";
        case MessageBase::MessageType::INCLIMENTAL_SEARCH:
            return "INCLIMENTAL_SEARCH";
        case MessageBase::MessageType::SETTING_CONTROL:
            return "SETTING_CONTROL";
        default:
            LOG_ERROR("Unknown MessageType enum value.");
            break;
    }
    return std::string();
}

bool MessageBase::validate_common_json_fields(const json &message)
{

    try
    {
        // 1. 必須キーがすべて存在するかチェック
        static const std::vector<std::string> required_keys = {
            "message_type", "timestamp", "correlation_id"
        };

        for (const auto& key : required_keys) {
            if (!message.contains(key)) {
                throw std::runtime_error("Missing key: " + key);
            }
        }
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("Key check failed: {}", e.what());
        return false;
    }

    return true;
}

json MessageBase::serialize_common_message_fields(const MessageBase &message)
{
    json common_json;
    try
    {
        if (validate_common_message_fields(message) == false)
        {
            throw std::runtime_error("Common message field validation failed during serialization.");
        }
        common_json["message_type"] = message.message_type_to_string(message._common_fields._message_type);
        common_json["timestamp"] = message._common_fields._timestamp;
        common_json["correlation_id"] = message._common_fields._correlation_id;
    }
    catch (const std::runtime_error &re)
    {
        LOG_ERROR("Runtime error : {}", re.what());
        return json{};
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Serialization failed: {}", e.what());
        return json{};
    }
    return common_json;
}

bool MessageBase::validate_common_message_fields(const MessageBase &message)
{
    try
    {
        if (message._common_fields._timestamp.empty())
        {
            throw std::runtime_error("Time stamp is empty");
        }
        if (message._common_fields._correlation_id.empty())
        {
            throw std::runtime_error("Correlation ID is empty");
        }
        return true;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Common message field validation failed: {}", e.what());
        return false;
    }
}

MessageBase::CommonField MessageBase::parse_common_fields(const json &message) const
{
    try
    {
        // 他のフィールドの解析
        std::string message_type_str = message.at("message_type").get<std::string>();
        std::string correlation_id = message.at("correlation_id").get<std::string>();
        std::string raw_timestamp = message.at("timestamp").get<std::string>();
        std::string timestamp = normalize_timestamp(raw_timestamp); // タイムスタンプを正規化
        
        // MessageTypeに変換
        MessageBase::MessageType message_type = parse_message_type(message_type_str);
        
        // CommonField構造体を作成して返す
        CommonField common_field;
        common_field._message_type = message_type;
        common_field._correlation_id = correlation_id;
        common_field._timestamp = timestamp;
        
        return common_field;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Error parsing common fields: {}", e.what());
        throw; // 再スローして呼び出し元で処理させる
    }
}

std::string MessageBase::normalize_timestamp(const std::string& timestamp) {
    if (timestamp.empty()) {
        return ServerTime::get_current_time_to_milliseconds();
    }
    
    // ISO 8601形式かどうかチェック
    if (is_iso8601_format(timestamp)) {
        return timestamp; // すでに正しい形式
    }
    
    // 従来の形式 "YYYY-MM-DD HH:MM:SS.mmm" から ISO 8601形式に変換
    try {
        // "2025-12-19 22:36:27.073" -> "2025-12-19T22:36:27.0730000+09:00"
        std::string result = timestamp;
        
        // 日付と時刻の間のスペースをTに置換
        size_t space_pos = result.find(' ');
        if (space_pos != std::string::npos) {
            result[space_pos] = 'T';
        }
        
        // ミリ秒部分を7桁に拡張
        size_t dot_pos = result.find('.');
        if (dot_pos != std::string::npos) {
            std::string fraction = result.substr(dot_pos + 1);
            if (fraction.length() == 3) {
                // 3桁を7桁に拡張
                result = result.substr(0, dot_pos + 1) + fraction + "0000";
            }
        } else {
            // ミリ秒部分がない場合は追加
            result += ".0000000";
        }
        
        // タイムゾーンを追加
        result += "+09:00";
        
        return result;
    } catch (const std::exception& e) {
        LOG_ERROR("Error normalizing timestamp '{}': {}", timestamp, e.what());
        return ServerTime::get_current_time_to_milliseconds();
    }
}

bool MessageBase::is_iso8601_format(const std::string& timestamp) {
    // ISO 8601形式の基本的なパターンをチェック
    // "YYYY-MM-DDTHH:MM:SS.fffffffZ" または "YYYY-MM-DDTHH:MM:SS.fffffff+HH:MM"
    if (timestamp.length() < 20) {
        return false;
    }
    
    // 'T'が存在するかチェック
    size_t t_pos = timestamp.find('T');
    if (t_pos == std::string::npos) {
        return false;
    }
    
    // タイムゾーン指定があるかチェック（Z、+、-のいずれか）
    return (timestamp.find('Z') != std::string::npos ||
            timestamp.find('+') != std::string::npos ||
            timestamp.find('-', 10) != std::string::npos); // 10文字目以降の'-'（日付部分の'-'を除外）
}
