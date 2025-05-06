#include "ChatRoomHandler.h"


ChatroomHandler::ChatRoom::ChatRoom()
	:_chatroom_id{ "" },
	_chatroom_name{ "" },
	_chatroom_icon_url{""}
{
}

ChatroomHandler::ChatRoom::ChatRoom(
	const std::string& chatroom_name,
	const std::string& icon_url,
	const std::vector<std::string>& cids
	
)
	: 
	_chatroom_id{ChatroomHandler::issue_chatroom_id_unhex()},
	_chatroom_name{ chatroom_name },
	_chatroom_icon_url{ icon_url },
	_cid_list{ cids }
{
}


ChatroomHandler::ChatRoom::~ChatRoom()
{

}

bool ChatroomHandler::ChatRoom::add_client_chatroom(const std::string& cid)
{
	try
	{
		this->_cid_list.push_back(cid);
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

bool ChatroomHandler::ChatRoom::delete_client_chatroom(const std::string& cid)
{
	auto element_to_remove = search_client_form_chatroom(cid);
	if (element_to_remove == this->_cid_list.end()) {
		LOG_ERROR("The client ID({}) you tried to delete could not be found.", cid);
		return false;
	}
	this->_cid_list.erase(element_to_remove);
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

std::string& ChatroomHandler::ChatRoom::get_chatroom_icon()
{
	return this->_chatroom_icon_url;
}

const std::string& ChatroomHandler::ChatRoom::get_chatroom_icon() const
{
	return this->_chatroom_icon_url;
}

std::vector<std::string>& ChatroomHandler::ChatRoom::get_client_list()
{
	return this->_cid_list;
}

const std::vector<std::string>& ChatroomHandler::ChatRoom::get_client_list() const
{
	return this->_cid_list;
}

void ChatroomHandler::ChatRoom::set_chatroom_name(const std::string& chatroom_name)
{
	this->_chatroom_name = chatroom_name;
}

void ChatroomHandler::ChatRoom::set_chatroom_icon(const std::string& icon_url)
{
	this->_chatroom_icon_url = icon_url;
}

 std::vector<std::string>::iterator ChatroomHandler::ChatRoom::search_client_form_chatroom(const std::string& cid)
{
	 try
	 {
		auto iterator = std::find(this->_cid_list.begin(), this->_cid_list.end(), cid);
		if (iterator == this->_cid_list.end()) {
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
		return this->_cid_list.end();
	 }
}





ChatroomHandler::ChatroomHandler()
{
}

ChatroomHandler::~ChatroomHandler()
{
}


void ChatroomHandler::add_chatroom(std::unique_ptr<ChatRoom> chatroom)
{
	if (chatroom->get_chatroom_name() == "" || chatroom->get_client_list().size() == 0) {
		LOG_ERROR("Chatroom name or client list is empty.");
		return;
	}
	if (this->chatroom_list.size() == MAX_ACTIVE_CHATROOM_NUM) {
		this->chatroom_list.erase(this->chatroom_list.begin());
	}

	this->chatroom_list.push_back(std::move(chatroom));
}

void ChatroomHandler::delete_chatroom(const std::string& room_id)
{
	auto it = find_chatroom_id_from_list(room_id);
	if (it == this->chatroom_list.end()) {
		LOG_ERROR("The chatroom ID({}) you tried to delete could not be found.", room_id);
		return;
	}
	this->chatroom_list.erase(it);
	spdlog::info("Chatroom ID:{} deleted from active room list successfully.", room_id);
}

bool ChatroomHandler::add_member_to_chatroom(const std::string& room_id, const std::vector<std::string>& cids)
{
	auto it = find_chatroom_id_from_list(room_id);
	if (it == this->chatroom_list.end()) {
		LOG_ERROR("The chatroom ID({}) you tried to add a member to could not be found.", room_id);
		return false;
	}
	for (const auto& cid : cids) {
		if ((*it)->get_client_list().size() == MAX_CLIENT_NUM_IN_CHATROOM) {
			LOG_ERROR("The chatroom ID({}) has reached the maximum number of clients.", room_id);
			return false;
		}
		if (!(*it)->add_client_chatroom(cid)) {
			LOG_ERROR("Failed to add client ID({}) to chatroom ID({}).", cid, room_id);
			return false;
		}
	}
	spdlog::info("{}Clients added to chatroom ID({}) successfully.", cids.size(), room_id);
	return true;
}

bool ChatroomHandler::delete_member_from_chatroom(const std::string& room_id, const std::vector<std::string>& cids)
{
	auto it = find_chatroom_id_from_list(room_id);
	if (it == this->chatroom_list.end()) {
		LOG_ERROR("The chatroom ID({}) you tried to add a member to could not be found.", room_id);
		return false;
	}
	for (const auto& cid : cids) {
		if ((*it)->get_client_list().size() == 0) {
			LOG_ERROR("The chatroom ID({}) has no clients.", room_id);
			return false;
		}
		if (!(*it)->delete_client_chatroom(cid)) {
			LOG_ERROR("Failed to delete client ID({}) from chatroom ID({}).", cid, room_id);
			return false;
		}
	}
	return true;
}

bool ChatroomHandler::change_room_name(const std::string& room_id, const std::string& room_name)
{
	auto it = find_chatroom_id_from_list(room_id);
	if (it == this->chatroom_list.end()) {
		LOG_ERROR("The chatroom ID({}) you tried to add a member to could not be found.", room_id);
		return false;
	}
	if (room_name.empty()) {
		LOG_ERROR("The room name cannot be empty.");
		return false;
	}
	(*it)->set_chatroom_name(room_name);
	return true;
}

bool ChatroomHandler::change_room_icon(const std::string& room_id, const std::string& icon_url)
{
	auto it = find_chatroom_id_from_list(room_id);
	if (it == this->chatroom_list.end()) {
		LOG_ERROR("The chatroom ID({}) you tried to add a member to could not be found.", room_id);
		return false;
	}
	if (icon_url.empty()) {
		LOG_ERROR("The room icon URL cannot be empty.");
		return false;
	}
	(*it)->set_chatroom_icon(icon_url);
	return true;
}


std::vector<std::string> ChatroomHandler::get_client_infos_by_chatroom_id(int list_num)
{
	return this->chatroom_list[list_num]->get_client_list();
}

std::vector<std::string> ChatroomHandler::get_client_infos_by_chatroom_id(const std::string& room_id) 
{
	int chatroom_list_num = this->search_chatroom_id_from_list(room_id);
	if (chatroom_list_num != -1) {
		return this->chatroom_list[chatroom_list_num]->get_client_list();
	}
	else {
		return std::vector<std::string>();
	}

}

std::vector<std::unique_ptr<ChatroomHandler::ChatRoom>>& ChatroomHandler::get_chatroom_list()
{
	return this->chatroom_list;
}

int ChatroomHandler::search_chatroom_id_from_list(const std::string& room_id)
{
	for (int i = 0; i < this->chatroom_list.size(); i++) {
		if (this->chatroom_list[i]->get_chatroom_id() == room_id) {
			return i;
		}
	}
	return -1;
}

std::vector<std::unique_ptr<ChatroomHandler::ChatRoom>>::iterator ChatroomHandler::find_chatroom_id_from_list(const std::string& room_id)
{
	return std::find_if(chatroom_list.begin(), chatroom_list.end(),
		[&room_id](const std::unique_ptr<ChatRoom>& room) {
			return room->get_chatroom_id() == room_id;
		});
		
}

std::vector<std::string> ChatroomHandler::get_client_list_from_chatroom(const std::string& chatroom_id)
{
	int chatroom_list_num = this->search_chatroom_id_from_list(chatroom_id);
	if (chatroom_list_num != -1) {
		return this->chatroom_list[chatroom_list_num]->get_client_list();
	}
	else {
		LOG_ERROR("Chatroom id is not found.");
		return std::vector<std::string>();
	}
}

std::unique_ptr<ChatroomHandler::ChatRoom> ChatroomHandler::create_chatroom(
	const std::string& chatroom_name, 
	const std::string& icon_url,
	const std::vector<std::string>& room_menber
)
{
	std::vector<std::string> cids;
	
	// チャットルーム名が空でないか確認
	if (chatroom_name.empty()) {
		LOG_ERROR("Chatroom name cannot be empty.");
		return nullptr;
	}

	// 初期クライアント数が制限を超えていないか確認
	if (room_menber.size() > MAX_CLIENT_NUM_IN_CHATROOM) {
		LOG_ERROR("Cannot create chatroom. Initial client count ({}) exceeds the maximum allowed ({}) in a chatroom.",
			room_menber.size(), MAX_CLIENT_NUM_IN_CHATROOM);
		return nullptr;
	}
	for (const auto& member : room_menber) {
		cids.push_back(member);
	}
	spdlog::info("Chatroom '{}' created successfully with {} initial clients.", chatroom_name, room_menber.size());
	return std::make_unique<ChatRoom>(chatroom_name, icon_url, cids);
}


std::string ChatroomHandler::issue_chatroom_id()
{
	// ランダムバイトジェネレータを作成
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(0, 255);

	// ミリ秒単位の現在時刻
	auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();

	// ランダムバイト生成関数
	auto random_func = [&]() -> uint8_t {
		return static_cast<uint8_t>(dist(gen));
		};

	// ULIDを作成
	ulid::ULID ulid = ulid::Create(now, random_func);

	// 文字列に変換して返す
	return ulid::Marshal(ulid);
}

std::string ChatroomHandler::issue_chatroom_id_unhex()
{
	// ランダムバイトジェネレータを作成
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(0, 255);

	// ミリ秒単位の現在時刻
	auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();

	// ランダムバイト生成関数
	auto random_func = [&]() -> uint8_t {
		return static_cast<uint8_t>(dist(gen));
		};

	// ULIDを作成
	ulid::ULID ulid = ulid::Create(now, random_func);

	// 文字列に変換して返す
	auto ulid_str =  ulid::Marshal(ulid);
	auto decode_ulid = base32::decode(ulid_str);
	std::ostringstream oss;
	for (auto b : decode_ulid) {
		oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
	}
	spdlog::debug("Create Chatroom ID: {}", oss.str());
	return oss.str();
}

