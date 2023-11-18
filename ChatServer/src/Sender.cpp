#include "Sender.h"



Sender::Sender()
{
}

Sender::~Sender()
{
}

bool Sender::tcp_unicast_data(const ClientHandler& client_handler, const std::string& send_data, const int send_data_length, int send_socket_num)
{
	try
	{
		
		int send_result = send(client_handler.connected_client_list[send_socket_num].get_client_socket(), send_data.c_str(), send_data_length, 0);
		if (send_result == SOCKET_ERROR) {
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				break;
			case WSAENETDOWN:
				break;
			case WSAEACCES:
				break;
			case WSAEINTR:
				break;
			case WSAEINPROGRESS:
				break;
			case WSAEFAULT:
				break;
			case WSAENOBUFS:
				break;
			case WSAENOTCONN:
				break;
			case WSAENOTSOCK:
				break;
			case WSAEOPNOTSUPP:
				break;
			case WSAESHUTDOWN:
				break;
			case WSAEWOULDBLOCK:
				break;
			case WSAEMSGSIZE:
				break;
			case WSAEHOSTUNREACH:
				break;
			case WSAEINVAL:
				break;
			case WSAECONNABORTED:
				break;
			case WSAECONNRESET:
				break;
			case WSAETIMEDOUT:
				break;

			default:
				break;
			}
		}

	}
	catch (const exception::ExceptionBase& ex)
	{
		std::cerr << StringFormat("%s\33[31m[ERROR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str());
	}

	return false;
}

