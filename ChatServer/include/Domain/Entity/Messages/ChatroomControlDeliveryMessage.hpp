#ifndef CHATROOMCONTROLDELIVERYMESSAGE_HPP
#define CHATROOMCONTROLDELIVERYMESSAGE_HPP
#include "MessageBase.hpp"
#include "ISendMessage.hpp"
#include "Domain/MessageIDGenerator.hpp"
#include "Domain/Entity/Messages/ChatroomControlMessage.hpp"
class ChatroomControlDeliveryMessage :
    public MessageBase, public ISendMessage
{
public:
	struct UserInfo {
		std::string uid;	//ユーザーID
		std::string name;	//ユーザー名
		std::string mail_addr;
		std::string icon_url;

		UserInfo(
			const std::string& uid,
			const std::string& name,
			const std::string& mail_addr,
			const std::string& icon_url)
			:uid{uid}, name{name}, mail_addr{mail_addr}, icon_url{icon_url}
		{
		}
	};
private:
	std::string _sender_uid;	//送信者ID
	std::string _room_id;
	std::string _room_name;
	std::string _room_icon_url;
	std::vector<UserInfo> _user_infos;	//クライアントリスト
	ChatroomControlMessage::RoomAction _room_action;	//ルームアクション

public:
	ChatroomControlDeliveryMessage();
	ChatroomControlDeliveryMessage(
		const MessageBase::CommonField& common_fields,
		const uint64_t& mid,
		const std::string& sender_uid,
		const std::string& room_id,
		const std::string& room_name,
		const std::string& room_icon_url,
		const std::vector<UserInfo>& user_infos,
		const ChatroomControlMessage::RoomAction& room_action);
	~ChatroomControlDeliveryMessage();

	//各種アクセッサ(getter/setter)
	std::string get_sender_uid() const;
	std::string get_room_id() const;
	void set_room_id(const std::string& room_id);
	std::string get_room_name() const;
	void set_room_name(const std::string& room_name);
	std::string get_room_icon_url() const;
	void set_room_icon_url(const std::string& room_icon_url);
	std::vector<UserInfo> get_user_infos() const;
	void set_user_infos(const std::vector<UserInfo>& user_infos);
	ChatroomControlMessage::RoomAction get_room_action() const;

	//ISendMessageの実装
	json serialize_payload() const override;
	bool validate_payload() const override;

private:
	ChatroomControlMessage::RoomAction parse_room_action(const std::string& action_str) const;
	bool is_valid_room_id(const std::string& room_id) const;
	bool is_valid_room_name(const std::string& room_name) const;
	bool is_valid_room_icon_url(const std::string& room_icon_url) const;


};
#endif // !CHATROOMCONTROLDELIVERYMESSAGE_HPP

