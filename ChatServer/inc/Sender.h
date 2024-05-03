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
	

	virtual bool tcp_unicast_data(const SOCKET socket ,const std::string& send_data, const int send_data_length, const std::string& dst_client_id, const std::string& src_client_id);
	virtual bool tcp_multicast_data(const std::vector<SOCKET>& send_sockets, std::string& send_data, const int send_data_length, const std::vector<std::string>& dst_client_ids, const std::string& src_client_id);
	virtual bool udp_multicast_data();
private:


};

#endif // !SENDER_H_


