#ifndef MT000RECEIVER_H_
#define MT000RECEIVER_H_
#include "Receiver.h"

#define CLIENT_ID_START_POS 3
#define CLIENT_ID_BYTE_SIZE 28
class MT000Receiver : public Receiver
{
	class MT000Data : public Receiver::Message
	{
	public:
		//コンストラクタ・デストラクタ
		explicit MT000Data();
		explicit MT000Data(const std::string& id);
		~MT000Data();

	};


	//関数
private:
	std::string extraction_src_client_id(const std::string& data) override;
	bool check_validation_src_client_id(const std::string& id) override;


public:
	//コンストラクタ・デストラクタ
	MT000Receiver();
	~MT000Receiver();


	//other
	void show_receive_info(const std::string& ipv4_addr, int& detect_num, int& receive_result) override;
	std::unique_ptr<MT000Data> create_mt000_instance();
	std::unique_ptr<MT000Data> create_mt000_instance(const std::string& id);
	std::string data_extraction_(const std::string& data);


};


#endif // !MT000RECEIVER_H_



