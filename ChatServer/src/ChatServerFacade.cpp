#include "ChatServerFacade.hpp"

std::time_t ServerTime::current_time;
std::tm ServerTime::time_info;

ChatServerFacade::ChatServerFacade() : _server_ip{""}
{
	// ログレベルの設定
#ifdef DEBUG_MODE
	spdlog::set_level(spdlog::level::debug);
#else
	// ログのレベル設定
	spdlog::set_level(spdlog::level::info);

#endif // DEBUG_MODE
	////ログのフォーマット設定
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread id : %t] [%^%l%$] %v");
	spdlog::info("Starting MyChat server version 1.0.0!!");
	_socket_handler = std::make_unique<SocketHandler>(SOCKET_CONFIG_FILE_PATH);
	_mysql_client = std::make_unique<MySQLClient>();
	_redis_client = std::make_unique<RedisClient>();
	_protocol_handler = std::make_unique<ProtocolHandler>(SERVER_CERT_FILE_PATH, SERVER_PRIVATEKEY_FILE_PATH);
	_server_time = std::make_unique<ServerTime>();
	_client_handler = std::make_unique<ClientHandler>();
	_thread_pool_handler = std::make_unique<ThreadPoolHandler>(10);
	_chat_room_handler = std::make_unique<ChatroomHandler>();

	if (_socket_handler == nullptr ||
		_mysql_client == nullptr ||
		_redis_client == nullptr ||
		_protocol_handler == nullptr ||
		_server_time == nullptr ||
		_client_handler == nullptr ||
		_thread_pool_handler == nullptr ||
		_chat_room_handler == nullptr)
	{
		LOG_ERROR("Failed to initialize ChatServerFacade components. Please check the configuration and try again.");
		exit(1);
	}

	// ClientHandlerに渡す送受信用の関数をセットする。
	_client_handler->set_send_function(
		[this](SSL *ssl, const json &message)
		{
			return _protocol_handler->send_message_json(ssl, message);
		});

	_client_handler->set_receive_function(
		[this](SSL *ssl)
		{
			return _protocol_handler->receive_message_json(ssl);
		});

	_client_handler->set_ssl_shutdown_function(
		[this](SSL *ssl) -> bool
		{
			return _protocol_handler->shutdown_ssl(ssl);
		});
}

ChatServerFacade::~ChatServerFacade()
{
}

// サーバプログラムの実行
void ChatServerFacade::run()
{
	// リッスンソケットの生成
	if (_socket_handler->open_tcp_listen_socket() == false)
	{
		LOG_ERROR("The TCP listening socket for message could not be opened properly, so the program is forcibly terminated. Please set it again.");
		exit(1);
	}

	if (_socket_handler->open_udp_multicast_tx_socket() == false)
	{
		LOG_ERROR("The program terminates because it failed to create a socket for multicast transmission. Please reset the settings.");
		exit(1);
	}

	this->_mysql_client->create_database();
	this->_mysql_client->create_table();
	this->_thread_pool_handler->enqueue_task([this](){ heartbeat_check_process(); });
	this->_thread_pool_handler->enqueue_task([this](){ _protocol_handler->hot_reload_certificates(60, SERVER_CERT_FILE_PATH, SERVER_PRIVATEKEY_FILE_PATH); });
	spdlog::info("Start monitoring requests from clients.");
	// ディスクリプション監視スタート
	while (1)
	{

#ifdef USE_POLL
		_socket_handler->monitor_socket_descriptor([this](socket_t changed_socket)
												   { allocate_process(changed_socket); });
#elif defined(USE_SELECT)
		auto abs_time = std::chrono::system_clock::now() + std::chrono::milliseconds(1000);
		this->_socket_handler->initialize_select_param();
		this->prepare_client_fd_set();
		std::vector<socket_t> changed_client_sockets = this->_client_handler->get_all_connected_sockets();
		_socket_handler->monitor_socket_descriptor(changed_client_sockets,
												   [this](socket_t changed_socket)
												   { allocate_process(changed_socket); });
		std::this_thread::sleep_until(abs_time);
#endif
	}
}

void ChatServerFacade::show_server_running_info()
{
	std::cout << " * Cipher suite : " << this->_protocol_handler->get_cipher_suite(this->_protocol_handler->get_ssl_object()) << std::endl;
	std::cout << StringFormat(" * Server Listening %s : %d", this->_socket_handler->get_server_ip_address(), this->_socket_handler->get_port());
}


const std::string &ChatServerFacade::get_server_ip()
{
	return this->_server_ip;
}

void ChatServerFacade::set_server_ip(const std::string &ip)
{
	this->_server_ip = ip;
}

