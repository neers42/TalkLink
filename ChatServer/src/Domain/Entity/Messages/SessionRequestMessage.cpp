#include "Domain/Entity/Messages/SessionRequestMessage.hpp"

SessionRequestMessage::SessionRequestMessage()
    : MessageBase(MessageBase::MessageType::SESSION_REQUEST),
      _sign_type{SignType::SIGNIN},
      _session_token{""},
      _user_name{""},
      _user_mail_addr{""},
      _last_received_message_id{0}
{
}
SessionRequestMessage::SessionRequestMessage(
    const MessageBase::CommonField& common_fields, 
    SignType s_type, 
    const std::string &session_token, 
    const std::string &name, 
    const std::string &mail_addr, 
    const uint64_t last_received_message_id)    
    : MessageBase(common_fields, MessageBase::MessageType::SESSION_REQUEST), 
      _sign_type{s_type},
      _session_token{session_token},
      _user_name{name},
      _user_mail_addr{mail_addr},
      _last_received_message_id{last_received_message_id}
{
}

SessionRequestMessage::~SessionRequestMessage()
{
}

SessionRequestMessage::SignType SessionRequestMessage::get_sign_type() const
{
    return this->_sign_type;
}

int SessionRequestMessage::get_sign_type_int() const
{
    switch (this->_sign_type)
    {
	case SignType::SIGNUP:
		return 0;
		break;
	case SignType::SIGNIN:
		return 1;
		break;
	case SignType::SIGNOUT:
		return 2;
		break;

    default:
        return -1;
        break;
    }
}

void SessionRequestMessage::set_sign_type(SignType s_type)
{
    this->_sign_type = s_type;
}

void SessionRequestMessage::set_sign_type(int s_type)
{
    switch (s_type)
    {
    case 0:
        this->_sign_type = SignType::SIGNUP;
        break;
    case 1:
        this->_sign_type = SignType::SIGNIN;
        break;
    case 2:
        this->_sign_type = SignType::SIGNOUT;
        break;
    default:
        break;
    }
}

std::string SessionRequestMessage::get_session_token() const
{
    return this->_session_token;
}
void SessionRequestMessage::set_session_token(const std::string &session_token)
{
    this->_session_token = session_token;
}

std::string SessionRequestMessage::get_user_name() const
{
    return this->_user_name;
}

void SessionRequestMessage::set_user_name(const std::string& name)
{
    this->_user_name = name;
}

std::string SessionRequestMessage::get_user_mail_addr() const
{
    return this->_user_mail_addr;
}

void SessionRequestMessage::set_user_mail_addr(const std::string& mail_addr)
{
    this->_user_mail_addr = mail_addr;
}

uint64_t SessionRequestMessage::get_last_received_message_id() const
{
    return this->_last_received_message_id;
}
void SessionRequestMessage::set_last_received_message_id(uint64_t last_received_message_id)
{
    this->_last_received_message_id = last_received_message_id;
}
bool SessionRequestMessage::validate_payload(const json& message) const
{
    try
    {
        // payloadキーの存在をチェック
		if (!message.contains("payload"))
		{
			throw std::runtime_error("Missing key: payload");
		}

		const auto& payload = message["payload"];

        // 基本的な必須キー
        std::vector<std::string> required_keys = {
            "sign_type",
            "session_token", 
            "last_received_message_id"
        };

        // sign_typeをチェックして、必要に応じてnameを追加
        if (!payload.contains("sign_type")) {
            throw std::runtime_error("Missing key: sign_type");
        }
        
        std::string sign_type_str = payload["sign_type"];
        if (sign_type_str == "SIGN UP") {
            // SIGN UPの場合のみnameを必須とする
            required_keys.push_back("name");
            required_keys.push_back("mail_address");
        }

        // 1. 必須キーがすべて存在するかチェック
        for (const auto& key : required_keys) {
            if (!payload.contains(key)) {
                throw std::runtime_error("Missing key: " + key);
            }
        }

        if (!payload["sign_type"].is_string()) {
			throw std::runtime_error("Invalid type for 'sign_type'");
        }
        if (!payload["session_token"].is_string()) {
            throw std::runtime_error("Invalid type for 'session_token'");
        }
        
        // nameフィールドが存在する場合のみ型チェック
        if (payload.contains("name") && !payload["name"].is_string()) {
            throw std::runtime_error("Invalid type for 'name'");
        }
        
		if (payload.contains("mail_address") && !payload["mail_address"].is_string()) {
			throw std::runtime_error("Invalid type for 'mail_address'");
		}
        if (!payload["last_received_message_id"].is_number_unsigned()) {
            throw std::runtime_error("Invalid type for 'last_received_message_id'");
        }
    }
    catch(const std::runtime_error& re)
    {
        LOG_ERROR("Runtime error : {}", re.what());
        return false;
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("Key check failed: {}", e.what());
        return false;
    }

    return true;
}

