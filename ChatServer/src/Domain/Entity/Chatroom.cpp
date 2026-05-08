#include "Domain/Entity/Chatroom.hpp"
Chatroom::Chatroom()
	:_chatroom_id{ "" },
	_chatroom_name{ "" },
	_chatroom_icon_url{ "" }
{
}

Chatroom::Chatroom(
	const std::string& chatroom_id,
	const std::string& chatroom_name,
	const std::string& icon_url,
	const std::vector<std::string>& uids
)	:
	_chatroom_id{ chatroom_id },
	_chatroom_name{ chatroom_name },
	_chatroom_icon_url{ icon_url },
	_uid_list{ uids }
{
}


Chatroom::~Chatroom()
{

}

bool Chatroom::add_user_chatroom(const std::string& uid)
{
	if (uid.empty()) {
		LOG_ERROR("The user ID cannot be empty.");
		return false;
	}
	std::unique_lock<std::mutex> lock(this->mtx);

	try
	{
		this->_uid_list.push_back(uid);
	}
	catch (const std::bad_alloc&)
	{
		LOG_ERROR("Failed to allocate memory while adding a user to a chat room.");
		return false;
	}
	return true;
}

bool Chatroom::delete_user_chatroom(const std::string& uid)
{
	auto element_to_remove = search_user_from_chatroom(uid);
	if (element_to_remove == this->_uid_list.end()) {
		LOG_ERROR("The user ID({}) you tried to delete could not be found.", uid);
		return false;
	}
	{
		std::unique_lock<std::mutex> lock(this->mtx);
		this->_uid_list.erase(element_to_remove);
	}
	
	return true;
}


std::string& Chatroom::get_chatroom_id()
{
	return this->_chatroom_id;
}

const std::string& Chatroom::get_chatroom_id() const
{
	return this->_chatroom_id;
}

std::string& Chatroom::get_chatroom_name()
{
	return this->_chatroom_name;
}

const std::string& Chatroom::get_chatroom_name() const
{
	return this->_chatroom_name;
}

std::string& Chatroom::get_chatroom_icon()
{
	return this->_chatroom_icon_url;
}

const std::string& Chatroom::get_chatroom_icon() const
{
	return this->_chatroom_icon_url;
}

std::vector<std::string>& Chatroom::get_users()
{
	return this->_uid_list;
}

const std::vector<std::string>& Chatroom::get_users() const
{
	return this->_uid_list;
}

void Chatroom::set_chatroom_name(const std::string& chatroom_name)
{
	std::unique_lock<std::mutex> lock(this->mtx);
	this->_chatroom_name = chatroom_name;
}

void Chatroom::set_chatroom_icon(const std::string& icon_url)
{
	std::unique_lock<std::mutex> lock(this->mtx);
	this->_chatroom_icon_url = icon_url;
}

std::vector<std::string>::iterator Chatroom::search_user_from_chatroom(const std::string& uid)
{
	try
	{
		auto iterator = std::find(this->_uid_list.begin(), this->_uid_list.end(), uid);
		if (iterator == this->_uid_list.end()) {
			throw std::runtime_error("User ID not found in chat room.");
		}
		else {
#ifdef DEBUG
			spdlog::debug("Search number: {:c}", *iterator);
#endif // DEBUG
			return iterator;
		}

	}
	catch(const std::runtime_error& e)
	{
		LOG_ERROR("Runtime Error : ", e.what());
		return this->_uid_list.end();
	}
}