// ソケットの変化を検知し、処理を振り分ける
void ChatServerFacade::allocate_process(const socket_t socket)
{
	// socketがINVALIDの場合は処理をしない
	if (socket == INVALID_SOCKET_FD)
	{
		return;
	}
	if (socket == this->_socket_handler->get_listen_socket())
	{
		//this->accept_client_socket_process();
		this->_thread_pool_handler->enqueue_task([this]()
												 { accept_client_socket_process(); });
	}
	else
	{
		// 既存クライアントのデータを処理
		this->_thread_pool_handler->enqueue_task([this, socket]()
												 { message_handle_process(socket); });
	}
}

bool ChatServerFacade::accept_client_socket_process()
{
	LOG_TRACE;
	socket_t new_client_socket = INVALID_SOCKET_FD;
	struct sockaddr_in new_client_sockaddr_in = {0};
	socklen_t new_client_socklen_t = 0;
	std::string new_client_ipv4_addr;

	// TCPソケットのアクセプト処理
	std::tie(new_client_socket, new_client_sockaddr_in) = _socket_handler->accept_client_socket(_client_handler->get_init_client().get_socket_address_in_length());
	if (new_client_socket == INVALID_SOCKET_FD)
	{
		LOG_ERROR("The accepted socket for message could not be added to the list because it was an INVALID SOCKET");
		return ACCEPT_FAILED;
	}
	else
	{
		spdlog::info("accept success!! client socket :{}", new_client_socket);
		new_client_ipv4_addr = this->_socket_handler->extraction_client_ip(new_client_sockaddr_in, sizeof(new_client_ipv4_addr));
	}
#ifdef USE_POLL
	// ノンブロッキング設定（fcntl利用例）
	int flags = fcntl(new_client_socket, F_GETFL, 0);
	if (flags == -1)
	{
		LOG_ERROR("fcntl get flags failed");
		// 必要ならclose(new_client_socket); してエラー処理
		return ACCEPT_FAILED;
	}
#endif

	// SSLアクセプト処理
	SSL *new_client_ssl = this->_protocol_handler->ssl_accept(new_client_socket);
	if (new_client_ssl == nullptr)
	{
		LOG_ERROR("client ssl is not accepted(socket:{:d})", new_client_socket);
		SSL_free(new_client_ssl);
		return ACCEPT_FAILED;
	}
	else
	{
		spdlog::info("client ssl is accepted(socket:{:d})", new_client_socket);
		this->_protocol_handler->get_peer_cetificate(new_client_ssl);
	}

	// 新規クライアントのインスタンスを生成
	auto new_client = std::make_unique<Client>(new_client_socket, new_client_sockaddr_in, new_client_socklen_t, new_client_ipv4_addr, new_client_ssl);
	spdlog::info("client ip : {:s} , success, server accept client(socket:{:d}).", new_client->get_ipv4_address(), new_client->get_socket());
	if (_client_handler->add_client(std::move(new_client)) == true)
	{
#ifdef USE_POLL
		//アクセプトしたクライアントの通知を検知できるようにepollの設定を実施
		this->_socket_handler->add_epoll_ctrl(new_client_socket);
#endif
		return ACCEPT_SUCCEEDED;
	}
	else
	{
		return ACCEPT_FAILED;
	}
}


