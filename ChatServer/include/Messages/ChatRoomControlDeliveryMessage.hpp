#ifndef CHATROOMCONTROLDELIVERYMESSAGE_HPP
#define CHATROOMCONTROLDELIVERYMESSAGE_HPP
#include "BaseMessage.hpp"
#include "Chat/MessageIDGenerator.hpp"
#include "Messages/ChatRoomControlMessage.hpp"
class ChatRoomControlDeliveryMessage :
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
	std::string _sender_cid;	//送信者ID
	std::string _room_id;
	std::string _room_name;
	std::string _room_icon_url;
	std::vector<ClientInfo> _client_info_list;	//クライアントリスト
	ChatRoomControlMessage::RoomAction _room_action;	//ルームアクション

public:
	ChatRoomControlDeliveryMessage() = delete;
	ChatRoomControlDeliveryMessage(
		const std::string& sender_cid,
		const std::string& room_id,
		const std::string& room_name,
		const std::string& room_icon_url,
		const std::vector<ClientInfo>& client_info_list,
		const ChatRoomControlMessage::RoomAction& room_action);
	~ChatRoomControlDeliveryMessage();

	//各種アクセッサ(getter/setter)
	std::string get_sender_cid() const;
	std::string get_room_id() const;
	void set_room_id(const std::string& room_id);
	std::string get_room_name() const;
	void set_room_name(const std::string& room_name);
	std::string get_room_icon_url() const;
	void set_room_icon_url(const std::string& room_icon_url);
	std::vector<ClientInfo> get_client_info_list() const;
	void set_client_info_list(const std::vector<ClientInfo>& client_info_list);
	ChatRoomControlMessage::RoomAction get_room_action() const;
	static json deserialize_message(const std::unique_ptr<ChatRoomControlDeliveryMessage>& chatroom_delivery_message);


};
#endif // !CHATROOMCONTROLDELIVERYMESSAGE_HPP

