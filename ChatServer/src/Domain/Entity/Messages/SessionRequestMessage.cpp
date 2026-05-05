#include "Domain/Entity/Messages/SessionRequestMessage.hpp"

SessionRequestMessage::SessionRequestMessage()
    : MessageBase(MessageBase::MessageType::SESSION_REQUEST),
      _sign_type{SignType::SIGNIN},
      _jwt{""},
      _client_name{""},
      _client_mail_addr{""},
      _last_received_message_id{0}
{
}
SessionRequestMessage::SessionRequestMessage(const MessageBase::CommonField& common_fields,SignType s_type, const std::string &jwt, const std::string &name, const std::string &mail_addr, const uint64_t last_received_message_id)    
    : MessageBase(common_fields, MessageBase::MessageType::SESSION_REQUEST), 
      _sign_type{s_type},
      _jwt{jwt},
      _client_name{name},
      _client_mail_addr{mail_addr},
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

std::string SessionRequestMessage::get_jwt() const
{
    return this->_jwt;
}
void SessionRequestMessage::set_jwt(const std::string &jwt)
{
    this->_jwt = jwt;
}

std::string SessionRequestMessage::get_client_name() const
{
    return this->_client_name;
}

void SessionRequestMessage::set_client_name(const std::string& name)
{
    this->_client_name = name;
}

std::string SessionRequestMessage::get_client_mail_addr() const
{
    return this->_client_mail_addr;
}

void SessionRequestMessage::set_client_mail_addr(const std::string& mail_addr)
{
    this->_client_mail_addr = mail_addr;
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
            "jwt", 
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
        if (!payload["jwt"].is_string()) {
            throw std::runtime_error("Invalid type for 'jwt'");
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
        std::string jwt = message["payload"]["jwt"];
        uint64_t last_received_message_id = message["payload"]["last_received_message_id"];
        std::string sign_type_str = message["payload"]["sign_type"];
		int sign_type;
		if (sign_type_str == "SIGN UP") {
			sign_type = 0;
            std::string client_name = message["payload"]["name"];
            std::string client_mail_addr = message["payload"]["mail_address"];

            return std::make_unique<SessionRequestMessage>(
            this->parse_common_fields(message),
            static_cast<SignType>(sign_type),
            jwt,
            client_name,        //Client nameはSign UPの場合必須
            client_mail_addr,     //Client mail addressはSign UPの場合必須
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
            jwt,
            "",     //Client nameはSign IN/OUTの場合不要
            "",    //Client mail addressはSign IN/OUTの場合不要
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
    if (is_valid_jwt(this->_jwt) == true &&
        is_valid_name(this->_client_name) == true &&
        is_valid_mail_addr(this->_client_mail_addr) == true) {
        return true;
    }
    else
    {
        return false;
    }
}

bool SessionRequestMessage::is_valid_jwt(const std::string &jwt) const
{
    if(this->get_sign_type() == SignType::SIGNOUT){
        // SIGN OUTの場合はJWTのバリデーションをスキップ
        return true;
    }
    if (jwt.size() > 0) {
        return true;
    }
    else {
        LOG_ERROR("JWT is invalid by byte size error(jwt byte size : {:d}).", jwt.size());
        return false;
    }
}

bool SessionRequestMessage::is_valid_name(const std::string &name) const
{
    if(this->get_sign_type() == SignType::SIGNIN || this->get_sign_type() == SignType::SIGNOUT  ){
        // SIGN OUTの場合は名前のバリデーションをスキップ
        return true;
    }
    //Client nameが0byteより大きい場合
    if (name.size() > 0) {
        return true;
    }
    else {
		LOG_ERROR("Client name is invalid by byte size error(name byte size : {:d}).", name.size());
        return false;
    }
}

bool SessionRequestMessage::is_valid_mail_addr(const std::string& mail_addr) const
{
    if(this->get_sign_type() == SignType::SIGNIN || this->get_sign_type() == SignType::SIGNOUT  ){
        // SIGN OUTの場合は名前のバリデーションをスキップ
        return true;
    }
    //Client mail addressが0byteより大きい場合
    if (mail_addr.size() > 0) {
        return true;
    }
    else {
		LOG_ERROR("Client Mail Address is invalid by byte size error(mailAddress byte size : {:d}).", mail_addr.size());
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