// メッセージの処理を行う
void ChatServerFacade::message_handle_process(const socket_t socket)
{
	std::string mt, cid = "";
	json receive_json;
	bool disconnect_flag = false;
	bool recv_flag = false;
	std::string error_msg;
	bool result;

	try
	{
		if ((_socket_handler->is_socket_closed(socket) == true))
		{
			spdlog::warn("Client closed tcp connection.");
			this->handle_disconnect_process(socket);
			return;
		}

		// メッセージの受信
		std::tie(receive_json, disconnect_flag, recv_flag) = this->_client_handler->client_message_receive(socket);

		//disconnect_flagがfalseの場合、epoll_ctrlにONESHOTを登録する。
		if (disconnect_flag == false)
		{
#ifdef USE_POLL
			this->_socket_handler->reactivate_epoll_ctrl(socket);
#endif
		}
		// クライアントの切断を検知した場合
		else
		{
			spdlog::info("client(ip:{:s}, client id:{:s}) is disconnected, server initialize client info.", _client_handler->get_client_ipv4_address(socket), _client_handler->get_client_id(socket));
			this->handle_disconnect_process(socket);
			return;
		}

		if (receive_json.empty() == true)
		{
			return;
		}

		mt = receive_json["message_type"];
		// mt000 = ハートビートを受信した際の処理
		if (mt == "000")
		{
			auto mt000 = MessageType000::serialize_message(receive_json);
			// this->mt000_process(mt000, client);
			this->mt000_process(mt000, socket);
		}
		// MT001 = Auth認証メッセージの場合
		else if (mt == "001")
		{
			auto mt001 = MessageType001::serialize_message(receive_json);
			if (mt001 == nullptr)
			{
				return;
			}
			this->_protocol_handler->print_received_message_info(0, "", "", _client_handler->get_client_ipv4_address(socket), socket, receive_json);
			auto verify_result = this->mt001_process(mt001, socket, cid);
			this->mt101_process(socket, cid, verify_result, recv_flag);
		}
		// MT002 = Group or Friend追加のメッセージの場合
		else if (mt == "002")
		{
		}
		else if (mt == "003")
		{
			auto mt003 = MessageType003::serialize_message(receive_json);
			if (mt003 == nullptr)
			{
				return;
			}
			this->_protocol_handler->print_received_message_info(mt003->get_message_id(), mt003->get_sender_id(), mt003->get_room_id(), this->_client_handler->get_client_ipv4_address(socket), socket, receive_json);
			this->mt003_process(mt003, &error_msg, &result);
		}
		// MT004 = 通常メッセージの場合
		else if (mt == "004")
		{
			auto mt004 = MessageType004::serialize_message(receive_json);
			if (mt004 == nullptr)
			{
				return;
			}
			this->_protocol_handler->print_received_message_info(mt004->get_message_id(), mt004->get_sender_id(), mt004->get_room_id(), this->_client_handler->get_client_ipv4_address(socket), socket, receive_json);
			this->mt004_process(mt004);
			this->mt104_process(mt004, socket);
		}

		else
		{
			LOG_ERROR("The receive process was interrupted due to a bad message.");
		}
	}
	catch (const std::exception &e)
	{
		LOG_ERROR(e.what());
	}

}

void ChatServerFacade::mt000_process(const std::unique_ptr<MessageType000> &mt000, std::shared_ptr<Client> &client)
{
	try
	{
		if (mt000 == nullptr || mt000->is_valid() == false)
		{
			throw std::invalid_argument("The reception process was interrupted because the contents of MT000 were invalid.");
		}
		if (mt000->get_client_id() == client->get_id())
		{
			spdlog::info("client(ip:{:s}, client id:{:s}) send HeartBeat.", client->get_ipv4_address(), client->get_id());
			this->_client_handler->incliment_heart_beat_count(client);
		}
		else
		{
			spdlog::error("The reception process was interrupted because the contents of MT000 were invalid.");
			return;
		}
	}
	catch (const std::invalid_argument &e)
	{
		LOG_ERROR(e.what());
	}
}

void ChatServerFacade::mt000_process(const std::unique_ptr<MessageType000> &mt000, const socket_t socket)
{
	try
	{
		auto cid = this->_client_handler->get_client_id(socket);
		if (mt000 == nullptr || mt000->is_valid() == false)
		{
			throw std::invalid_argument("The reception process was interrupted because the contents of MT000 were invalid.");
		}
		if (mt000->get_client_id() == cid)
		{
			spdlog::info("client(ip:{:s}, client id:{:s}) send HeartBeat.", this->_client_handler->get_client_ipv4_address(socket), cid);
			this->_client_handler->incliment_heart_beat_count(socket);
		}
		else
		{
			spdlog::error("The reception process was interrupted because the contents of MT000 were invalid.");
			return;
		}
	}
	catch (const std::invalid_argument &e)
	{
		LOG_ERROR(e.what());
	}
}


