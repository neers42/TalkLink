#include "MessageBase.hpp"

MessageBase::MessageBase()
    :_content_type{ ContentType::TEXT }, 
    _message_type{""}, 
    _time_stamp{ServerTime::get_current_time_to_milliseconds()}, 
    _id{0}
{
}

MessageBase::MessageBase(ContentType c_type, const std::string &mt)
    :_content_type{ c_type },
    _message_type{mt},
    _time_stamp{ServerTime::get_current_time_to_milliseconds()},
    _id{0}
{
}

MessageBase::MessageBase(int c_type, const std::string &mt)
    :_content_type{ static_cast<ContentType>(c_type) }, 
    _message_type{mt}, 
    _time_stamp{ServerTime::get_current_time_to_milliseconds()}, 
    _id{0}
{
}

MessageBase::MessageBase(ContentType c_type, const std::string &m_type, const uint64_t id)
    :_content_type{ c_type }, 
    _message_type{m_type}, 
    _time_stamp{ServerTime::get_current_time_to_milliseconds()}, 
    _id{id}
{
}

MessageBase::MessageBase(ContentType c_type, const std::string &m_type, const uint64_t id, const std::string &time_stamp)
    : _content_type{c_type}, _message_type{m_type}, _id{id}, _time_stamp{time_stamp}
{
}

MessageBase::MessageBase(int c_type, const std::string &m_type, const uint64_t id, const std::string &time_stamp)
: _content_type{static_cast<ContentType>(c_type)}, _message_type{m_type}, _id{id}, _time_stamp{time_stamp}
{
}

MessageBase::~MessageBase()
{
}

bool MessageBase::validate_common_json_fields(const json &message)
{

    try
    {
        // 1. 必須キーがすべて存在するかチェック
        static const std::vector<std::string> required_keys = {
            "content_type", "message_type", "time_stamp", "message_id"
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
        switch (message._content_type)
        {
        case MessageBase::ContentType::TEXT:
            // テキストメッセージのシリアライズ
            common_json["content_type"] = "TEXT";
            break;
        case MessageBase::ContentType::IMAGE:
            // 画像メッセージのシリアライズ
            common_json["content_type"] = "IMAGE";
            break;
        case MessageBase::ContentType::VIDEO:
            // 動画メッセージのシリアライズ
            common_json["content_type"] = "VIDEO";
            break;
        case MessageBase::ContentType::NOTIFICATION:
            // 通知メッセージのシリアライズ
            common_json["content_type"] = "NOTIFICATION";
            break;
        case MessageBase::ContentType::HEARTBEAT:
            // ハートビートメッセージのシリアライズ
            common_json["content_type"] = "HEARTBEAT";
            break;
        case MessageBase::ContentType::AUTHENTICATION:
            // 認証メッセージのシリアライズ
            common_json["content_type"] = "AUTHENTICATION";
            break;

        default:
            break;
        }
        common_json["message_type"] = message._message_type;
        common_json["time_stamp"] = message._time_stamp;
        common_json["message_id"] = message._id;
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
        switch (message._content_type)
        {
        case MessageBase::ContentType::TEXT:
            // テキストメッセージのバリデーション
            break;
        case MessageBase::ContentType::IMAGE:
            // 画像メッセージのバリデーション
            break;
        case MessageBase::ContentType::VIDEO:
            // 動画メッセージのバリデーション
            break;
        case MessageBase::ContentType::NOTIFICATION:
            // 通知メッセージのバリデーション
            break;
        case MessageBase::ContentType::HEARTBEAT:
            // ハートビートメッセージのバリデーション
            break;
        case MessageBase::ContentType::AUTHENTICATION:
            // 認証メッセージのバリデーション
            break;

        default:
            throw std::runtime_error("Invalid content type");
            break;
        }
        if (message._message_type.empty())
        {
            throw std::runtime_error("Message type is empty");
        }
        if (message._time_stamp.empty())
        {
            throw std::runtime_error("Time stamp is empty");
        }
        if (message._id < 0)
        {
            throw std::runtime_error("Message ID is invalid");
        }
        return true;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Common message field validation failed: {}", e.what());
        return false;
    }
}

std::tuple<MessageBase::ContentType, std::string, uint64_t, std::string> MessageBase::parse_common_fields(const json &message) const
{
    MessageBase::ContentType content_type;
    std::string message_type;
    uint64_t message_id;
    std::string time_stamp;

    try
    {
        // content_typeの解析
        std::string content_type_str = message.at("content_type").get<std::string>();
        if (content_type_str == "TEXT") {
            content_type = MessageBase::ContentType::TEXT;
        }
        else if (content_type_str == "IMAGE") {
            content_type = MessageBase::ContentType::IMAGE;
        }
        else if (content_type_str == "VIDEO") {
            content_type = MessageBase::ContentType::VIDEO;
        }
        else if (content_type_str == "NOTIFICATION") {
            content_type = MessageBase::ContentType::NOTIFICATION;
        }
        else if (content_type_str == "HEARTBEAT") {
            content_type = MessageBase::ContentType::HEARTBEAT;
        }
        else if (content_type_str == "AUTHENTICATION") {
            content_type = MessageBase::ContentType::AUTHENTICATION;
        }
        else {
            throw std::runtime_error("Invalid content type string: " + content_type_str);
        }

        // 他のフィールドの解析
        message_type = message.at("message_type").get<std::string>();
        message_id = message.at("message_id").get<uint64_t>();
        time_stamp = message.at("time_stamp").get<std::string>();
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Error parsing common fields: {}", e.what());
        throw; // 再スローして呼び出し元で処理させる
    }

    return std::make_tuple(content_type, message_type, message_id, time_stamp);
}
