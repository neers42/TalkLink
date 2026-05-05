#include "Domain/Entity/Client.hpp"
Client::Client() 
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

Client::Client(
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

Client::Client(
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

Client::~Client()
{

}

std::string Client::get_id() const
{
	return this->_id;
}

std::string Client::get_name() const
{
	return this->_name;
}

std::string Client::get_mail_address() const
{
	return this->_mail_address;
}

std::string Client::get_password() const
{
	return this->_password;
}


int Client::get_heart_beat_count() const
{
	return this->_heart_beat_count.load();
}

bool Client::get_auth_status() const
{
	return this->_auth_status.load();
}

Client::ConnectState Client::get_connect_state() const
{
	return _connection_state.load();
}


void Client::set_id(const std::string& id)
{
	this->_id = id;
}

void Client::set_name(const std::string& name)
{
	this->_name = name;
}

void Client::set_mail_address(const std::string& mail_address)
{
	this->_mail_address = mail_address;
}

void Client::set_password(const std::string& password)
{
	this->_password = password;
}


void Client::set_heart_beat_count(const int count)
{
	this->_heart_beat_count.store(count);
}

void Client::set_auth_status(const bool status)
{
	this->_auth_status.store(status);
}

void Client::set_connect_state(ConnectState state)
{
	_connection_state.store(state);
}

void Client::increment_heart_beat_count()
{
	this->_heart_beat_count.fetch_add(1);
}

void Client::clear_heart_beat_count()
{
	this->_heart_beat_count.store(0);
}

void Client::change_auth_status()
{
	bool expected = _auth_status.load();
    _auth_status.store(!expected);
}

bool Client::operator==(const Client& other_client) const
{
	return this->_id == other_client._id;
}

Client& Client::operator=(const Client& other_client)
{
	_id = other_client._id;
	_name = other_client._name;
	_mail_address = other_client._mail_address;
	_password = other_client._password;
	_heart_beat_count.store(other_client._heart_beat_count.load());
	_auth_status.store(other_client._auth_status.load());
	_connection_state.store(other_client._connection_state.load());
	return *this;
}

bool Client::compare_and_set_state(Client::ConnectState& expected, Client::ConnectState desired) 
{
    return _connection_state.compare_exchange_strong(expected, desired);
}

bool Client::can_disconnect() const
{
	// ドメインロジック: 切断可能かの判定
	ConnectState current_state = _connection_state.load();
	return current_state == ConnectState::ACTIVE || current_state == ConnectState::DISCONNECTING;
}

// 新しいメソッドの実装
time_t Client::get_last_activity() const
{
	return _last_activity.load();
}

void Client::reset_heart_beat_count()
{
	// clear_heart_beat_countのエイリアス
	clear_heart_beat_count();
}

void Client::update_last_activity(time_t timestamp)
{
	_last_activity.store(timestamp);
}