bool ChatServerFacade::mt001_process(const std::unique_ptr<MessageType001> &mt001, const socket_t socket, std::string& cid)
{
	try
	{
		if (mt001 == nullptr || mt001->is_valid() == false)
		{
			std::runtime_error("The reception process was interrupted because the contents of MT001 were invalid.");
		}

		auto decoded_token = jwt::decode(mt001->get_jwt_token());
		if(decoded_token.has_subject())
		{
			cid = decoded_token.get_subject();
			spdlog::debug("JWT token client id : {}", cid);
		}else{
			LOG_ERROR("JWT token is invalid.");
			return false;
		}
		//Redisからクライアントのsession idを取得する。有効期限を確認する。
		std::string session_key = "session:" + cid;
		if(this->_redis_client->is_client_session_key(session_key) == false)
		{
			LOG_ERROR("Client session key is not found in Redis.");
			return false;
		}else{
			spdlog::info("Client session(cid : {}) key is found in Redis.", cid);
		}


		switch (mt001->get_sign_type_int())
		{
		case 0: // sign up
			// メッセージがsign_upの場合の処理を追記
			if (cid != "" && _client_handler->get_connect_clients_num() != 0)
			{
				// SIGN UPが成功した場合に、クライアント情報を登録する
				this->_client_handler->set_authentication_info(socket, cid, mt001->get_client_name(), mt001->get_client_mail_addr());

				if (this->_mysql_client->insert_client_data(cid, mt001->get_client_name(), mt001->get_client_mail_addr(), "", ServerTime::get_current_time_to_seconds(), "") == true)
				{
					spdlog::info("Client(ip:{:s}, socket num:{:d}) information has been successfully registered in the database.", _client_handler->get_client_ipv4_address(socket), socket);
				}
				else
				{
					LOG_ERROR("Client(ip:{:s}, socket num:{:d}) information has been failed registered in the database.", _client_handler->get_client_ipv4_address(socket), socket);
				}

				// Auth statusを変更する
				this->_client_handler->change_client_auth_status(socket);
				// ハートビートのカウント回数をインクリメントする
				this->_client_handler->incliment_heart_beat_count(socket);
			}

			break;

		case 1: // sign in
			if (cid != "" && _client_handler->get_connect_clients_num() != 0)
			{
				// データベースからユーザ情報を読み取る。
				std::string client_name, client_mail_addr, client_icon_url;
				std::tie(client_name, client_mail_addr, client_icon_url) = this->_mysql_client->get_client_info_fromDB(cid);
				this->_client_handler->set_authentication_info(socket, cid, client_name, client_mail_addr);

				spdlog::debug("client name:{}, mail address:{}, icon_url:{}", client_name, client_mail_addr, client_icon_url);

				// Auth statusを変更する
				this->_client_handler->change_client_auth_status(socket);
				// ハートビートのカウント回数をインクリメントする
				this->_client_handler->incliment_heart_beat_count(socket);
			}

			break;

		case 2: // sign out
			break;

		default:
			throw std::runtime_error("MT001 authentication data region is in violation.");
			break;
		}
	}
	catch (const std::runtime_error &e)
	{
		LOG_ERROR("runtime error: {}", e.what());
		return false;
	}
	catch (const std::exception &e)
	{
		LOG_ERROR("exception: {}", e.what());
		return false;
	}	
	return true;
}

void ChatServerFacade::mt003_process(const std::unique_ptr<MessageType003> &mt003, std::string *error_msg, bool *action_result)
{
	switch (mt003->get_room_action())
	{

	case MessageType003::RoomAction::CREATE_ROOM:
		// ルームの作成
		{
			auto new_room = this->_chat_room_handler->create_chatroom(mt003->get_room_name(), mt003->get_room_image_url(), mt003->get_add_member_list());
			if (new_room == nullptr)
			{
				LOG_ERROR("The chat room was not created.");
				*action_result = false;
				*error_msg = "The chat room was not created.";
				return;
			};

			// チャットルームのDB保存
			*action_result = this->_mysql_client->insert_chatroom(new_room->get_chatroom_id(), new_room->get_chatroom_name(), new_room->get_chatroom_icon(), ServerTime::get_current_time_to_seconds());
			if (*action_result == false)
			{
				LOG_ERROR("The chat room was not saved in the database.");
				*error_msg = "The chat room was not saved in the database.";
				return;
			}
			*action_result = this->_mysql_client->insert_chatroom_member(new_room->get_chatroom_id(), mt003->get_add_member_list(), ServerTime::get_current_time_to_seconds());
			if (*action_result == false)
			{
				LOG_ERROR("The chat room member was not saved in the database.");
				*error_msg = "The chat room member was not saved in the database.";
				return;
			}
			if (*action_result == true)
			{
				spdlog::info("The chat room(ID:{}) was successfully created and saved in the database.", new_room->get_chatroom_id());
				*error_msg = "No Error.";
			}
			else
			{
				*error_msg = "The chat room(ID:{}) was not created.", new_room->get_chatroom_id();
				return;
			}
			break;
			this->_chat_room_handler->add_chatroom(std::move(new_room));
			auto chatrooms = this->_chat_room_handler->get_chatrooms_snapshot();
			for (const auto &room : chatrooms)
			{
				spdlog::debug("room id : {:s}", room->get_chatroom_id());
			}
		}

	case MessageType003::RoomAction::ADD_MEMBER:
		// ルームのメンバー追加
		*action_result = this->_chat_room_handler->add_member_to_chatroom(mt003->get_room_id(), mt003->get_add_member_list());

		// データベースにメンバーを追加
		*action_result = this->_mysql_client->insert_chatroom_member(mt003->get_room_id(), mt003->get_add_member_list(), ServerTime::get_current_time_to_seconds());

		if (*action_result == true)
		{
			spdlog::info("The chat room(ID:{}) was successfully added and saved in the database.", mt003->get_room_id());
			*error_msg = "No Error.";
		}
		else
		{
			*error_msg = "The chat room(ID:{}) was not created.", mt003->get_room_id();
			return;
		}

		break;

	case MessageType003::RoomAction::REMOVE_MEMBER:
		// ルームのメンバー削除
		*action_result = this->_chat_room_handler->delete_member_from_chatroom(mt003->get_room_id(), mt003->get_remove_member_list());

		// データベースからメンバーを削除
		*action_result = this->_mysql_client->delete_chatroom_member(mt003->get_room_id(), mt003->get_remove_member_list());

		if (*action_result == true)
		{
			spdlog::info("The chat room(ID:{}) was successfully removed and saved in the database.", mt003->get_room_id());
			*error_msg = "No Error.";
		}
		else
		{
			*error_msg = "The chat room(ID:{}) was not created.", mt003->get_room_id();
			return;
		}
		break;

	case MessageType003::RoomAction::ROOM_NAME_CHANGE:
		// ルーム名の変更
		*action_result = this->_chat_room_handler->change_room_name(mt003->get_room_id(), mt003->get_room_name());
		*action_result = this->_mysql_client->update_chatroom_name(mt003->get_room_id(), mt003->get_room_name());
		if (*action_result == true)
		{
			spdlog::info("The chat room(ID:{}) was successfully changed and saved in the database.", mt003->get_room_id());
			*error_msg = "No Error.";
		}
		else
		{
			*error_msg = "The chat room(ID:{}) was not created.", mt003->get_room_id();
			return;
		}
		break;

	case MessageType003::RoomAction::ROOM_ICON_CHANGE:
		// ルームアイコンの変更
		*action_result = this->_chat_room_handler->change_room_icon(mt003->get_room_id(), mt003->get_room_image_url());
		*action_result = this->_mysql_client->update_chatroom_icon(mt003->get_room_id(), mt003->get_room_image_url());
		if (*action_result == true)
		{
			spdlog::info("The chat room(ID:{}) was successfully changed and saved in the database.", mt003->get_room_id());
			*error_msg = "No Error.";
		}
		else
		{
			*error_msg = "The chat room(ID:{}) was not created.", mt003->get_room_id();
			return;
		}
		break;

	default:
		break;
	}
}

