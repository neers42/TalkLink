#ifndef USERSETTINGCONTROLMESSAGE_HPP
#define USERSETTINGCONTROLMESSAGE_HPP
#include "MessageBase.hpp"
#include "IReceiveMessage.hpp"
class UserSettingControlMessage :
    public MessageBase, public IReceiveMessage
{
public:
    //コンストラクタ
    UserSettingControlMessage();
    ~UserSettingControlMessage();

    //IReceiveMessageの実装
    bool validate_payload(const json& message) const override;
    std::unique_ptr<MessageBase> create_instance(const json& message) const override;
    bool validate_value() const override;
};
#endif // !USERSETTINGCONTROLMESSAGE_HPP



