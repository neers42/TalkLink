#ifndef CHATROOMCONTROLMESSAGE_HPP
#define CHATROOMCONTROLMESSAGE_HPP
#include "Utils/common.hpp"
#include "MessageBase.hpp"
#include "Domain/MessageIDGenerator.hpp"
#include "IReceiveMessage.hpp"

class ChatroomControlMessage :
    public MessageBase, public IReceiveMessage
{
public:
	enum class RoomAction
	{
		CREATE_ROOM,
		LEAVE_ROOM,
		ADD_MEMBER,
		REMOVE_MEMBER,
		ROOM_NAME_CHANGE,
		ROOM_ICON_CHANGE,
		DELETE_ROOM
	};
	

private:
	uint64_t _message_id;	// メッセージID
	std::string _sender_cid;
	std::string _room_id;	//メンバーを追加するチャットルームID
	std::vector<std::string> _add_member_list;	//追加メンバーリスト
	std::vector<std::string> _remove_member_list;	//削除メンバーリスト
	std::string _room_name;	//チャットルーム名
	std::string _room_image_url;	//チャットルームの画像
	RoomAction _room_action;	//チャットルームのアクション
	std::string _message_content;	//メッセージ内容(JSON形式)



public:
	//コンストラクタ
	ChatroomControlMessage();
	ChatroomControlMessage(
		const MessageBase::CommonField& common_fields,
		const uint64_t mid,
		const std::string& sender_cid,
		const std::string& room_id_add_member,
		const std::vector<std::string>& add_member_list,
		const std::vector<std::string>& remove_member_list,
		const std::string& room_name,
		const std::string& room_image,
		const RoomAction& room_action,
		const std::string& message_content);
	//デストラクタ
	~ChatroomControlMessage();

	//各種アクセッサ
	uint64_t get_message_id() const;
	std::string get_sender_cid() const;
	std::string get_room_id() const;
	std::vector<std::string> get_add_member_list() const;
	std::vector<std::string> get_remove_member_list() const;
	std::string get_room_name() const;
	std::string get_room_image_url() const;
	RoomAction get_room_action() const;
	std::string get_message_content() const;

	//IReceiveMessageの実装
	bool validate_payload(const json& message) const override;
	std::unique_ptr<MessageBase> create_instance(const json& message) const override;
	bool validate_value() const override;

private:
	RoomAction parse_room_action(const std::string& action_str) const;
	bool is_valid_room_id(const std::string& room_id, RoomAction room_action) const;
	bool is_valid_room_name(const std::string& room_name) const;
	bool is_valid_room_image_url(const std::string& room_image_url) const;

};
#endif // CHATROOMCONTROLMESSAGE_HPP

