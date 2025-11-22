#ifndef CLIENTSESSIONMESSAGE_HPP
#define CLIENTSESSIONMESSAGE_HPP
#include "Utils/common.hpp"
#include "MessageBase.hpp"
#include "IReceiveMessage.hpp"
class ClientSessionMessage :
    public MessageBase, public IReceiveMessage
{
public:
    enum class SignType
    {
        SIGNUP,
        SIGNIN,
        SIGNOUT
    };

private:
    SignType _sign_type;
    std::string _jwt;
    std::string _client_name;
    std::string _client_mail_addr;
    uint64_t _last_received_message_id;

public:
    //コンストラクタ
    ClientSessionMessage();
    ClientSessionMessage(SignType s_type, const std::string& jwt, const std::string& name, const std::string& mail_addr, const uint64_t last_received_message_id);
    ClientSessionMessage(int s_type, const std::string& jwt, const std::string& name, const std::string& mail_addr, const uint64_t last_received_message_id);
    ClientSessionMessage(ContentType c_type, const std::string& mt, SignType s_type, const std::string& jwt, const std::string& name, const std::string& mail_addr, const uint64_t last_received_message_id);
    ClientSessionMessage(int c_type, const std::string& mt, int s_type, const std::string& jwt, const std::string& name, const std::string& mail_addr, const uint64_t last_received_message_id);
    //デストラクタ
    ~ClientSessionMessage();

    //各種アクセッサ(getter/setter)
    SignType get_sign_type() const;
	int get_sign_type_int() const;
    void set_sign_type(SignType s_type);
    void set_sign_type(int s_type);

    std::string get_jwt() const;
    void set_jwt(const std::string& jwt);

    std::string get_client_name() const;
    void set_client_name(const std::string& name);

    std::string get_client_mail_addr() const;
    void set_client_mail_addr(const std::string& mail_addr);

    uint64_t get_last_received_message_id() const;
    void set_last_received_message_id(uint64_t last_received_message_id);

    //メッセージインスタンスの内容検証
    bool validate_specific_fields(const json& message) const override;
    std::unique_ptr<MessageBase> create_instance(const json& message) const override;
    bool validate_value() const override;

private:
    bool is_valid_jwt(const std::string& jwt) const;
    bool is_valid_name(const std::string& name) const;
    bool is_valid_mail_addr(const std::string& mail_addr) const;

    SignType exchange_sign_type_from_int(const int value);
};
#endif // !MESSAGETYPE001_HPP



