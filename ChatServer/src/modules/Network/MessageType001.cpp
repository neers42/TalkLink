#include "MessageType001.h"

MessageType001::MessageType001(SignType s_type, AuthType a_type, const std::string& name, const std::string& mail_addr, const std::string& password)
    :_sign_type{s_type},
    _auth_type{a_type},
    _client_name{name},
    _client_mail_addr{mail_addr},
    _client_password{password}
{
}
MessageType001::MessageType001(int s_type, int a_type, const std::string& name, const std::string& mail_addr, const std::string& password)
    :_sign_type{exchange_sign_type_from_int(s_type)},
    _auth_type{exchange_auth_type_from_int(a_type)},
    _client_name{name},
    _client_mail_addr{mail_addr},
    _client_password{password}
{
}
MessageType001::MessageType001(ContentType c_type, const std::string& mt, SignType s_type, AuthType a_type, const std::string& name, const std::string& mail_addr, const std::string& password)
    :BaseMessage(c_type, mt),
    _sign_type{ s_type },
    _auth_type{ a_type },
    _client_name{ name },
    _client_mail_addr{ mail_addr },
    _client_password{ password }
{
}
MessageType001::MessageType001(int c_type, const std::string& mt, int s_type, int a_type, const std::string& name, const std::string& mail_addr, const std::string& password)
    :BaseMessage(c_type, mt),
    _sign_type{ this->exchange_sign_type_from_int(s_type)},
    _auth_type{exchange_auth_type_from_int(a_type)},
    _client_name{name}, _client_mail_addr{mail_addr},
    _client_password{password}
{
}
MessageType001::~MessageType001()
{
}


MessageType001::SignType MessageType001::get_sign_type() const
{
    return this->_sign_type;
}

int MessageType001::get_sign_type_int() const
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

void MessageType001::set_sign_type(SignType s_type)
{
    this->_sign_type = s_type;
}

void MessageType001::set_sign_type(int s_type)
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

MessageType001::AuthType MessageType001::get_auth_type() const
{
    return this->_auth_type;
}

void MessageType001::set_auth_type(AuthType a_type)
{
    this->_auth_type = a_type;
}

void MessageType001::set_auth_type(int a_type)
{
    switch (a_type)
    {
    case 0:
        this->_auth_type = AuthType::NORMAL;
        break;
    case 1:
        this->_auth_type = AuthType::GOOGLE;
        break;
    case 2:
        this->_auth_type = AuthType::FACEBOOK;
        break;
    case 3:
        this->_auth_type = AuthType::TWITTER;
        break;
    case 4:
        this->_auth_type = AuthType::MICROSOFT;
        break;
    case 5:
        this->_auth_type = AuthType::YAHOO;
    default:
        break;
    }
}

std::string MessageType001::get_client_name() const
{
    return this->_client_name;
}

void MessageType001::set_client_name(const std::string& name)
{
    this->_client_name = name;
}

std::string MessageType001::get_client_mail_addr() const
{
    return this->_client_mail_addr;
}

void MessageType001::set_clilent_mail_addr(const std::string& mail_addr)
{
    this->_client_mail_addr = mail_addr;
}

std::string MessageType001::get_client_password() const
{
    return this->_client_password;
}

void MessageType001::set_client_password(const std::string& password)
{
    this->_client_password = password;
}

bool MessageType001::is_valid()
{
    if (is_valid_name(this->_client_name) == true &&
        is_valid_mail_addr(this->_client_mail_addr) == true &&
        is_valid_password(this->_client_password) == true) {
        return true;
    }
    else
    {
        return false;
    }
}

std::unique_ptr<MessageType001> MessageType001::serialize_message(const json& json_message)
{
    try {
        // 1. 必須キーがすべて存在するかチェック
        static const std::vector<std::string> required_keys = {
            "content_type", "message_type", "sign_type",
            "auth_type", "name", "mail_address", "password"
        };

        for (const auto& key : required_keys) {
            if (!json_message.contains(key)) {
                throw std::runtime_error("Missing key: " + key);
            }
        }

        // 2. 各フィールドの型を検証しながら読み取る
        if (!json_message["content_type"].is_string()) {
            throw std::runtime_error("Invalid type for 'content_type'");
        }
        if (!json_message["message_type"].is_string()) {
            throw std::runtime_error("Invalid type for 'message_type'");
        }
        if (!json_message["sign_type"].is_string()) {
			throw std::runtime_error("Invalid type for 'sign_type'");
        }
        if (!json_message["auth_type"].is_string()) {
            throw std::runtime_error("Invalid type for 'auth_type'");
        }
		if (!json_message["name"].is_string()) {
			throw std::runtime_error("Invalid type for 'name'");
		}
		if (!json_message["mail_address"].is_string()) {
			throw std::runtime_error("Invalid type for 'mail_address'");
		}
		if (!json_message["password"].is_string()) {
			throw std::runtime_error("Invalid type for 'password'");
		}
		// 3. 各フィールドを読み取る
        std::string content_type_str = json_message["content_type"];
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
        std::string message_type = json_message["message_type"];
        std::string sign_type_str = json_message["sign_type"];
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
        std::string auth_type_str = json_message["auth_type"];
		int auth_type;
		if (auth_type_str == "NORMAL") {
			auth_type = 0;
		}
		else if (auth_type_str == "GOOGLE") {
			auth_type = 1;
		}
		else if (auth_type_str == "FACEBOOK") {
			auth_type = 2;
		}
		else if (auth_type_str == "TWITTER") {
			auth_type = 3;
		}
		else if (auth_type_str == "MICROSOFT") {
			auth_type = 4;
		}
		else if (auth_type_str == "YAHOO") {
			auth_type = 5;
		}
		else {
			throw std::runtime_error("Invalid auth type");
		}
        std::string client_name = json_message["name"];
        std::string client_mail_addr = json_message["mail_address"];
        std::string client_password = json_message["password"];

        return std::make_unique<MessageType001>(
            content_type,
            message_type,
            sign_type,
            auth_type,
            client_name,
            client_mail_addr,
            client_password
        );
    }
    catch (const std::exception& e) {
		LOG_ERROR("Error deserializing MessageType001: {}", e.what());
        return nullptr;
    }

}

bool MessageType001::is_valid_name(const std::string& name)
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

bool MessageType001::is_valid_mail_addr(const std::string& mail_addr)
{
    //Client mail addressが0byteより大きい場合
    if (mail_addr.size() > 0) {
        return true;
    }
    else {
		LOG_ERROR("Client Mail Address is invalid by byte size error(mailAddress byte size : {:d}).", mail_addr.size());
        return false;
    }
    return false;
}

bool MessageType001::is_valid_password(const std::string& password)
{
    //Client nameが0byteより大きい場合
    if (password.size() > 0) {
        return true;
    }
    else {
		LOG_ERROR("Password is invalid by byte size error(password byte size : {:d}).", password.size());
        return false;
    }
}

MessageType001::SignType MessageType001::exchange_sign_type_from_int(const int value)
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
        break;
    }
}

MessageType001::AuthType MessageType001::exchange_auth_type_from_int(const int value)
{
    switch (value)
    {
    case 0:
        return AuthType::NORMAL;
        break;
    case 1:
        return AuthType::GOOGLE;
        break;
    case 2:
        return AuthType::FACEBOOK;
        break;
    case 3:
        return AuthType::TWITTER;
        break;
    case 4:
        return AuthType::MICROSOFT;
        break;
    case 5:
        return AuthType::YAHOO;
        break;

    default:
        break;
    }
}


