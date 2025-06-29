#ifndef MESSAGETYPE103A_H
#define MESSAGETYPE103A_H
#include "BaseMessage.h"
#include "Chat/MessageIDGenerator.h"
#include "MessageType003.h"
class MessageType103A :
    public BaseMessage
{
public:
	struct ClientInfo {
		std::string cid;	//クライアントID
		std::string name;	//クライアント名
		std::string mail_addr;
		std::string icon_url;

		ClientInfo(
			const std::string& cid,
			const std::string& name,
			const std::string& mail_addr,
			const std::string& icon_url)
			:cid{cid}, name{name}, mail_addr{mail_addr}, icon_url{icon_url}
		{
		}
	};
private:
	std::string _sender_id;	//送信者ID
	std::string _room_id;
	std::string _room_name;
	std::string _room_icon_url;
	std::vector<ClientInfo> _client_info_list;	//クライアントリスト
	MessageType003::RoomAction _room_action;	//ルームアクション

public:
	MessageType103A() = delete;
	MessageType103A(
		const std::string& sender_id,
		const std::string& room_id,
		const std::string& room_name,
		const std::string& room_icon_url,
		const std::vector<ClientInfo>& client_info_list,
		const MessageType003::RoomAction& room_action);
	~MessageType103A();

	//各種アクセッサ(getter/setter)
	std::string get_sender_id() const;
	std::string get_room_id() const;
	void set_room_id(const std::string& room_id);
	std::string get_room_name() const;
	void set_room_name(const std::string& room_name);
	std::string get_room_icon_url() const;
	void set_room_icon_url(const std::string& room_icon_url);
	std::vector<ClientInfo> get_client_info_list() const;
	void set_client_info_list(const std::vector<ClientInfo>& client_info_list);
	MessageType003::RoomAction get_room_action() const;
	static json deserialize_message(const std::unique_ptr<MessageType103A>& mt103A);


};
#endif // !MESSAGETYPE103A_H

