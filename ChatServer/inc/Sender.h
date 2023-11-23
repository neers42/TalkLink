#ifndef SENDER_H_
#define SENDER_H_
#include "MT002Receiver.h"
#include "ClientHandler.h"
#include "AppException.h"
class Sender
{
public:
	Sender();
	~Sender();
	

	bool tcp_unicast_data(const ClientHandler& client_handler,const std::string& send_data, const int send_data_length, int send_socket_num);
	bool udp_multicast_data();
private:


};

#endif // !SENDER_H_


