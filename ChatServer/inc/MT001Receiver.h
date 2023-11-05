#ifndef MT001RECEIVER_H_
#define MT001RECEIVER_H_

#include "TransmissionDataHandler.h"
#include "SocketHandler.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "AppException.h"
#include "StringFormat.h"
#include "ClientHandler.h"


#define DEBUG
#define MAX_RECEIVE_MESSAGE_LEN 2048

struct client_socket_info;

class MT001Receiver :
    public TransmissionDataHandler
{
private:
    //ユニキャストorマルチキャストなのかをデータから抽出する
    int extraction_cast_type(const std::string& data);

    //データタイプを抽出する(未実装)
    int extraction_data_type(const std::string& data);

    //送信元のIPアドレスを抽出する(未実装)
    __int16 extraction_dst_ip_num(const std::string& data);

    //送信元のIPアドレスを抽出する(未実装)
    std::string extraction_scr_ip(const std::string &data);

    //送信先のIPアドレスを抽出する(未実装)
    std::string extraction_dst_ip(const std::string &data);
    
    //送信先のグループIDを抽出する
    std::string extraction_group_id(const std::string& data);

    //送信先のグループIDから送信先のIPアドレスリストを抽出する
    std::vector<std::string> extraction_dst_ip_list(const std::string &group_id);
    
    //暗号化されているペイロードを抽出する
    std::string extraction_scr_payload(const std::string& data);

    //データを送信する
    bool store_receive_data();

    void print_src_recv_data(const std::string& data, std::vector<SocketHandler::client_socket_info>& client_socket_info_list
        , int& detection_num, int& receive_result);

    void print_src_recv_data(const std::string& data, const ClientHandler& client_handler, int& detection_num, int& receive_result);


public:
    MT001Receiver();
    ~MT001Receiver();

    //データ受信処理
    int data_receive(std::vector<SocketHandler::client_socket_info> client_socket_info_list,int detection_num);
    int data_receive(const ClientHandler& client_handler, int detection_num);
    
    void data_parse(std::string& data) override;
};

#endif // !MT001RECEIVER_H_


