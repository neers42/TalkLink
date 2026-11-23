#ifndef USERSETTINGCONTROLRESPONSEMESSAGE_HPP
#define USERSETTINGCONTROLRESPONSEMESSAGE_HPP
#include "MessageBase.hpp"
#include "ISendMessage.hpp"

class UserSettingControlResponseMessage :
    public MessageBase, public ISendMessage
{
public:
    //コンストラクタ
    UserSettingControlResponseMessage();
    ~UserSettingControlResponseMessage();

    //ISendMessageの実装
    json serialize_payload() const override;
    bool validate_payload() const override;
};

#endif // !USERSETTINGCONTROLRESPONSEMESSAGE_HPP


