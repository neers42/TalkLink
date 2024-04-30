#ifndef MT002RECEIVER_H_
#define MT002RECEIVER_H_

#include "Receiver.h"
#include "SocketHandler.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "AppException.h"
#include "StringFormat.h"
#include "ClientHandler.h"
#include "common.h"
#include <tuple>
#include <string>


#define DEBUG_MT002_CONTAIN
#define MAX_RECEIVE_MESSAGE_LEN 2048
#define CLIENT_DST_ID_START_POS 4
#define CLIENT_DST_ID_BYTE_SIZE 28
#define CLIENT_SRC_ID_START_POS 32
#define CLIENT_SRC_ID_BYTE_SIZE 28
#define CHATROOM_ID_START_POS 60
#define CHATROOM_ID_BYTE_SIZE 36
#define DATA_TYPE_START_POS 96
#define DATA_TYPE_BYTE_SIZE 1
#define MAIN_DATA_START_POS 97


//struct client_socket_info;

class MT002Receiver :
    public Receiver
{
    class MT002Data
    {
    
    public:
    enum class trans_type
        {
            unicast,
            multicast,
            bloadcast
        };

		//データタイプ
        enum class data_type 
        {
			message,
			image,
			video,
            contact_details
        };

    private:
        trans_type m_trans_type;
        data_type m_data_type;
        std::string m_dst_ip_address;
        std::string m_src_ip_address;
        std::string m_dst_client_id;
        std::string m_src_client_id;
        std::string m_dst_chatroom_id;
        std::string m_main_data;

public:
        MT002Data();
        ~MT002Data();

        //setter
        void set_trans_type(const int& trans_type);
        void set_data_type(const int& data_type);
        void set_dst_client_id(const std::string& dst_client_id);
        void set_src_client_id(const std::string& src_client_id);
        void set_dst_chatroom_id(const std::string& dst_chatroom_id);
        void set_main_data(const std::string& main_data);

        //getter
        const int get_trans_type();
        const int get_data_type();
        std::string& get_dst_client_id();
        std::string& get_src_client_id();
        std::string& get_dst_chatroom_id();
        std::string& get_main_data();
    };

private:
    MT002Data m_mt002_data;

private:
    
    bool extraction_trans_type(const std::string& data);
    bool extraction_dst_client_id(const std::string& data);
    bool extraction_src_clinet_id(const std::string& data);
    bool extraction_dst_chatroom_id(const std::string& data);
    bool extraction_data_type(const std::string& data);
    bool extraction_main_data(const std::string& data);
    bool store_receive_data();

    void print_src_recv_data(const std::string& data, const ClientHandler& client_handler, int& detection_num, int& receive_result);
   
    std::string remove_zero_padding(std::string& ip_address);

public:
    MT002Receiver();
    ~MT002Receiver();

    int data_receive(const ClientHandler& client_handler, int detection_num);
    bool data_extraction(const std::string& data) override;

    const MT002Data& get_mt002_data();
    const std::string& get_dst_id();
    const int get_trans_type();
    const std::string& get_dst_chatroom_id();

};

#endif // !MT002RECEIVER_H_