void ChatServerFacade::mt004_process(const std::unique_ptr<MessageType004> &mt004)
{
	LOG_TRACE;
	try
	{
		if (mt004 == nullptr || mt004->is_valid() == false)
		{
			// メッセージ処理
			THROW_APP_EXCEPTION("The reception process was interrupted because the contents of MT002 were invalid.");
		}
		// メッセージをデータベースに保存する
		if (this->_mysql_client->is_room_id_hex_exist(mt004->get_room_id()))
		{
			if (this->_mysql_client->insert_chat_message(mt004->get_message_id(), mt004->get_room_id(), mt004->get_sender_id(), mt004->get_send_time(), mt004->get_content()))
			{
				spdlog::info("The message was successfully saved in the database.");
			}
			else
			{
				LOG_ERROR("The message was not saved in the database.");
			}
		}
		else
		{
			THROW_APP_EXCEPTION("The reception process was interrupted because the room ID was not found.");
		}
	}
	catch (const exception::ExceptionBase &ex)
	{
		LOG_ERROR(ex.what_for_str());
		return;
	}
}


void ChatServerFacade::mt101_process(
	const socket_t socket,
	const std::string &cid,
	const bool verify_result,
	const bool recv_flag)
{
	std::string error_msg = "";
	if (recv_flag == false)
	{
		LOG_ERROR("The reception process was interrupted because the MT001 message was not received properly.");
		return;
	}
	// TODO　ここでcidに関するメッセージをデータベースから取得する。
	json unsend_messages_json, unsend_control_messages_json;
	// 未送信コントロールメッセージを取得する
	unsend_control_messages_json = this->_mysql_client->get_unsend_control_message_fromDB(this->_client_handler->get_client_id(socket));
	// 未送信チャットメッセージを取得する
	unsend_messages_json = this->_mysql_client->get_unsend_message_fromDB(this->_client_handler->get_client_id(socket), 20);

	// MT101を送信する処理を記載する
	auto mt101 = std::make_unique<MessageType101>(cid, verify_result);

	if(verify_result == true){
		error_msg = "Verification succeeded.";
	}else{
		error_msg = "Verification failed.";
	}

	// メッセージのデシリアライズ
	json mt101_json = MessageType101::deserialize_message(mt101, unsend_messages_json, error_msg);

	// メッセージの送信
	bool send_result = this->_client_handler->client_message_send(socket, mt101_json);

	// メッセージ送信が成功の場合
	if (send_result == true)
	{
		spdlog::info("The MT101 message was successfully sent to the client(ip:{}, id:{}).", this->_client_handler->get_client_ipv4_address(socket), cid);

		if (unsend_control_messages_json.empty() != true)
		{
			for (const auto &unsend_control_message_json : unsend_control_messages_json)
			{
				// データベースから未送信コントロールメッセージを削除する
				const auto mid = unsend_control_message_json["mid"].get<uint64_t>();
				const auto cid = unsend_control_message_json["cid"].get<std::string>();
				const auto rid = unsend_control_message_json["room_id"].get<std::string>();
				this->_mysql_client->delete_unsend_control_message(mid, cid, rid);
			}
		}

		if (unsend_messages_json.empty() != true)
		{
			// unsend_messageを削除する
			for (const auto &unsend_message : unsend_messages_json)
			{
				// データベースから未送信メッセージを削除する
				const auto mid = unsend_message["mid"].get<uint64_t>();
				const auto cid = unsend_message["sender_cid"].get<std::string>();
				const auto rid = unsend_message["send_room_id"].get<std::string>();
				this->_mysql_client->delete_unsend_message(mid, cid, rid);
			}
		}
	}
	else
	{
		spdlog::error("The MT101 message was not sent to the client.");
		// TODO MT101の送信に失敗した場合の処理を記載する⇒再送処理する
	}
}

