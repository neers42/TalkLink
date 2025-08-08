#include "Chat/ChatRoomHandler.hpp"


ChatroomHandler::ChatroomHandler()
{
}

ChatroomHandler::~ChatroomHandler()
{
}


void ChatroomHandler::add_chatroom(std::shared_ptr<ChatRoom> chatroom)
{
	try
	{
		std::unique_lock<std::shared_mutex> lock(this->shared_mtx);
		if (chatroom->get_chatroom_name() == "" || chatroom->get_client_list().size() == 0) {
			LOG_ERROR("Chatroom name or client list is empty.");
			return;
		}
		if (this->chatroom_list.size() == MAX_ACTIVE_CHATROOM_NUM) {
			this->chatroom_list.erase(this->chatroom_list.begin());
		}

		this->chatroom_list.push_back(std::move(chatroom));
	}
	catch (const std::bad_alloc& e)
	{
		LOG_ERROR("Memory allocation failed: ", e.what());
	}
	catch (const std::system_error& e)
	{
		LOG_ERROR("System error: ", e.what());
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR("Runtime error: ", e.what());
	}
	catch (const std::logic_error& e)
	{
		LOG_ERROR("Logic error: ", e.what());
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("exception : ", e.what());
	}
	
}

void ChatroomHandler::delete_chatroom(const std::string& room_id)
{
	std::shared_lock<std::shared_mutex> lock(this->shared_mtx);
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
	std::shared_lock<std::shared_mutex> lock(this->shared_mtx);
	auto it = find_chatroom_id_from_list(room_id);
	if (it == this->chatroom_list.end()) {
		spdlog::warn("The chatroom ID({}) you tried to add a member to could not be found.", room_id);
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
	std::shared_lock<std::shared_mutex> lock(this->shared_mtx);
	auto it = find_chatroom_id_from_list(room_id);
	if (it == this->chatroom_list.end()) {
		spdlog::warn("The chatroom ID({}) you tried to add a member to could not be found.", room_id);
		return false;
	}
	for (const auto& cid : cids) {
		if ((*it)->get_client_list().size() == 0) {
			spdlog::warn("The chatroom ID({}) has no clients.", room_id);
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
	std::shared_lock<std::shared_mutex> lock(this->shared_mtx);
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
	std::shared_lock<std::shared_mutex> lock(this->shared_mtx);
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
	std::shared_lock<std::shared_mutex> lock(this->shared_mtx);
	return this->chatroom_list[list_num]->get_client_list();
}

std::vector<std::string> ChatroomHandler::get_client_infos_by_chatroom_id(const std::string& room_id) 
{
	std::shared_lock<std::shared_mutex> lock(this->shared_mtx);
	int chatroom_list_num = this->search_chatroom_id_from_list(room_id);
	if (chatroom_list_num != -1) {
		return this->chatroom_list[chatroom_list_num]->get_client_list();
	}
	else {
		return std::vector<std::string>();
	}

}

std::vector<std::shared_ptr<ChatRoom>> ChatroomHandler::get_chatrooms_snapshot()
{
	std::shared_lock<std::shared_mutex> lock(this->shared_mtx);
	return this->chatroom_list;
}

int ChatroomHandler::search_chatroom_id_from_list(const std::string& room_id)
{
	std::shared_lock<std::shared_mutex> lock(this->shared_mtx);
	for (int i = 0; i < this->chatroom_list.size(); i++) {
		if (this->chatroom_list[i]->get_chatroom_id() == room_id) {
			return i;
		}
	}
	return -1;
}

std::vector<std::shared_ptr<ChatRoom>>::iterator ChatroomHandler::find_chatroom_id_from_list(const std::string& room_id)
{
	std::shared_lock<std::shared_mutex> lock(this->shared_mtx);
	return std::find_if(chatroom_list.begin(), chatroom_list.end(),
		[&room_id](const std::shared_ptr<ChatRoom>& room) {
			return room->get_chatroom_id() == room_id;
		});
		
}

std::vector<std::string> ChatroomHandler::get_client_list_from_chatroom(const std::string& chatroom_id)
{
	std::shared_lock<std::shared_mutex> lock(this->shared_mtx);
	int chatroom_list_num = this->search_chatroom_id_from_list(chatroom_id);
	if (chatroom_list_num != -1) {
		return this->chatroom_list[chatroom_list_num]->get_client_list();
	}
	else {
		LOG_ERROR("Chatroom id is not found.");
		return std::vector<std::string>();
	}
}

std::shared_ptr<ChatRoom> ChatroomHandler::create_chatroom(
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
	return std::make_unique<ChatRoom>(this->issue_chatroom_id_unhex(), chatroom_name, icon_url, cids);
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

