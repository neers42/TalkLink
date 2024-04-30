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
	~Sender();
	

	bool tcp_unicast_data(const SOCKET socket ,const std::string& send_data, const int send_data_length);
	bool tcp_multicast_data(const std::vector<SOCKET> send_sockets, std::string& send_data, const int send_data_length);
	bool udp_multicast_data();
private:


};

#endif // !SENDER_H_