void ChatServerFacade::mt103_process(const std::unique_ptr<MessageType003> &mt003, const std::shared_ptr<Client> &client, socket_t socket)
{
	int send_cid_num = 0;
	std::vector<std::string> unsend_cids;
	std::tie(send_cid_num, unsend_cids) = this->mt103A_process(mt003, socket);
}

const std::tuple<int, std::vector<std::string>> ChatServerFacade::mt103A_process(const std::unique_ptr<MessageType003> &mt003, socket_t socket)
{
	int send_cid_num = 0;
	std::vector<std::string> unsend_cids;
	std::vector<MessageType103A::ClientInfo> client_infos;

	// MT103Aを生成する
	switch (mt003->get_room_action())
	{
	case MessageType003::RoomAction::CREATE_ROOM:
	case MessageType003::RoomAction::ADD_MEMBER:
	case MessageType003::RoomAction::REMOVE_MEMBER:
		client_infos = this->generate_client_infos(mt003->get_add_member_list());
		break;
	default:
		break;
	}
	auto mt103A = std::make_unique<MessageType103A>(
		mt003->get_sender_id(),
		mt003->get_room_id(),
		mt003->get_room_name(),
		mt003->get_room_image_url(),
		client_infos,
		mt003->get_room_action());
	std::tie(send_cid_num, unsend_cids) = this->send_message_to_room<MessageType103A>(mt103A, socket, mt003->get_sender_id());

	return std::make_tuple(send_cid_num, unsend_cids);
}

void ChatServerFacade::mt103B_process(
	const std::unique_ptr<MessageType003> &mt003,
	const std::vector<std::string> &unsend_cids,
	const std::shared_ptr<Client> &client)
{
	std::string message_type = "103B";
	MessageType103B::MessageSendStatus send_status;
	std::string send_room_id = mt003->get_room_id();
	// 送り先のclientの情報を取得する
	auto receivers_infos = this->_chat_room_handler->get_client_infos_by_chatroom_id(send_room_id);
	// ChatroomHandlerが保持しているクライアントリストからチャットルームを検出できない場合
	if (receivers_infos.size() == 0)
	{
		// データベースからチャットルームに参加しているcidのリストを抽出
		auto client_list = this->_mysql_client->get_client_list_from_chatroom(send_room_id);
		if (client_list.size() == 0 && unsend_cids.size() == 0)
		{
			// LOG_ERROR("The Chatroom ID was not found in the database.");
			send_status = MessageType103B::MessageSendStatus::SEND_FAILED;
		}
		else if (client_list.size() != 1 && client_list.size() - 1 == unsend_cids.size())
		{
			// 全てのクライアントにメッセージが送信できなかった場合
			send_status = MessageType103B::MessageSendStatus::SEND_FAILED;
		}
		else if (client_list.size() > 0 && unsend_cids.size() == 0)
		{
			// クライアントにメッセージが送信できた場合
			send_status = MessageType103B::MessageSendStatus::SEND_SUCCESS;
		}

		else
		{ // 一部のクライアントにメッセージが送信できなかった場合
			send_status = MessageType103B::MessageSendStatus::SEND_PARTICAL_SUCCESS;
		}
	}
	else
	{
		if (receivers_infos.size() == 0 && unsend_cids.size() == 0)
		{
			// 全てのクライアントにメッセージが送信できなかった場合
			send_status = MessageType103B::MessageSendStatus::SEND_FAILED;
		}
		else if (receivers_infos.size() - 1 == unsend_cids.size())
		{
			// 全てのクライアントにメッセージが送信できなかった場合
			send_status = MessageType103B::MessageSendStatus::SEND_FAILED;
		}
		else if (receivers_infos.size() > 0 && unsend_cids.size() == 0)
		{
			// クライアントにメッセージが送信できた場合
			send_status = MessageType103B::MessageSendStatus::SEND_SUCCESS;
		}
		else
		{ // 一部のクライアントにメッセージが送信できなかった場合
			send_status = MessageType103B::MessageSendStatus::SEND_PARTICAL_SUCCESS;
		}
	}
	auto mt103B = std::make_unique<MessageType103B>(
		0,
		message_type,
		send_status,
		send_room_id,
		mt003->get_message_id(),
		unsend_cids);
	try
	{
		// mt103Bをデシリアライズする
		json mt103B_json = MessageType103B::deserialize_message(mt103B);
		if (this->_protocol_handler->send_message_json(client->get_ssl(), mt103B_json))
		{
			spdlog::info("The MT103B message was successfully sent to the client(ip:{}, id:{}).", client->get_ipv4_address(), client->get_id());
		}
		else
		{
			LOG_ERROR("The MT103B message was not sent to the client(ip:{}, id:{}).", client->get_ipv4_address(), client->get_id());
		}
	}
	catch (const std::out_of_range &e)
	{
		// エラーハンドリング
		LOG_ERROR("Sender ID not found. exception content: {}", e.what());
	}
}

