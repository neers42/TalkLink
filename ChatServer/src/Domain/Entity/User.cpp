#include "Domain/Entity/User.hpp"
User::User() 
	: _id{ "" }
	, _name{ "" }
	, _mail_address{ "" }
	, _password{ "" }
	, _heart_beat_count{ 0 }
	, _auth_status{ false }
	, _connection_state{ ConnectState::ACTIVE }
	, _last_activity{ std::time(nullptr) }
{
	// ドメインモデル専用 - インフラ情報は削除
}

User::User(
	const std::string& id, 
	const std::string& name, 
	const std::string& mail_addr
)
	: _id{ id },
	_name{ name },
	_mail_address{ mail_addr },
	_password{ "" },
	_heart_beat_count{ 0 },
	_auth_status{ false },
	_connection_state{ ConnectState::ACTIVE },
	_last_activity{ std::time(nullptr) }
{
	// ドメイン情報のみのコンストラクタ
}

User::User(
	const std::string& id, 
	const std::string& name, 
	const std::string& mail_addr, 
	const std::string& password
)
	: _id{ id },
	_name{ name },
	_mail_address{ mail_addr },
	_password{ password },
	_heart_beat_count{ 0 },
	_auth_status{ false },
	_connection_state{ ConnectState::ACTIVE },
	_last_activity{ std::time(nullptr) }
{
	// パスワード付きドメインコンストラクタ
}

User::~User()
{

}

std::string User::get_id() const
{
	return this->_id;
}

std::string User::get_name() const
{
	return this->_name;
}

std::string User::get_mail_address() const
{
	return this->_mail_address;
}

std::string User::get_password() const
{
	return this->_password;
}


int User::get_heart_beat_count() const
{
	return this->_heart_beat_count.load();
}

bool User::get_auth_status() const
{
	return this->_auth_status.load();
}

User::ConnectState User::get_connect_state() const
{
	return _connection_state.load();
}


void User::set_id(const std::string& id)
{
	this->_id = id;
}

void User::set_name(const std::string& name)
{
	this->_name = name;
}

void User::set_mail_address(const std::string& mail_address)
{
	this->_mail_address = mail_address;
}

void User::set_password(const std::string& password)
{
	this->_password = password;
}


void User::set_heart_beat_count(const int count)
{
	this->_heart_beat_count.store(count);
}

void User::set_auth_status(const bool status)
{
	this->_auth_status.store(status);
}

void User::set_connect_state(ConnectState state)
{
	_connection_state.store(state);
}

void User::increment_heart_beat_count()
{
	this->_heart_beat_count.fetch_add(1);
}

void User::clear_heart_beat_count()
{
	this->_heart_beat_count.store(0);
}

void User::change_auth_status()
{
	bool expected = _auth_status.load();
    _auth_status.store(!expected);
}

bool User::operator==(const User& other_user) const
{
	return this->_id == other_user._id;
}

User& User::operator=(const User& other_user)
{
	_id = other_user._id;
	_name = other_user._name;
	_mail_address = other_user._mail_address;
	_password = other_user._password;
	_heart_beat_count.store(other_user._heart_beat_count.load());
	_auth_status.store(other_user._auth_status.load());
	_connection_state.store(other_user._connection_state.load());
	return *this;
}

bool User::compare_and_set_state(User::ConnectState& expected, User::ConnectState desired) 
{
    return _connection_state.compare_exchange_strong(expected, desired);
}

bool User::can_disconnect() const
{
	// ドメインロジック: 切断可能かの判定
	ConnectState current_state = _connection_state.load();
	return current_state == ConnectState::ACTIVE || current_state == ConnectState::DISCONNECTING;
}

// 新しいメソッドの実装
time_t User::get_last_activity() const
{
	return _last_activity.load();
}

void User::reset_heart_beat_count()
{
	// clear_heart_beat_countのエイリアス
	clear_heart_beat_count();
}

void User::update_last_activity(time_t timestamp)
{
	_last_activity.store(timestamp);
}
