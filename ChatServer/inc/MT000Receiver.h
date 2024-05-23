#ifndef MT000RECEIVER_H_
#define MT000RECEIVER_H_
#include "Receiver.h"

#define CLIENT_ID_START_POS 3
#define CLIENT_ID_BYTE_SIZE 28
class MT000Receiver : public Receiver
{
	class MT000Data
	{
	public:
		//コンストラクタ・デストラクタ
		explicit MT000Data();
		explicit MT000Data(const std::string& id);
		~MT000Data();

		//gettr
		std::string get_mt000_client_id();

		//setter
		void set_mt000_client_id(const std::string& id);


	private:
		std::string client_id;
	};
	//変数
private:
	MT000Data mt000_data;


	//関数
private:
	bool extract_client_id(const std::string& data);


public:
	//コンストラクタ・デストラクタ
	MT000Receiver();
	~MT000Receiver();

	//getter
	MT000Data get_mt000_data();
	std::string get_mt000_client_id();

	//setter
	void set_mt000_data_client_id(const std::string& id);


	//other
	bool data_extraction(const std::string& data) override;
	void show_receive_info(const std::string& ipv4_addr, int& detect_num, int& receive_result) override;



};


#endif // !MT000RECEIVER_H_



