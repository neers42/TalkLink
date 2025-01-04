#ifndef RECEIVER_H_
#define RECEIVER_H_
#define MAX_GROUP_MEMBER_NUM 500
#define MAX_RECEIVE_MESSAGE_LEN 2048
#define MT_START_POS 0
#define MT_BYTE_SIZE 3
#define TRANS_TYPE_START_POS 3
#define TRANS_TYPE_BYTE_SIZE 1
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
//#define DEBUG
class Receiver
{
public:
	class Message
	{
	public:
		Message();
		Message(std::string& s_id, std::string& d_id, std::string& data, std::string& time_stamp, std::string& id);
		~Message();

		
	public:
		//送信元のクライアントID
		std::string src_client_id;

		//送信先のクライアントID
		std::string dst_client_id;

		//送信データの本体データ
		std::string main_content;

		//送信されたメッセージデータのタイムスタンプ
		std::string time_stamp;

		//送信されたメッセージデータのメッセージID
		std::string message_id;

	public:
		const std::string& get_src_client_id() { return this->src_client_id; };
		const std::string& get_dst_client_id() { return this->dst_client_id; };
		const std::string& get_main_content() { return this->main_content; };
		const std::string& get_time_stamp() { return this->time_stamp; };
		const std::string& get_message_id() { return this->message_id; };

		void set_s_client_id(std::string& s_id) { this->src_client_id = s_id; };
		void set_d_client_id(std::string& d_id) { this->dst_client_id = d_id; };
		void set_main_content(std::string& data) { this->main_content = data; };
		void set_message_id(std::string& id) { this->message_id = id; };
		virtual void print_message_contents() = 0;
	private:

	};

private:
    virtual void show_receive_info(const std::string& ipv4_addr, int& detection_num, int& receive_result);
	virtual void show_receive_info(const json& receive_json, const std::string& ip, int receive_byte);
	virtual std::string extraction_src_client_id(const std::string& data);
	virtual std::string extraction_dst_client_id(const std::string& data);
	virtual std::string extraction_main_content(const std::string& data);

	virtual bool check_validation_src_client_id(const std::string& s_id);
	virtual bool check_validation_dst_client_id(const std::string& d_id);
	virtual bool check_validation_main_content(const std::string& main_content);

public:
	Receiver();
	virtual ~Receiver();
	static std::tuple<int32_t, json> receive_data(const SOCKET& receive_socket, int client_num);
	static std::string extraction_mt(const std::string& data);
	static std::string extraction_trans_type(const std::string& data);
	virtual bool check_validation(const std::string& data);



};             
#endif // !RECEIVER_H_


