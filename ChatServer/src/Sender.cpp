#include "Sender.h"



Sender::Sender()
{
}

Sender::~Sender()
{
}

std::string Sender::generate_heart_beat_message()
{
	std::string heart_beat_message = StringFormat("100%sHeartBeat from server.", ServerTime::get_current_time());
	return heart_beat_message;
}

std::vector<SOCKET> Sender::send_heart_beat_message(const std::vector<SOCKET> sockets)
{
	std::vector<SOCKET> fail_send_heart_beat_message_socket_list;
	//ハートビート用のメッセージを生成
	std::string heart_beat_message = this->generate_heart_beat_message();

	fail_send_heart_beat_message_socket_list = this->tcp_multicast_data(sockets, heart_beat_message, heart_beat_message.size());
	return fail_send_heart_beat_message_socket_list;
}


bool Sender::tcp_unicast_data(const SOCKET socket, const std::string& send_data, const int send_data_length, const std::string& dst_client_id, const std::string& src_client_id)
{
	//TODO
	//送信失敗したときに、失敗したsocketの番号を返すように改修する
	try
	{
		int send_result = send(socket, send_data.c_str(), send_data_length, 0);
		if (send_result ==SOCKET_ERROR) {
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				THROW_APP_EXCEPTION("SEND_ERROR CASE : WSANOTINITIALISED(Successful WSAStartup not yet performed.Either the application has not called WSAStartup or WSAStartup failed.The application may be accessing a socket that the current active task does not own(that is, trying to share a socket between tasks), or WSACleanup has been called too many times.)");
				break;
			case WSAENETDOWN:
				THROW_APP_EXCEPTION("Network is down.A socket operation encountered a dead network.This could indicate a serious failure of the network system(that is, the protocol stack that the Windows Sockets DLL runs over), the network interface, or the local network itself.");
				break;
			case WSAEACCES:
				THROW_APP_EXCEPTION("Permission denied.An attempt was made to access a socket in a way forbidden by its access permissions.An example is using a broadcast address for sendto without broadcast permission being set using setsockopt(SO_BROADCAST).Another possible reason for the WSAEACCES error is that when the bind function is called(on Windows NT 4.0 with SP4 and later), another application, service, or kernel mode driver is bound to the same address with exclusive access.Such exclusive access is a new feature of Windows NT 4.0 with SP4 and later, and is implemented by using the SO_EXCLUSIVEADDRUSE option.");
				break;
			case WSAEINTR:
				THROW_APP_EXCEPTION("Interrupted function call.A blocking operation was interrupted by a call to WSACancelBlockingCall.");
				break;
			case WSAEINPROGRESS:
				THROW_APP_EXCEPTION("Operation now in progress.A blocking operation is currently executing.Windows Sockets only allows a single blocking operation—per - task or thread—to be outstanding, and if any other function call is made(whether or not it references that or any other socket) the function fails with the WSAEINPROGRESS error.");
				break;
			case WSAEFAULT:
				THROW_APP_EXCEPTION("Bad address.The system detected an invalid pointer address in attempting to use a pointer argument of a call.This error occurs if an application passes an invalid pointer value, or if the length of the buffer is too small.For instance, if the length of an argument, which is a sockaddr structure, is smaller than the sizeof(sockaddr).");
				break;
			case WSAENOBUFS:
				THROW_APP_EXCEPTION("No buffer space available.An operation on a socket could not be performed because the system lacked sufficient buffer space or because a queue was full.");
				break;
			case WSAENOTCONN:
				THROW_APP_EXCEPTION("Socket is not connected.A request to send or receive data was disallowed because the socket is not connected and (when sending on a datagram socket using sendto) no address was supplied.Any other type of operation might also return this error—for example, setsockopt setting SO_KEEPALIVE if the connection has been reset.");
				break;
			case WSAENOTSOCK:
				THROW_APP_EXCEPTION("Socket operation on nonsocket.An operation was attempted on something that is not a socket.Either the socket handle parameter did not reference a valid socket, or for select, a member of an fd_set was not valid.");
				break;
			case WSAEOPNOTSUPP:
				THROW_APP_EXCEPTION("Operation not supported.The attempted operation is not supported for the type of object referenced.Usually this occurs when a socket descriptor to a socket that cannot support this operation is trying to accept a connection on a datagram socket.");
				break;
			case WSAESHUTDOWN:
				THROW_APP_EXCEPTION("Cannot send after socket shutdown.A request to send or receive data was disallowed because the socket had already been shut down in that direction with a previous shutdown call.By calling shutdown a partial close of a socket is requested, which is a signal that sending or receiving, or both have been discontinued.");
				break;
			case WSAEWOULDBLOCK:
				THROW_APP_EXCEPTION("Resource temporarily unavailable.This error is returned from operations on nonblocking sockets that cannot be completed immediately, for example recv when no data is queued to be read from the socket.It is a nonfatal error, and the operation should be retried later.It is normal for WSAEWOULDBLOCK to be reported as the result from calling connect on a nonblocking SOCK_STREAM socket, since some time must elapse for the connection to be established.");
				break;
			case WSAEMSGSIZE:
				THROW_APP_EXCEPTION("Message too long.A message sent on a datagram socket was larger than the internal message buffer or some other network limit, or the buffer used to receive a datagram was smaller than the datagram itself.");
				break;
			case WSAEHOSTUNREACH:
				THROW_APP_EXCEPTION("No route to host.A socket operation was attempted to an unreachable host.See WSAENETUNREACH.");
				break;
			case WSAEINVAL:
				THROW_APP_EXCEPTION("Invalid argument.Some invalid argument was supplied(for example, specifying an invalid level to the setsockopt function).In some instances, it also refers to the current state of the socket—for instance, calling accept on a socket that is not listening.");
				break;
			case WSAECONNABORTED:
				THROW_APP_EXCEPTION("Software caused connection abort.An established connection was aborted by the software in your host computer, possibly due to a data transmission time - out or protocol error.");
				break;
			case WSAECONNRESET:
				THROW_APP_EXCEPTION("Connection reset by peer.An existing connection was forcibly closed by the remote host.This normally results if the peer application on the remote host is suddenly stopped, the host is rebooted, the host or remote network interface is disabled, or the remote host uses a hard close(see setsockopt for more information on the SO_LINGER option on the remote socket).This error may also result if a connection was broken due to keep - alive activity detecting a failure while one or more operations are in progress.Operations that were in progress fail with WSAENETRESET.Subsequent operations fail with WSAECONNRESET.");
				break;
			case WSAETIMEDOUT:
				THROW_APP_EXCEPTION("Connection timed out.A connection attempt failed because the connected party did not properly respond after a period of time, or the established connection failed because the connected host has failed to respond.");
				break;

			default:
				break;
			}
		}
		else {
			LogHandler::logging_info(LogHandler::LogType::SEND, std::cout, StringFormat("The server sent a message to the client. (src id : %s  =>  dst id : %s)", src_client_id, dst_client_id));
			return true;
		}

	}
	catch (const exception::ExceptionBase& ex)
	{
		LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
	}

	return false;
}

