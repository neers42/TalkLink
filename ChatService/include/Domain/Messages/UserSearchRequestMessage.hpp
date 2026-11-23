#ifndef USERSEARCHREQUESTMESSAGE_HPP
#define USERSEARCHREQUESTMESSAGE_HPP
#include "Utils/common.hpp"
#include "MessageBase.hpp"
#include "IReceiveMessage.hpp"

class UserSearchRequestMessage :
    public MessageBase, public IReceiveMessage
{
private:
    std::string _query;
public:
    //コンストラクタ
    UserSearchRequestMessage();
    UserSearchRequestMessage(const MessageBase::CommonField& common_fields, const std::string& query);
    //デストラクタ
    ~UserSearchRequestMessage();

    std::string get_query() const;
    void set_query(const std::string& query);

    //メッセージインスタンスの内容検証
    bool validate_payload(const json& message) const override;
    std::unique_ptr<MessageBase> create_instance(const json& message) const override;
    bool validate_value() const override;

private:
    bool is_valid_query(const std::string& query) const;
};



#endif // USERSEARCHREQUESTMESSAGE_HPP