#ifndef CLIENTSETTINGCONTROLMESSAGE_HPP
#define CLIENTSETTINGCONTROLMESSAGE_HPP
#include "MessageBase.hpp"
#include "ISendMessage.hpp"

class ClientSettingControlResponseMessage :
    public MessageBase, public ISendMessage
{
public:
    //コンストラクタ
    ClientSettingControlResponseMessage();
    ~ClientSettingControlResponseMessage();

    //ISendMessageの実装
    json serialize_specific_message_fields() const override;
    bool validate_specific_message_fields() const override;
};

#endif // !CLIENTSETTINGCONTROLMESSAGE_HPP