std::vector<MessageType103A::ClientInfo> ChatServerFacade::generate_client_infos(const std::vector<std::string> &cids)
{
	std::vector<MessageType103A::ClientInfo> client_infos;
	if (cids.size() == 0)
	{
		return std::vector<MessageType103A::ClientInfo>();
	}
	for (const auto &cid : cids)
	{
		std::string client_name, client_mail_addr, client_icon_url;
		std::make_tuple(client_name, client_mail_addr, client_icon_url) = this->_mysql_client->get_client_info_fromDB(cid);
		MessageType103A::ClientInfo add_client_info(cid, client_name, client_mail_addr, client_icon_url);
		client_infos.push_back(add_client_info);
	}
	return client_infos;
}


void ChatServerFacade::mt104_process(const std::unique_ptr<MessageType004> &mt004, const socket_t socket)
{
	int send_cid_num = 0;
	std::vector<std::string> unsend_cids;
	std::tie(send_cid_num, unsend_cids) = this->mt104A_process(mt004, socket);
	this->mt104B_process(mt004, send_cid_num, unsend_cids, socket);
}

void ChatServerFacade::heartbeat_check_process()
{
	LOG_TRACE;
	while (true)
	{
		// 30秒wait
		std::this_thread::sleep_for(std::chrono::seconds(30)); // 30秒間隔
		const auto unres_sockets = this->_client_handler->monitor_heartbeat();
		if(unres_sockets.size() == 0){
			// ハートビート回数をクリアする
			this->_client_handler->all_clear_heart_beat_count();
			continue;
		}
		for(const auto socket : unres_sockets){
			this->handle_disconnect_process(socket);
		}
		this->_client_handler->all_clear_heart_beat_count();
	}
}


#ifdef USE_SELECT
void ChatServerFacade::prepare_client_fd_set()
{
	auto clients_snapshot = this->_client_handler->get_connected_clients_snapshot();
	for (auto &client : clients_snapshot)
	{
		// 有効なソケットにFDSETを実行する
		if (client != nullptr && client->get_socket() != INVALID_SOCKET_FD)
		{
			FD_SET(client->get_socket(), &_socket_handler->get_fd_read());
		}
	}
	this->_socket_handler->copy_fd_read_to_fds(&(_socket_handler->get_fds()), &(_socket_handler->get_fd_read()));
}

void ChatServerFacade::setting_select_process()
{
	while (true)
	{
		{
			this->_socket_handler->initialize_select_param();
			this->prepare_client_fd_set();
		}
	}
}
#endif


