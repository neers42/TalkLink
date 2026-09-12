#ifndef SESSION_REQUEST_MESSAGE_HPP
#define SESSION_REQUEST_MESSAGE_HPP
#include "Utils/common.hpp"
#include "MessageBase.hpp"
#include "IReceiveMessage.hpp"
class SessionRequestMessage :
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
    std::string _session_token;
    std::string _user_name;
    std::string _user_mail_addr;
    uint64_t _last_received_message_id;

public:
    //コンストラクタ
    SessionRequestMessage();
    SessionRequestMessage(
        const MessageBase::CommonField& common_fields, 
        SignType s_type, 
        const std::string& session_token, 
        const std::string& user_name, 
        const std::string& user_mail_addr, 
        const uint64_t last_received_message_id);
    //デストラクタ
    ~SessionRequestMessage();

    //各種アクセッサ(getter/setter)
    SignType get_sign_type() const;
	int get_sign_type_int() const;
    void set_sign_type(SignType s_type);
    void set_sign_type(int s_type);

    std::string get_session_token() const;
    void set_session_token(const std::string& session_token);

    std::string get_user_name() const;
    void set_user_name(const std::string& name);

    std::string get_user_mail_addr() const;
    void set_user_mail_addr(const std::string& mail_addr);

    uint64_t get_last_received_message_id() const;
    void set_last_received_message_id(uint64_t last_received_message_id);

    //メッセージインスタンスの内容検証
    bool validate_payload(const json& message) const override;
    std::unique_ptr<MessageBase> create_instance(const json& message) const override;
    bool validate_value() const override;

private:
    bool is_valid_session_token(const std::string& jwt) const;
    bool is_valid_name(const std::string& name) const;
    bool is_valid_mail_addr(const std::string& mail_addr) const;

    SignType exchange_sign_type_from_int(const int value);
};
#endif // !SESSION_REQUEST_MESSAGE_HPP



