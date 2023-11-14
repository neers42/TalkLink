#include "ChatRoomHandler.h"





ChatroomHandler::ChatRoom::~ChatRoom()
{

}

bool ChatroomHandler::ChatRoom::add_client_chatroom(const ClientHandler::Client& client)
{
	this->client_list.push_back(client);
	return true;
}

bool ChatroomHandler::ChatRoom::delete_client_chatroom(const ClientHandler::Client& client)
{
	auto element_to_remove = search_client_form_chatroom(client);
	this->client_list.erase(element_to_remove);
	return true;
}

 std::vector<ClientHandler::Client>::iterator ChatroomHandler::ChatRoom::search_client_form_chatroom(const ClientHandler::Client& client)
{
	 try
	 {
		auto iterator = std::find(this->client_list.begin(), this->client_list.end(), client);
		if (iterator == this->client_list.end()) {
			THROW_APP_EXCEPTION("We couldn't find the client we were looking for!");
		}
		else {
#ifdef DEBUG
			std::cout << StringFormat("%s[DEBUG]Search number: %c", ServerTime::get_current_time(), *iterator) << std::endl;
#endif // DEBUG
			return iterator;
		}

	 }
	 catch (const exception::ExceptionBase& ex)
	 {
		std::cerr << StringFormat("%s\033[31m[ERROR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str());
		return this->client_list.end();
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

std::string ChatroomHandler::issue_chatroom_id()
{
	return std::string();
}

