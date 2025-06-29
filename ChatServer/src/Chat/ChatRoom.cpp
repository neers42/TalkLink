#include "Chat/ChatRoom.h"
ChatRoom::ChatRoom()
	:_chatroom_id{ "" },
	_chatroom_name{ "" },
	_chatroom_icon_url{ "" }
{
}

ChatRoom::ChatRoom(
	const std::string& chatroom_id,
	const std::string& chatroom_name,
	const std::string& icon_url,
	const std::vector<std::string>& cids
)	:
	_chatroom_id{ chatroom_id },
	_chatroom_name{ chatroom_name },
	_chatroom_icon_url{ icon_url },
	_cid_list{ cids }
{
}


ChatRoom::~ChatRoom()
{

}

bool ChatRoom::add_client_chatroom(const std::string& cid)
{
	if (cid.empty()) {
		LOG_ERROR("The client ID cannot be empty.");
		return false;
	}
	std::unique_lock<std::mutex> lock(this->mtx);

	try
	{
		this->_cid_list.push_back(cid);
	}
	catch (const std::bad_alloc&)
	{
		LOG_ERROR("Failed to allocate memory while adding a client to a chat room.");
		return false;
	}
	return true;
}

bool ChatRoom::delete_client_chatroom(const std::string& cid)
{
	auto element_to_remove = search_client_from_chatroom(cid);
	if (element_to_remove == this->_cid_list.end()) {
		LOG_ERROR("The client ID({}) you tried to delete could not be found.", cid);
		return false;
	}
	{
		std::unique_lock<std::mutex> lock(this->mtx);
		this->_cid_list.erase(element_to_remove);
	}
	
	return true;
}


std::string& ChatRoom::get_chatroom_id()
{
	return this->_chatroom_id;
}

const std::string& ChatRoom::get_chatroom_id() const
{
	return this->_chatroom_id;
}

std::string& ChatRoom::get_chatroom_name()
{
	return this->_chatroom_name;
}

const std::string& ChatRoom::get_chatroom_name() const
{
	return this->_chatroom_name;
}

std::string& ChatRoom::get_chatroom_icon()
{
	return this->_chatroom_icon_url;
}

const std::string& ChatRoom::get_chatroom_icon() const
{
	return this->_chatroom_icon_url;
}

std::vector<std::string>& ChatRoom::get_client_list()
{
	return this->_cid_list;
}

const std::vector<std::string>& ChatRoom::get_client_list() const
{
	return this->_cid_list;
}

void ChatRoom::set_chatroom_name(const std::string& chatroom_name)
{
	std::unique_lock<std::mutex> lock(this->mtx);
	this->_chatroom_name = chatroom_name;
}

void ChatRoom::set_chatroom_icon(const std::string& icon_url)
{
	std::unique_lock<std::mutex> lock(this->mtx);
	this->_chatroom_icon_url = icon_url;
}

std::vector<std::string>::iterator ChatRoom::search_client_from_chatroom(const std::string& cid)
{
	try
	{
		auto iterator = std::find(this->_cid_list.begin(), this->_cid_list.end(), cid);
		if (iterator == this->_cid_list.end()) {
			throw std::runtime_error("Client ID not found in chat room.");
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
		return this->_cid_list.end();
	}
}
