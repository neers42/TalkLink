#ifndef MESSAGETYPE001_HPP
#define MESSAGETYPE001_HPP
#include "BaseMessage.hpp"
#include "Utils/common.hpp"
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

private:
    SignType _sign_type;
    std::string _jwt_token;
    std::string _client_name;
    std::string _client_mail_addr;
    std::string _client_password;

public:
    //コンストラクタ
    MessageType001() = delete;
    MessageType001(SignType s_type, const std::string& jwt_token, const std::string& name, const std::string& mail_addr);
    MessageType001(int s_type, const std::string& jwt_token, const std::string& name, const std::string& mail_addr);
    MessageType001(ContentType c_type, const std::string& mt, SignType s_type, const std::string& jwt_token, const std::string& name, const std::string& mail_addr);
    MessageType001(int c_type, const std::string& mt, int s_type, const std::string& jwt_token, const std::string& name, const std::string& mail_addr);
    //デストラクタ
    ~MessageType001();

    //各種アクセッサ(getter/setter)
    SignType get_sign_type() const;
	int get_sign_type_int() const;
    void set_sign_type(SignType s_type);
    void set_sign_type(int s_type);

    std::string get_jwt_token() const;
    void set_jwt_token(const std::string& jwt_token);

    std::string get_client_name() const;
    void set_client_name(const std::string& name);

    std::string get_client_mail_addr() const;
    void set_client_mail_addr(const std::string& mail_addr);

    bool is_valid() override;
    static std::unique_ptr<MessageType001> serialize_message(const json& json_message);

private:
    bool is_valid_jwt_token(const std::string& jwt_token);
    bool is_valid_name(const std::string& name);
    bool is_valid_mail_addr(const std::string& mail_addr);

    SignType exchange_sign_type_from_int(const int value);
};
#endif // !MESSAGETYPE001_HPP



