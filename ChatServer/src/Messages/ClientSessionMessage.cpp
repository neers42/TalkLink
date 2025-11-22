#include "Messages/ClientSessionMessage.hpp"

ClientSessionMessage::ClientSessionMessage(){}
ClientSessionMessage::ClientSessionMessage(SignType s_type, const std::string &jwt, const std::string &name, const std::string &mail_addr, const uint64_t last_received_message_id)    
    : _sign_type{s_type},
      _jwt{jwt},
      _client_name{name},
      _client_mail_addr{mail_addr},
      _last_received_message_id{last_received_message_id}
{
}

ClientSessionMessage::ClientSessionMessage(int s_type, const std::string& jwt, const std::string& name, const std::string& mail_addr, const uint64_t last_received_message_id)
    :_sign_type{exchange_sign_type_from_int(s_type)},
    _jwt{jwt},
    _client_name{name},
    _client_mail_addr{mail_addr},
    _last_received_message_id{last_received_message_id}
{
}
ClientSessionMessage::ClientSessionMessage(ContentType c_type, const std::string& mt, SignType s_type, const std::string& jwt, const std::string& name, const std::string& mail_addr, const uint64_t last_received_message_id)
    :MessageBase(c_type, mt),
    _sign_type{ s_type },
    _jwt{ jwt },
    _client_name{ name },
    _client_mail_addr{ mail_addr },
    _last_received_message_id{ last_received_message_id }
{
}
ClientSessionMessage::ClientSessionMessage(int c_type, const std::string& mt, int s_type, const std::string& jwt, const std::string& name, const std::string& mail_addr, const uint64_t last_received_message_id)
    :MessageBase(c_type, mt),
    _sign_type{ this->exchange_sign_type_from_int(s_type)},
    _jwt{ jwt },
    _client_name{name},
    _client_mail_addr{mail_addr},
    _last_received_message_id{last_received_message_id}
{
}

ClientSessionMessage::~ClientSessionMessage()
{
}

ClientSessionMessage::SignType ClientSessionMessage::get_sign_type() const
{
    return this->_sign_type;
}

int ClientSessionMessage::get_sign_type_int() const
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

void ClientSessionMessage::set_sign_type(SignType s_type)
{
    this->_sign_type = s_type;
}

void ClientSessionMessage::set_sign_type(int s_type)
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

std::string ClientSessionMessage::get_jwt() const
{
    return this->_jwt;
}
void ClientSessionMessage::set_jwt(const std::string &jwt)
{
    this->_jwt = jwt;
}

std::string ClientSessionMessage::get_client_name() const
{
    return this->_client_name;
}

void ClientSessionMessage::set_client_name(const std::string& name)
{
    this->_client_name = name;
}

std::string ClientSessionMessage::get_client_mail_addr() const
{
    return this->_client_mail_addr;
}

void ClientSessionMessage::set_client_mail_addr(const std::string& mail_addr)
{
    this->_client_mail_addr = mail_addr;
}

uint64_t ClientSessionMessage::get_last_received_message_id() const
{
    return this->_last_received_message_id;
}
void ClientSessionMessage::set_last_received_message_id(uint64_t last_received_message_id)
{
    this->_last_received_message_id = last_received_message_id;
}
bool ClientSessionMessage::validate_specific_fields(const json& message) const
{
    try
    {
        // 1. 必須キーがすべて存在するかチェック
        static const std::vector<std::string> required_keys = {
            "sign_type",
            "jwt",
            "name", 
            "mail_address", 
            "last_received_message_id"
        };

        for (const auto& key : required_keys) {
            if (!message.contains(key)) {
                throw std::runtime_error("Missing key: " + key);
            }
        }

        if (!message["sign_type"].is_string()) {
			throw std::runtime_error("Invalid type for 'sign_type'");
        }
        if (!message["jwt"].is_string()) {
            throw std::runtime_error("Invalid type for 'jwt'");
        }
		if (!message["name"].is_string()) {
			throw std::runtime_error("Invalid type for 'name'");
		}
		if (!message["mail_address"].is_string()) {
			throw std::runtime_error("Invalid type for 'mail_address'");
		}
        if (!message["last_received_message_id"].is_number_unsigned()) {
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

std::unique_ptr<MessageBase> ClientSessionMessage::create_instance(const json &message) const
{
    try
    {
		// 各フィールドを読み取る
        std::string content_type_str = message["content_type"];
		int content_type;
		if (content_type_str == "TEXT") {
			content_type = 0;
		}
		else if (content_type_str == "IMAGE") {
			content_type = 1;
		}
		else if (content_type_str == "VIDEO") {
			content_type = 2;
		}
		else if (content_type_str == "NOTIFICATION") {
			content_type = 3;
		}
		else {
			throw std::runtime_error("Invalid content type");
		}
        std::string message_type = message["message_type"];
        std::string sign_type_str = message["sign_type"];
		int sign_type;
		if (sign_type_str == "SIGN UP") {
			sign_type = 0;
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
        std::string jwt = message["jwt"];
        std::string client_name = message["name"];
        std::string client_mail_addr = message["mail_address"];
        uint64_t last_received_message_id = message["last_received_message_id"];

        return std::make_unique<ClientSessionMessage>(
            content_type,
            message_type,
            sign_type,
            jwt,
            client_name,
            client_mail_addr,
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
        LOG_ERROR("Error creating ClientSessionMessage instance: {}", e.what());
        return std::unique_ptr<MessageBase>();
    }
    
    
}


bool ClientSessionMessage::validate_value() const
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

bool ClientSessionMessage::is_valid_jwt(const std::string &jwt) const
{
    if (jwt.size() > 0) {
        return true;
    }
    else {
        LOG_ERROR("JWT is invalid by byte size error(jwt byte size : {:d}).", jwt.size());
        return false;
    }
}

bool ClientSessionMessage::is_valid_name(const std::string &name) const
{
    //Client nameが0byteより大きい場合
    if (name.size() > 0) {
        return true;
    }
    else {
		LOG_ERROR("Client name is invalid by byte size error(name byte size : {:d}).", name.size());
        return false;
    }
}

bool ClientSessionMessage::is_valid_mail_addr(const std::string& mail_addr) const
{
    //Client mail addressが0byteより大きい場合
    if (mail_addr.size() > 0) {
        return true;
    }
    else {
		LOG_ERROR("Client Mail Address is invalid by byte size error(mailAddress byte size : {:d}).", mail_addr.size());
        return false;
    }

}
 

ClientSessionMessage::SignType ClientSessionMessage::exchange_sign_type_from_int(const int value)
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



