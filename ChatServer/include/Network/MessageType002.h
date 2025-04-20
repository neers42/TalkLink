#ifndef MESSAGE_TYPE002_H
#define MESSAGE_TYPE002_H
#include "common.h"
#include "BaseMessage.h"
#include "LogHandler.h"

class MessageType002 :
    public BaseMessage
{
public:
	enum class RoomType
	{
		ONE_TO_ONE,
		GROUP
	};
	enum class RoomAction
	{
		CREATE_ROOM,
		DELETE_ROOM,
		ADD_MEMBER,
		REMOVE_MEMBER
	};

private:
	std::string _room_id_add_member;	//メンバーを追加するチャットルームID
	std::string _room_id_remove_member;	//メンバーを削除するチャットルームID
	std::string _add_member_id;	//追加するメンバーのID
	std::string _remove_member_id;	//削除するメンバーのID
	std::string _room_name;	//チャットルーム名
	std::string _room_image;	//チャットルームの画像
	std::string _room_description;	//チャットルームの説明
	std::string _room_type;	//チャットルームの種類
	std::string _room_id;	//チャットルームID
	std::string _room_action;	//チャットルームのアクション
	std::string _room_create_time;	//チャットルームの作成時間



public:
	//コンストラクタ
	MessageType002();
	//デストラクタ
	~MessageType002();

	static std::unique_ptr<MessageType002> serialize_message(const json& recv_json);

};
#endif // MESSAGE_TYPE002_H

