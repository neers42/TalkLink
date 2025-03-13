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
#define SPLIT_START_POS 3
#define TRANS_TYPE_BYTE_SIZE 1
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

class MT002Receiver : public Receiver
{
    class MT002Data : public Receiver::Message
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
        std::string m_dst_chatroom_id;

public:
        MT002Data();
        MT002Data(const int& trans_type, const std::string& d_id, const std::string& s_id, const std::string& dst_chatroom_id, const int& data_type, const std::string& content);
        ~MT002Data();

        //setter
        void set_trans_type(const int& trans_type);
        void set_data_type(const int& data_type);
        void set_dst_chatroom_id(const std::string& dst_chatroom_id);

        //getter
        const int get_trans_type();
        const int get_data_type();
        std::string& get_dst_chatroom_id();

        void print_message_contents() override;
    };

private:
    
    int extraction_trans_type(const std::string& data);
    std::string extraction_dst_client_id(const std::string& data) override;
    std::string extraction_src_client_id(const std::string& data) override;
    std::string extraction_dst_chatroom_id(const std::string& data);
    int extraction_data_type(const std::string& data);
    std::string extraction_main_content(const std::string& data) override;


    bool check_validation_trans_type(const int& trans_type);
    bool check_validation_dst_client_id(const std::string& d_id) override;
    bool check_validation_src_client_id(const std::string& s_id) override;
    bool check_validation_dst_chatroom_id(const std::string& d_chatroom_id);
    bool check_validation_data_type(const int& data_type);
    bool check_validation_main_content(const std::string& main_content) override;
    bool check_validation_contents(const int& trans_type, const std::string& d_id, const std::string& s_id, const std::string& d_chatroom_id, const int& data_type, const std::string& main_content);

    bool store_receive_data();
    void print_src_recv_data(const std::string& data, const std::string& dst_ipv4_addr, int& detection_num, int& receive_result);
    std::string remove_zero_padding(std::string& ip_address);

public:
    MT002Receiver();
    ~MT002Receiver();

    bool data_extraction(const std::string& data) override;
    void show_receive_info(const std::string& ipv4_addr, int& detect_num, int& receive_result) override;
    void show_receive_info(const json& receive_json, const std::string& ip, int receive_byte) override;
    void show_mt002_data();

    std::unique_ptr<MT002Data> create_mt002data_instance();
    std::unique_ptr<MT002Data> create_mt002data_instance(const int& trans_type, const std::string& d_id, const std::string& s_id, const std::string& d_chatroom_id, const int& data_type, const std::string& main_content);
    std::tuple<int, std::string, std::string, std::string, int, std::string> data_extraction_(const json& receive_json);

};

#endif // !MT002RECEIVER_H_


