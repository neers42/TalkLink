#ifndef RECEIVER_H_
#define RECEIVER_H_


#include "TransmissionDataHandler.h"

class Receiver :
    public TransmissionDataHandler
{
private:
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

public:
    Receiver();
    ~Receiver();

    //データ受信処理
    bool data_receive(int detection_num);
    
};

#endif // !RECEIVER_H_


