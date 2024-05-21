#ifndef RECEIVER_H_
#define RECEIVER_H_
#define MAX_GROUP_MEMBER_NUM 500
#define MAX_RECEIVE_MESSAGE_LEN 2048
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include "ServerTime.h"
#include "ClientHandler.h"
#include "AppException.h"
#include "common.h"
#include <tuple>
#include <queue>
#define DEBUG
class Receiver
{
protected:
	struct trans_data_info {

		//データタイプ
		enum data_type
		{
			//メッセージ
			message,
			//イメージファイル
			image,
			//動画ファイル
			video,
			//ログイン情報
			login_data
		};

		//送信先のIPアドレス数
		__int16 dst_ip_addr_num;

		//送信元のIPアドレス
		std::string src_ip_addr;

		//送信先のIPアドレス
		std::string dst_ip_addr;

		
		//送信先のリスト
		std::vector<std::string> dst_ip_addr_list;

		
		//送信先のグループID
		std::string dst_group_id;

		std::string src_recv_data;

		std::string decrypted_recv_data;

		//ペイロード(暗号化)
		std::string scr_payload;

		//ペイロード(復号化)
		std::string decrypted_payload;

		
		FILE* file_data;
		std::string file_name;


		//デフォルトコンストラクタ
		trans_data_info() : src_ip_addr{ "" }, dst_ip_addr{ "" }, dst_ip_addr_num{ 0 }, dst_group_id{""},
			src_recv_data{""}, decrypted_recv_data{""}, scr_payload{""}, decrypted_payload{""}, file_data{NULL}, file_name{""} {

			//ここに以下の変数の初期化処理を記載する
			dst_ip_addr_list = { "" };
		}

		//コピーコンストラクタ
		trans_data_info(std::string scr_ip_addr, std::string dst_ip_addr, 
			std::vector<std::string> dst_ip_addr_list, __int16 dst_ip_addr_num, std::string dst_group_id,
			std::string scr_recv_data, std::string decrypted_recv_data,
			std::string scr_payload, FILE* file_data, std::string file_name):src_ip_addr{scr_ip_addr}, dst_ip_addr{dst_ip_addr},dst_ip_addr_list{dst_ip_addr_list},
			dst_ip_addr_num{ dst_ip_addr_num }, dst_group_id{ dst_group_id },
			src_recv_data{scr_recv_data}, decrypted_recv_data{decrypted_recv_data},
			scr_payload{ scr_payload }, decrypted_payload{""}, file_data{file_data}, file_name(file_name) {}

		//メッセージキュー
		std::queue<std::string> message_queue;
		
	};
protected:
	virtual bool data_extraction(const std::string& data) = 0;

private:
    virtual void show_receive_info(const std::string& ipv4_addr, int& detection_num, int& receive_result);

public:
	Receiver();
	virtual ~Receiver();
	static std::tuple<int32_t, std::string> receive_data(const SOCKET& receive_socket, int client_num);
	static std::string extraction_mt(const std::string& data);
	static std::string extraction_trans_type(const std::string& data);


};             
#endif // !RECEIVER_H_


