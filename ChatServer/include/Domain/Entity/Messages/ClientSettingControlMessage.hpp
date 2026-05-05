#ifndef CLIENTSETTINGCONTROLMESSAGE_HPP
#define CLIENTSETTINGCONTROLMESSAGE_HPP
#include "MessageBase.hpp"
#include "IReceiveMessage.hpp"
class ClientSettingControlMessage :
    public MessageBase, public IReceiveMessage
{
public:
    //コンストラクタ
    ClientSettingControlMessage();
    ~ClientSettingControlMessage();

    //IReceiveMessageの実装
    bool validate_payload(const json& message) const override;
    std::unique_ptr<MessageBase> create_instance(const json& message) const override;
    bool validate_value() const override;
};
#endif // !CLIENTSETTINGCONTROLMESSAGE_HPP



