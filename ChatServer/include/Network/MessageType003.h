#ifndef MESSAGE_TYPE003_H
#define MESSAGE_TYPE003_H
#include "Utils/common.h"
#include "BaseMessage.h"
#include "LogHandler.h"

class MessageType003 :
    public BaseMessage
{
public:
	enum class RoomAction
	{
		CREATE_ROOM,
		LEAVE_ROOM,
		ADD_MEMBER,
		REMOVE_MEMBER,
		ROOM_NAME_CHANGE,
		ROOM_ICON_CHANGE
	};
	

private:
	std::string _sender_id;
	std::string _room_id;	//メンバーを追加するチャットルームID
	std::vector<std::string> _add_member_list;	//追加メンバーリスト
	std::vector<std::string> _remove_member_list;	//削除メンバーリスト
	std::string _room_name;	//チャットルーム名
	std::string _room_image_url;	//チャットルームの画像
	RoomAction _room_action;	//チャットルームのアクション
	std::string _room_create_time;	//チャットルームの作成時間



public:
	//コンストラクタ
	MessageType003() = delete;
	MessageType003(
		const std::string& sender_id,
		const std::string& room_id_add_member,
		const std::vector<std::string>& add_member_list,
		const std::vector<std::string>& remove_member_list,
		const std::string& room_name,
		const std::string& room_image,
		const RoomAction& room_action,
		const std::string& room_create_time);
	//デストラクタ
	~MessageType003();

	//各種アクセッサ
	std::string get_sender_id() const;
	std::string get_room_id() const;
	std::vector<std::string> get_add_member_list() const;
	std::vector<std::string> get_remove_member_list() const;
	std::string get_room_name() const;
	std::string get_room_image_url() const;
	RoomAction get_room_action() const;
	std::string get_room_create_time() const;

	//メッセージシリアライズ
	static std::unique_ptr<MessageType003> serialize_message(const json& recv_json);

};
#endif // MESSAGE_TYPE003_H