std::unique_ptr<MessageBase> SessionRequestMessage::create_instance(const json &message) const
{
    try
    {
		// 各フィールドを読み取る
        std::string session_token = message["payload"]["session_token"];
        uint64_t last_received_message_id = message["payload"]["last_received_message_id"];
        std::string sign_type_str = message["payload"]["sign_type"];
		std::string user_name = "";
		std::string user_mail_addr = "";

		// SIGN IN/OUTでもクライアントが任意で送ってきた場合は保持する
		if (message["payload"].contains("name") && message["payload"]["name"].is_string()) {
			user_name = message["payload"]["name"];
		}
		if (message["payload"].contains("mail_address") && message["payload"]["mail_address"].is_string()) {
			user_mail_addr = message["payload"]["mail_address"];
		}
		int sign_type;
		if (sign_type_str == "SIGN UP") {
			sign_type = 0;
            user_name = message["payload"]["name"];
            user_mail_addr = message["payload"]["mail_address"];

            return std::make_unique<SessionRequestMessage>(
            this->parse_common_fields(message),
            static_cast<SignType>(sign_type),
            session_token,
            user_name,
            user_mail_addr,
            last_received_message_id
            );
		}
		else if (sign_type_str == "SIGN IN") {
			sign_type = 1;
            
		}
		else if (sign_type_str == "SIGN OUT") {
			sign_type = 2;
		}
		else {
			throw std::runtime_error("Invalid sign type");
		}
        return std::make_unique<SessionRequestMessage>(
            this->parse_common_fields(message),
            static_cast<SignType>(sign_type),
            session_token,
            user_name,
            user_mail_addr,
            last_received_message_id
        );
    }
    catch(const std::runtime_error& re)
    {
        LOG_ERROR("Runtime error : {}", re.what());
        return std::unique_ptr<MessageBase>();
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("Error creating SessionRequestMessage instance: {}", e.what());
        return std::unique_ptr<MessageBase>();
    }
    
    return std::make_unique<SessionRequestMessage>();
}


bool SessionRequestMessage::validate_value() const
{
    if (is_valid_session_token(this->_session_token) == true &&
        is_valid_name(this->_user_name) == true &&
        is_valid_mail_addr(this->_user_mail_addr) == true) {
        return true;
    }
    else
    {
        return false;
    }
}

bool SessionRequestMessage::is_valid_session_token(const std::string &session_token) const
{
    if(this->get_sign_type() == SignType::SIGNOUT){
        // SIGN OUTの場合はsession_tokenのバリデーションをスキップ
        return true;
    }
    if (session_token.size() > 0) {
        return true;
    }
    else {
        LOG_ERROR("Session Token is invalid by byte size error(session token byte size : {:d}).", session_token.size());
        return false;
    }
}

bool SessionRequestMessage::is_valid_name(const std::string &name) const
{
    if(this->get_sign_type() == SignType::SIGNIN || this->get_sign_type() == SignType::SIGNOUT  ){
        // SIGN OUTの場合は名前のバリデーションをスキップ
        return true;
    }
    //User nameが0byteより大きい場合
    if (name.size() > 0) {
        return true;
    }
    else {
		LOG_ERROR("User name is invalid by byte size error(name byte size : {:d}).", name.size());
        return false;
    }
}

bool SessionRequestMessage::is_valid_mail_addr(const std::string& mail_addr) const
{
    if(this->get_sign_type() == SignType::SIGNIN || this->get_sign_type() == SignType::SIGNOUT  ){
        // SIGN OUTの場合はメールアドレスのバリデーションをスキップ
        return true;
    }
    //User mail addressが0byteより大きい場合
    if (mail_addr.size() > 0) {
        return true;
    }
    else {
		LOG_ERROR("User Mail Address is invalid by byte size error(mailAddress byte size : {:d}).", mail_addr.size());
        return false;
    }

}
 

SessionRequestMessage::SignType SessionRequestMessage::exchange_sign_type_from_int(const int value)
{
    switch (value)
    {
    case 0:
        return SignType::SIGNUP;
        break;
    case 1:
        return SignType::SIGNIN;
        break;
    case 2:
        return SignType::SIGNOUT;
        break;
    default:
        return SignType::SIGNUP;
        break;
    }
}



