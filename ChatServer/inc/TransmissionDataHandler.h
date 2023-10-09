#ifndef TRANSMISSIONDATAHANDLER_H_
#define TRANSMISSIONDATAHANDLER_H_
#define MAX_GROUP_MEMBER_NUM 500

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include "ServerTime.h"
class TransmissionDataHandler
{
public:
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
		std::string scr_ip_addr;

		//送信先のIPアドレス
		std::string dst_ip_addr;

		
		//送信先のリスト
		std::vector<std::string> dst_ip_addr_list;

		
		//送信先のグループID
		std::string dst_group_id;

		std::string scr_recv_data;

		std::string decrypted_recv_data;

		//ペイロード(暗号化)
		std::string scr_payload;

		//ペイロード(復号化)
		std::string decrypted_payload;

		
		FILE* file_data;
		std::string file_name;


		//デフォルトコンストラクタ
		trans_data_info() : scr_ip_addr{ "" }, dst_ip_addr{ "" }, dst_ip_addr_num{ 0 }, dst_group_id{""},
			scr_recv_data{""}, decrypted_recv_data{""}, scr_payload{""}, decrypted_payload{""}, file_data{NULL}, file_name{""} {

			//ここに以下の変数の初期化処理を記載する
			dst_ip_addr_list = { "" };
		}

		//コピーコンストラクタ
		trans_data_info(std::string scr_ip_addr, std::string dst_ip_addr, 
			std::vector<std::string> dst_ip_addr_list, __int16 dst_ip_addr_num, std::string dst_group_id,
			std::string scr_recv_data, std::string decrypted_recv_data,
			std::string scr_payload, FILE* file_data, std::string file_name):scr_ip_addr{scr_ip_addr}, dst_ip_addr{dst_ip_addr},dst_ip_addr_list{dst_ip_addr_list},
			dst_ip_addr_num{ dst_ip_addr_num }, dst_group_id{ dst_group_id },
			scr_recv_data{scr_recv_data}, decrypted_recv_data{decrypted_recv_data},
			scr_payload{ scr_payload }, decrypted_payload{""}, file_data{file_data}, file_name(file_name) {}

	};

	
private:
	//データキュー
	static std::queue<trans_data_info> data_queue;

	//受信データをキューにいれる(未実装)
	bool push_data_queue(trans_data_info data_info);

	//キューから処理済みデータを削除する
	bool pop_data_queue();

public:
	
	TransmissionDataHandler();

	virtual ~TransmissionDataHandler();

	const void show_data_info_queue()const;


	//データを復号化するクラス
	class DataParser
	{
	public:
		DataParser();
		~DataParser();

	private:
		
	};


};             
#endif // !TRANSMISSIONDATAHANDLER_H_


