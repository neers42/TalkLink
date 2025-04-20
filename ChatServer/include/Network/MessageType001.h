#ifndef MESSAGETYPE001_H_
#define MESSAGETYPE001_H_
#include "BaseMessage.h"
#include "common.h"
class MessageType001 :
    public BaseMessage
{
public:
    enum class SignType
    {
        SIGNUP,
        SIGNIN,
        SIGNOUT
    };

    enum class AuthType
    {
        NORMAL,
        GOOGLE,
        FACEBOOK,
        TWITTER,
        MICROSOFT,
        YAHOO
    };
private:
    SignType _sign_type;
    AuthType _auth_type;
    std::string _client_name;
    std::string _client_mail_addr;
    std::string _client_password;

public:
    //コンストラクタ
    MessageType001() = delete;
    MessageType001(SignType s_type, AuthType a_type, const std::string& name, const std::string& mail_addr, const std::string& password);
    MessageType001(int s_type, int a_type, const std::string& name, const std::string& mail_addr, const std::string& password);
    MessageType001(ContentType c_type, const std::string& mt, SignType s_type, AuthType a_type, const std::string& name, const std::string& mail_addr, const std::string& password);
    MessageType001(int c_type, const std::string& mt, int s_type, int a_type, const std::string& name, const std::string& mail_addr, const std::string& password);
    //デストラクタ
    ~MessageType001();

    //各種アクセッサ(getter/setter)
    SignType get_sign_type() const;
	int get_sign_type_int() const;
    void set_sign_type(SignType s_type);
    void set_sign_type(int s_type);

    AuthType get_auth_type() const;
    void set_auth_type(AuthType a_type);
    void set_auth_type(int a_type);

    std::string get_client_name() const;
    void set_client_name(const std::string& name);

    std::string get_client_mail_addr() const;
    void set_clilent_mail_addr(const std::string& mail_addr);

    std::string get_client_password() const;
    void set_client_password(const std::string& password);

    bool is_valid() override;
    static std::unique_ptr<MessageType001> serialize_message(const json& json_message);

private:
    bool is_valid_name(const std::string& name);
    bool is_valid_mail_addr(const std::string& mail_addr);
    bool is_valid_password(const std::string& password);

    SignType exchange_sign_type_from_int(const int value);
    AuthType exchange_auth_type_from_int(const int value);
};
#endif // !MESSAGETYPE001



