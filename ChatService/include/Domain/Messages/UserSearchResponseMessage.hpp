#ifndef USERSEARCHRESPONSEMESSAGE_HPP
#define USERSEARCHRESPONSEMESSAGE_HPP
#include "MessageBase.hpp"
#include "ISendMessage.hpp"
class UserSearchResponseMessage :
    public MessageBase, public ISendMessage
{
private:
    json _search_results = json::array();
    std::string _error_message;

public:
    //コンストラクタ
    UserSearchResponseMessage() = delete;
    UserSearchResponseMessage(const MessageBase::CommonField& common_fields, const json& search_results, const std::string& error_message);
    //デストラクタ
    ~UserSearchResponseMessage();
    //各種アクセッサ(getter/setter)
    json get_search_results() const {
        return this->_search_results;
    }

    void set_search_results(const json& search_results) {
        this->_search_results = search_results;
    }

    std::string get_error_message() const {
        return this->_error_message;
    }

    void set_error_message(const std::string& error_message) {
        this->_error_message = error_message;
    }

    //ISendMessageの実装
    json serialize_payload() const override;
    bool validate_payload() const override;
};

#endif // !USERSEARCHRESPONSEMESSAGE_HPP