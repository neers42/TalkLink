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
		LOG_ERROR(ex.what_for_str());
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
		 LOG_ERROR(ex.what_for_str());
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

std::vector<ClientHandler::Client> ChatroomHandler::get_client_list(const std::string& chatroom_id) 
{
	int chatroom_list_num = this->search_chatroom_id_from_list(chatroom_id);
	if (chatroom_list_num != -1) {
		return this->chatroom_list[chatroom_list_num].get_client_list();
	}
	else {
		return std::vector<ClientHandler::Client>();
	}

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

std::vector<ClientHandler::Client> ChatroomHandler::get_client_list_from_chatroom(const std::string& chatroom_id)
{
	int chatroom_list_num = this->search_chatroom_id_from_list(chatroom_id);
	if (chatroom_list_num != -1) {
		return this->chatroom_list[chatroom_list_num].get_client_list();
	}
	else {
		LOG_ERROR("Chatroom id is not found.");
		return std::vector<ClientHandler::Client>();
	}
}

bool ChatroomHandler::create_chatroom(const std::string& chatroom_name, const std::vector<ClientHandler::Client>& initial_clients)
{
	// チャットルームの最大数を超えていないか確認
	if (chatroom_list.size() >= MAX_ACTIVE_CHATROOM_NUM) {
		LOG_ERROR("Cannot create chatroom. Maximum number of active chatrooms ({}) reached.", MAX_ACTIVE_CHATROOM_NUM);
		return false;
	}

	// チャットルーム名が空でないか確認
	if (chatroom_name.empty()) {
		LOG_ERROR("Chatroom name cannot be empty.");
		return false;
	}

	// 初期クライアント数が制限を超えていないか確認
	if (initial_clients.size() > MAX_CLIENT_NUM_IN_CHATROOM) {
		LOG_ERROR("Cannot create chatroom. Initial client count ({}) exceeds the maximum allowed ({}) in a chatroom.",
			initial_clients.size(), MAX_CLIENT_NUM_IN_CHATROOM);
		return false;
	}

	// 新しいチャットルームを作成
	ChatRoom new_chatroom(chatroom_name, initial_clients);

	// チャットルームをリストに追加
	chatroom_list.push_back(std::move(new_chatroom));
	spdlog::info("Chatroom '{}' created successfully with {} initial clients.", chatroom_name, initial_clients.size());
	return true;
}


std::string ChatroomHandler::issue_chatroom_id()
{
	return std::string();
}