std::vector<SOCKET> Sender::tcp_multicast_data(const std::vector<SOCKET>& send_sockets, const std::string& send_data, const int send_data_length)
{
	std::vector<SOCKET> fail_send_sockets;
	for (auto&& socket : send_sockets) {
		try
		{
			int send_result = send(socket, send_data.c_str(), send_data_length, 0);
			if (send_result == SOCKET_ERROR) {
				switch (WSAGetLastError())
				{
				case WSANOTINITIALISED:
					THROW_APP_EXCEPTION("SEND_ERROR CASE : WSANOTINITIALISED(Successful WSAStartup not yet performed.Either the application has not called WSAStartup or WSAStartup failed.The application may be accessing a socket that the current active task does not own(that is, trying to share a socket between tasks), or WSACleanup has been called too many times.)");
					break;
				case WSAENETDOWN:
					THROW_APP_EXCEPTION("Network is down.A socket operation encountered a dead network.This could indicate a serious failure of the network system(that is, the protocol stack that the Windows Sockets DLL runs over), the network interface, or the local network itself.");
					break;
				case WSAEACCES:
					THROW_APP_EXCEPTION("Permission denied.An attempt was made to access a socket in a way forbidden by its access permissions.An example is using a broadcast address for sendto without broadcast permission being set using setsockopt(SO_BROADCAST).Another possible reason for the WSAEACCES error is that when the bind function is called(on Windows NT 4.0 with SP4 and later), another application, service, or kernel mode driver is bound to the same address with exclusive access.Such exclusive access is a new feature of Windows NT 4.0 with SP4 and later, and is implemented by using the SO_EXCLUSIVEADDRUSE option.");
					break;
				case WSAEINTR:
					THROW_APP_EXCEPTION("Interrupted function call.A blocking operation was interrupted by a call to WSACancelBlockingCall.");
					break;
				case WSAEINPROGRESS:
					THROW_APP_EXCEPTION("Operation now in progress.A blocking operation is currently executing.Windows Sockets only allows a single blocking operation—per - task or thread—to be outstanding, and if any other function call is made(whether or not it references that or any other socket) the function fails with the WSAEINPROGRESS error.");
					break;
				case WSAEFAULT:
					THROW_APP_EXCEPTION("Bad address.The system detected an invalid pointer address in attempting to use a pointer argument of a call.This error occurs if an application passes an invalid pointer value, or if the length of the buffer is too small.For instance, if the length of an argument, which is a sockaddr structure, is smaller than the sizeof(sockaddr).");
					break;
				case WSAENOBUFS:
					THROW_APP_EXCEPTION("No buffer space available.An operation on a socket could not be performed because the system lacked sufficient buffer space or because a queue was full.");
					break;
				case WSAENOTCONN:
					THROW_APP_EXCEPTION("Socket is not connected.A request to send or receive data was disallowed because the socket is not connected and (when sending on a datagram socket using sendto) no address was supplied.Any other type of operation might also return this error—for example, setsockopt setting SO_KEEPALIVE if the connection has been reset.");
					break;
				case WSAENOTSOCK:
					THROW_APP_EXCEPTION("Socket operation on nonsocket.An operation was attempted on something that is not a socket.Either the socket handle parameter did not reference a valid socket, or for select, a member of an fd_set was not valid.");
					break;
				case WSAEOPNOTSUPP:
					THROW_APP_EXCEPTION("Operation not supported.The attempted operation is not supported for the type of object referenced.Usually this occurs when a socket descriptor to a socket that cannot support this operation is trying to accept a connection on a datagram socket.");
					break;
				case WSAESHUTDOWN:
					THROW_APP_EXCEPTION("Cannot send after socket shutdown.A request to send or receive data was disallowed because the socket had already been shut down in that direction with a previous shutdown call.By calling shutdown a partial close of a socket is requested, which is a signal that sending or receiving, or both have been discontinued.");
					break;
				case WSAEWOULDBLOCK:
					THROW_APP_EXCEPTION("Resource temporarily unavailable.This error is returned from operations on nonblocking sockets that cannot be completed immediately, for example recv when no data is queued to be read from the socket.It is a nonfatal error, and the operation should be retried later.It is normal for WSAEWOULDBLOCK to be reported as the result from calling connect on a nonblocking SOCK_STREAM socket, since some time must elapse for the connection to be established.");
					break;
				case WSAEMSGSIZE:
					THROW_APP_EXCEPTION("Message too long.A message sent on a datagram socket was larger than the internal message buffer or some other network limit, or the buffer used to receive a datagram was smaller than the datagram itself.");
					break;
				case WSAEHOSTUNREACH:
					THROW_APP_EXCEPTION("No route to host.A socket operation was attempted to an unreachable host.See WSAENETUNREACH.");
					break;
				case WSAEINVAL:
					THROW_APP_EXCEPTION("Invalid argument.Some invalid argument was supplied(for example, specifying an invalid level to the setsockopt function).In some instances, it also refers to the current state of the socket—for instance, calling accept on a socket that is not listening.");
					break;
				case WSAECONNABORTED:
					THROW_APP_EXCEPTION("Software caused connection abort.An established connection was aborted by the software in your host computer, possibly due to a data transmission time - out or protocol error.");
					break;
				case WSAECONNRESET:
					THROW_APP_EXCEPTION("Connection reset by peer.An existing connection was forcibly closed by the remote host.This normally results if the peer application on the remote host is suddenly stopped, the host is rebooted, the host or remote network interface is disabled, or the remote host uses a hard close(see setsockopt for more information on the SO_LINGER option on the remote socket).This error may also result if a connection was broken due to keep - alive activity detecting a failure while one or more operations are in progress.Operations that were in progress fail with WSAENETRESET.Subsequent operations fail with WSAECONNRESET.");
					break;
				case WSAETIMEDOUT:
					THROW_APP_EXCEPTION("Connection timed out.A connection attempt failed because the connected party did not properly respond after a period of time, or the established connection failed because the connected host has failed to respond.");
					break;

				default:
					THROW_APP_EXCEPTION("Failed to multicast the message for some reason");
					break;
				}
				fail_send_sockets.push_back(socket);
			}

		}
		catch (const exception::ExceptionBase& ex)
		{
			LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("%s", ex.what_for_str()));
		}
	}
	return fail_send_sockets;
}

bool Sender::udp_multicast_data()
{
	return false;
}