void ChatServerFacade::handle_disconnect_process(const socket_t socket)
{
	std::shared_ptr<Client> client = this->_client_handler->get_client_by_socket(socket);

	if (client == nullptr)
	{
		LOG_ERROR("Client not found for socket {}", socket);
		return;
	}
	// すでに切断処理が開始されているか確認（State が Active → Disconnecting への遷移に成功した場合のみ処理）
	Client::ConnectState expected = Client::ConnectState::ACTIVE;
	if (!client->compare_and_set_state(expected, Client::ConnectState::DISCONNECTING))
	{
		// 他スレッドですでに切断中または完了している
		spdlog::warn("Client(ID: {}) is already disconnecting or disconnected.", client->get_id());
		return;
	}
#ifdef USE_POLL
	this->_socket_handler->delete_epoll_ctrl(socket);
	// **ここでブロッキングモードに切り替え**
	// 例:
	int flags = fcntl(socket, F_GETFL, 0);
	if (flags != -1)
	{
		fcntl(socket, F_SETFL, flags & ~O_NONBLOCK);
	}
#endif
	this->_client_handler->client_ssl_shutdown(socket);
	this->_socket_handler->close_socket(socket);
	this->_mysql_client->update_last_login(_client_handler->get_client_id(socket), this->_server_time->get_current_time_to_milliseconds());
	this->_client_handler->set_conncet_status(socket, Client::ConnectState::DISCONNECT);
	this->_client_handler->delete_client(socket);
}


const std::tuple<int, std::vector<std::string>> ChatServerFacade::mt104A_process(const std::unique_ptr<MessageType004> &mt004, socket_t socket)
{
	// mt104Aを生成
	std::string message_type = "104A";
	int send_cid_num = 0;
	std::vector<std::string> unsend_cids;

	// MT104Aを生成する
	auto mt104A = std::make_unique<MessageType104A>(
		0,
		message_type,
		mt004->get_room_id(),
		mt004->get_sender_id(),
		mt004->get_message_id(),
		mt004->get_send_time(),
		mt004->get_content());
	
	std::tie(send_cid_num, unsend_cids) = this->send_message_to_room<MessageType104A>(mt104A, socket, mt004->get_sender_id());
	//this->send_message_to_room<MessageType104A>(mt104A, &unsend_cids, socket, mt004->get_sender_id());

	return std::make_tuple(send_cid_num, unsend_cids);
}


void ChatServerFacade::mt104B_process(const std::unique_ptr<MessageType004> &mt004, const int send_cid_num, const std::vector<std::string> &unsend_cids, const socket_t socket)
{
	LOG_TRACE;
	// mt104Bを生成
	std::string message_type = "104B";
	MessageType104B::MessageSendStatus send_status;
	std::string send_room_id = mt004->get_room_id();

	if(send_cid_num == 0 && unsend_cids.size() == 0){
		// 送信先のクライアントがいない場合
		send_status = MessageType104B::MessageSendStatus::SEND_FAILED;
	}
	//送信先のクライアント(room_idの中にいるcids-1(送信先のクライアントをカウントしない)が1以上で、未送信クライアントがいない場合)
	else if(send_cid_num -1 >= 1 && unsend_cids.size() == 0){
		// 送信先のクライアントが複数いて、未送信クライアントがいない場合
		send_status = MessageType104B::MessageSendStatus::SEND_SUCCESS;
	}else if(send_cid_num -1 == unsend_cids.size()){
		//送信先のクライアントの数がunsend_cidsの数と同じ場合(全クライアントに送信できていない場合)
		send_status = MessageType104B::MessageSendStatus::SEND_FAILED;
	}
	else{
		// 一部のクライアントにメッセージが送信できなかった場合
		send_status = MessageType104B::MessageSendStatus::SEND_PARTIAL_SUCCESS;
	}

	auto mt104B = std::make_unique<MessageType104B>(
		0,
		message_type,
		send_status,
		send_room_id,
		mt004->get_message_id(),
		unsend_cids);
	try
	{
		// mt104Bをデシリアライズする
		json mt104B_json = MessageType104B::deserialize_message(mt104B);
		this->_client_handler->client_message_send(socket, mt104B_json);
	}
	catch (const std::out_of_range &e)
	{
		// エラーハンドリング
		LOG_ERROR("Sender ID not found. exception content: {}", e.what());
	}
	catch (const std::exception &e)
	{
		LOG_ERROR("Exception : {}", e.what());
	}
}

void ChatServerFacade::resend_message_process(const json &message, const socket_t socket)
{
	uint64_t mid = message["message_id"];
	// 1秒スリープする
	std::this_thread::sleep_for(std::chrono::seconds(1));
	// クライアントのソケットを取得する
	for (int resend_count = 1; resend_count < MAX_RESEND_COUNT; resend_count++)
	{
		if (resend_count == 2)
		{
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}
		if (resend_count == 3)
		{
			std::this_thread::sleep_for(std::chrono::seconds(5));
		}
		if (this->_client_handler->get_client_by_socket(socket) != nullptr)
		{
			// メッセージを送信する
			if (this->_client_handler->client_message_send(socket, message))
			{
				spdlog::info("Resend message(mid: {}) to client with socket: {}", mid, socket);
				return;
			}
		}
	}
	LOG_ERROR("Resend message(mid: {}) to client with socket: {} failed.", mid, socket);
}
