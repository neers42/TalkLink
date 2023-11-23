#ifndef MT002RECEIVER_H_
#define MT002RECEIVER_H_

#include "Receiver.h"
#include "SocketHandler.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "AppException.h"
#include "StringFormat.h"
#include "ClientHandler.h"
#include <tuple>
#include <string>


#define DEBUG
#define MAX_RECEIVE_MESSAGE_LEN 2048

struct client_socket_info;

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
        enum class data_type {

			//メッセージ
			message,
			//イメージファイル
			image,
			//動画ファイル
			video,
            //連絡先
            contact_details
        };

        

    private:
        
        trans_type m_trans_type;

        data_type m_data_type;

        //送信先ipアドレス
        std::string m_dst_ip_address;

        //送信元IPアドレス
        std::string m_src_ip_address;

        //送信先クライアントID
        std::string m_dst_client_id;

        //送信元クライアントID
        std::string m_src_client_id;

        //送信先チャットルームID
        std::string m_dst_chatroom_id;

        std::string m_main_data;

public:
        MT002Data();
        ~MT002Data();

        
        void set_trans_type(const int& trans_type);

        void set_data_type(const int& data_type);

        void set_dst_ip_address(const std::string& dst_ip);

        void set_src_ip_address(const std::string& src_ip);

        void set_dst_client_id(const std::string& dst_client_id);

        void set_src_client_id(const std::string& src_client_id);

        void set_dst_chatroom_id(const std::string& dst_chatroom_id);

        void set_main_data(const std::string& main_data);

        const int get_trans_type() {
            switch (m_trans_type)
            {
            case MT002Receiver::MT002Data::trans_type::unicast:
                return 0;
                break;
            case MT002Receiver::MT002Data::trans_type::multicast:
                return 1;
                break;
            case MT002Receiver::MT002Data::trans_type::bloadcast:
                return 2;
                break;
            default:
                return -1;
                break;
            }
        }

        const int get_data_type() {
            switch (m_data_type)
            {
            case MT002Receiver::MT002Data::data_type::message:
                return 0;
                break;
            case MT002Receiver::MT002Data::data_type::image:
                return 1;
                break;
            case MT002Receiver::MT002Data::data_type::video:
                return 2;
                break;
            case MT002Receiver::MT002Data::data_type::contact_details:
                return 3;
                break;
            default:
                return -1;
                break;
            }
        }

        std::string& get_dst_ip() {
            return m_dst_ip_address;
        }
        std::string& get_src_ip() {
            return m_src_ip_address;
        }
        std::string& get_dst_client_id() {
            return m_dst_client_id;
        }
        std::string& get_src_client_id() {
            return m_src_client_id;
        }
        std::string& get_dst_chatroom_id() {
            return m_dst_chatroom_id;
        }
        std::string& get_main_data() {
            return m_main_data;
        }
    };

private:
    MT002Data m_mt002_data;
    


private:
    
    //ユニキャストorマルチキャストなのかをデータから抽出する
    bool extraction_trans_type(const std::string& data);

   
    //送信先のIPアドレスを抽出する(未実装)
    bool extraction_dst_ip(const std::string &data);

    //送信元のIPアドレスを抽出する
    bool extraction_src_ip(const std::string& data) override;

    bool extraction_dst_client_id(const std::string& data);

    bool extraction_src_clinet_id(const std::string& data);
        
    //送信先のグループIDを抽出する
    bool extraction_dst_chatroom_id(const std::string& data);

    //データタイプを抽出する(未実装)
    bool extraction_data_type(const std::string& data);
 
    //暗号化されているペイロードを抽出する
    bool extraction_main_data(const std::string& data);

    //データを送信する
    bool store_receive_data();

   
    void print_src_recv_data(const std::string& data, const ClientHandler& client_handler, int& detection_num, int& receive_result);
   

    std::string remove_zero_padding(std::string& ip_address);

public:
    MT002Receiver();
    ~MT002Receiver();

    //データ受信処理
    int data_receive(const ClientHandler& client_handler, int detection_num);
    
    bool data_extraction(const std::string& data) override;

    const MT002Data& get_mt002_data() {
        return this->m_mt002_data;
    }
    const std::string& get_dst_ip_address() {
        return m_mt002_data.get_dst_ip();
    }

    const int get_trans_type() {
        return m_mt002_data.get_trans_type();
    }

};

#endif // !MT002RECEIVER_H_


