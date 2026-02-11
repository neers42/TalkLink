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
    json serialize_payload() const override;
    bool validate_payload() const override;
};

#endif // !CLIENTSETTINGCONTROLMESSAGE_HPP


