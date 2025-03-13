#include "ChatRoomHandler.h"


ChatroomHandler::ChatRoom::~ChatRoom()
{

}

bool ChatroomHandler::ChatRoom::add_client_chatroom(const ClientHandler::Client& client)
{
	try
	{
		this->_client_list.push_back(client);
	}
	catch (const std::bad_alloc&)
	{
		THROW_APP_EXCEPTION("Failed to allocate memory while adding a client to a chat room.");
	}
	catch (const exception::ExceptionBase& ex) {
		spdlog::error(ex.what_for_str());
		return false;
	}
	
	return true;
}

bool ChatroomHandler::ChatRoom::delete_client_chatroom(const ClientHandler::Client& client)
{
	auto element_to_remove = search_client_form_chatroom(client);
	this->_client_list.erase(element_to_remove);
	return true;
}

std::string& ChatroomHandler::ChatRoom::get_chatroom_id()
{
	return this->_chatroom_id;
}

const std::string& ChatroomHandler::ChatRoom::get_chatroom_id() const
{
	return this->_chatroom_id;
}

std::string& ChatroomHandler::ChatRoom::get_chatroom_name()
{
	return this->_chatroom_name;
}

const std::string& ChatroomHandler::ChatRoom::get_chatroom_name() const
{
	return this->_chatroom_name;
}

std::vector<ClientHandler::Client>& ChatroomHandler::ChatRoom::get_client_list()
{
	return this->_client_list;
}

const std::vector<ClientHandler::Client>& ChatroomHandler::ChatRoom::get_client_list() const
{
	return this->_client_list;
}

 std::vector<ClientHandler::Client>::iterator ChatroomHandler::ChatRoom::search_client_form_chatroom(const ClientHandler::Client& client)
{
	 try
	 {
		auto iterator = std::find(this->_client_list.begin(), this->_client_list.end(), client);
		if (iterator == this->_client_list.end()) {
			THROW_APP_EXCEPTION("We couldn't find the client we were looking for!");
		}
		else {
#ifdef DEBUG
			spdlog::debug("Search number: {:c}", *iterator);
#endif // DEBUG
			return iterator;
		}

	 }
	 catch (const exception::ExceptionBase& ex)
	 {
		 spdlog::error(ex.what_for_str());
		return this->_client_list.end();
	 }
}





ChatroomHandler::ChatroomHandler()
{
}

ChatroomHandler::~ChatroomHandler()
{
}

bool ChatroomHandler::add_chatroom(const ChatRoom& chatroom)
{
	return false;
}

bool ChatroomHandler::delete_chatroom(const ChatRoom& chatroom)
{
	return false;
}

std::vector<ClientHandler::Client>& ChatroomHandler::get_client_list(int list_num)
{
	return this->chatroom_list[list_num].get_client_list();
}

std::vector<ChatroomHandler::ChatRoom> ChatroomHandler::get_chatroom_list()
{
	return this->chatroom_list;
}

int ChatroomHandler::search_chatroom_id_from_list(const std::string& chatroom_id)
{
	for (int i = 0; i < this->chatroom_list.size(); i++) {
		if (this->chatroom_list[i].get_chatroom_id() == chatroom_id) {
			return i;
		}
	}
	return -1;
}


std::string ChatroomHandler::issue_chatroom_id()
{
	return std::string();
}

