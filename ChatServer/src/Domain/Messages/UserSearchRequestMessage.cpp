#include "Domain/Messages/UserSearchRequestMessage.hpp"

UserSearchRequestMessage::UserSearchRequestMessage()
    : MessageBase(MessageBase::MessageType::USER_SEARCH_REQUEST),
      _query{""}
{
}
UserSearchRequestMessage::UserSearchRequestMessage(const MessageBase::CommonField &common_fields, const std::string &query)
    : MessageBase(common_fields, MessageBase::MessageType::USER_SEARCH_REQUEST),
      _query{query}
{
}
UserSearchRequestMessage::~UserSearchRequestMessage()
{
}
std::string UserSearchRequestMessage::get_query() const
{
    return this->_query;
}

void UserSearchRequestMessage::set_query(const std::string &query)
{
    this->_query = query;
}

bool UserSearchRequestMessage::validate_payload(const json &message) const
{
        try
        {
            // payloadキーの存在をチェック
            if (!message.contains("payload"))
            {
                throw std::runtime_error("Missing key: payload");
            }
    
            const auto& payload = message["payload"];
    
            // 基本的な必須キー
            std::vector<std::string> required_keys = {
                "query",
            };
    
            for (const auto& key : required_keys)
            {
                if (!payload.contains(key))
                {
                    throw std::runtime_error("Missing key in payload: " + key);
                }
            }
    
            // クエリのバリデーション
            std::string query = payload["query"];
            if (!is_valid_query(query))
            {
                throw std::runtime_error("Invalid query value");
            }
    
            return true;
        }
        catch (const std::runtime_error &re)
        {
            LOG_ERROR("[UserSearchRequestMessage] Runtime error : {}", re.what());
            return false;
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("[UserSearchRequestMessage] Error validating UserSearchRequestMessage payload: {}", e.what());
            return false;
        }
}

std::unique_ptr<MessageBase> UserSearchRequestMessage::create_instance(const json &message) const
{
    try{
        const auto& payload = message["payload"];
        std::string query = payload["query"];

        return std::make_unique<UserSearchRequestMessage>(
            this->parse_common_fields(message),
            query
        );
    }
    catch (const std::runtime_error &re)
    {
        LOG_ERROR("[UserSearchRequestMessage] Runtime error : {}", re.what());
        return std::unique_ptr<MessageBase>();
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("[UserSearchRequestMessage] Error creating UserSearchRequestMessage instance: {}", e.what());
        return std::unique_ptr<MessageBase>();
    }
    return std::unique_ptr<MessageBase>();
}

bool UserSearchRequestMessage::validate_value() const
{
    return is_valid_query(this->_query);
}

bool UserSearchRequestMessage::is_valid_query(const std::string &query) const
{
    if(query.size() > 0){
        return true;
    }
    else{
        LOG_ERROR("[UserSearchRequestMessage] Query is invalid by byte size error(query byte size : {}).", query.size());
    }
    return false;
}
