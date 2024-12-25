#ifndef SENDER_H_
#define SENDER_H_
#include "MT002Receiver.h"
#include "ClientHandler.h"
#include "AppException.h"
#include "common.h"
class Sender
{
public:
	Sender();
	virtual ~Sender();
	
	std::string generate_heart_beat_message();
	std::vector<SOCKET> send_heart_beat_message(const std::vector<SOCKET> sockets);
	virtual bool tcp_unicast_data(const SOCKET socket ,const json& send_json_data, const int send_data_length, const std::string& dst_client_id, const std::string& src_client_id);
	virtual std::vector<SOCKET> tcp_multicast_data(const std::vector<SOCKET>& send_sockets, const json& send_json_data, const int send_data_length);
	virtual bool udp_multicast_data();

private:


};

#endif // !SENDER_H_


