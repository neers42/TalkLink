#include "Domain/Messages/UserSearchResponseMessage.hpp"

UserSearchResponseMessage::UserSearchResponseMessage(const MessageBase::CommonField &common_fields, const json &search_results, const std::string &error_message)
    : MessageBase(common_fields, MessageBase::MessageType::USER_SEARCH_RESPONSE),
      _search_results(search_results),
      _error_message{error_message}
{
}

UserSearchResponseMessage::~UserSearchResponseMessage()
{
}

json UserSearchResponseMessage::serialize_payload() const
{
    json payload, specific_fields;
    try{
        if (validate_payload() == false)
        {
            throw std::runtime_error("UserSearchResponseMessage specific fields validation failed during serialization.");
        }
        if(this->_search_results.is_array())
        {
            specific_fields["search_results"] = this->_search_results;
        }
        else
        {
            specific_fields["search_results"] = json::array();
        }
        specific_fields["error_message"] = this->_error_message;
        payload["payload"] = specific_fields;
        return payload;
    }catch(const std::exception &e)
    {
        LOG_ERROR("[UserSearchResponseMessage] Error serializing UserSearchResponseMessage specific fields: {}", e.what());
        return json(); // 空のJSONオブジェクトを返す
    }
}

bool UserSearchResponseMessage::validate_payload() const
{
        try
        {
            // search_resultsは配列であることを確認
            if (!this->_search_results.is_array())
            {
                throw std::runtime_error("search_results must be an array");
            }
            // _error_message は std::string 型なので追加チェック不要
            return true;
        }
        catch (const std::runtime_error &re)
        {
            LOG_ERROR("[UserSearchResponseMessage] Runtime error : {}", re.what());
            return false;
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("[UserSearchResponseMessage] Error validating UserSearchResponseMessage payload: {}", e.what());
            return false;
        }
    return true;
}
