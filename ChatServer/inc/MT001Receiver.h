#ifndef MT001RECEIVER_H_
#define MT001RECEIVER_H_
#include "Receiver.h"
#include "common.h"
#define SIGN_TYPE_START_POS 3
#define SIGN_TYPE_BYTE_SIZE 1
#define AUTH_TYPE_START_POS 4
#define AUTH_TYPE_BYTE_SIZE 1
#define CLIENT_ID_START_POS 5
#define CLIENT_ID_END_POS 36
#define CLIENT_NAME_BYTE_SIZE_START_POS 5
#define CLIENT_NAME_BYTE_SIZE 2
#define CLIENT_NAME_START_POS 7
#define CLIENT_MAIL_ADDRESS_BYTE_SIZE 3
#define CLIENT_PASSWORD_BYTE_SIZE 3
#define MAX_SIGN_TYPE 2
#define MIN_SIGN_TYPE 0
#define MAX_AUTH_TYPE 5
#define MIN_AUTH_TYPE 0
#define MAX_NAME_BYTE_SIZE 12
#define MIN_NAME_BYTE_SIZE 6
#define MAX_MAIL_ADDR_SIZE 254
#define MIN_MAIL_ADDR_SIZE 3
#define MAX_PASSWORD_SIZE 127
#define MIN_PASSWORD_SIZE 10

#define DEBUG_MT001_CONTAIN

class MT001Receiver : public Receiver
{

	class MT001Data : public Receiver::Message
	{
	public:
	


		//privateïœêî
	private:
		enum class SIGN_TYPE
		{
			sign_up,
			sign_in,
			sign_out
		};

		enum class AUTH_TYPE
		{
			normal_auth,
			google_auth,
			facebook_auth,
			twitter_auth,
			microsoft_auth,
			yahoo_auth
		};
		SIGN_TYPE m_sign_type;
		AUTH_TYPE m_auth_type;
		std::string m_client_id;
		uint8_t m_client_name_byte_size;
		std::string m_client_name;
		uint8_t m_client_mail_byte_size;
		std::string m_client_mail_addr;
		uint8_t m_client_password_byte_size;
		std::string m_client_password;
		

	public:
		
		//getter
		const int get_sign_type();
		const int get_auth_type();
		const std::string get_client_id();
		const uint8_t get_client_name_byte_size();
		const std::string& get_client_name();
		const uint8_t get_client_mail_addr_byte_size();
		const std::string& get_client_mail_addr();
		const uint8_t get_client_password_byte_size();
		const std::string& get_client_password();
		
		//setter
		void set_sign_type(const uint8_t& sign_type);
		void set_auth_type(const uint8_t& auth_type);
		void set_client_id(const std::string& id);
		void set_client_name_byte_size(const uint8_t& name_byte_size);
		void set_client_name(const std::string& name);
		void set_client_mail_addr_byte_size(const uint8_t& mail_addr_byte_size);
		void set_client_mail_addr(const std::string& mail_addr);
		void set_client_password_byte_size(const uint8_t& password_byte_size);
		void set_client_password(const std::string& password);
		
		explicit MT001Data();
		~MT001Data();
		explicit MT001Data(const uint8_t& sign_type, const uint8_t& auth_type
			, const uint8_t name_byte_size, const std::string& name
			, const uint8_t mail_addr_byte_size, const std::string& mail_addr
			, const uint8_t password_byte_size, const std::string& password);
	};


private:
	uint8_t extraction_sign_type_(const std::string& data);
	uint8_t extraction_auth_type_(const std::string& data);
	uint8_t extraction_client_name_byte_size_(const std::string& data);
	std::string extraction_client_name_(const std::string& data, uint8_t client_name_byte_size);
	uint8_t extraction_client_mail_addr_byte_size_(const std::string& data, uint8_t name_byte_size);
	std::string extraction_client_mail_addr_(const std::string& data, uint8_t name_byte_size, uint8_t mail_addr_byte_size);
	uint8_t extraction_client_password_byte_size_(const std::string& data, uint8_t name_byte_size, uint8_t mail_addr_byte_size);
	std::string extraction_client_password_(const std::string& data, uint8_t name_byte_size, uint8_t mail_addr_byte_size, uint8_t password_byte_size);

	bool check_validation_sign_type(const uint8_t& sign_type);
	bool check_validation_auth_type(const uint8_t& auth_type);
	bool check_validation_client_name_byte_size(const uint8_t& name_byte_size);
	bool check_validation_client_name(const std::string& name);
	bool check_validation_client_mail_addr_byte_size(const uint8_t& mail_addr_byte_size);
	bool check_validation_client_mail_addr(const std::string& mail_addr);
	bool check_validation_client_password_byte_size(const uint8_t& password_byte_size);
	bool check_validation_client_password(const std::string& password);

	bool check_validation_contents(const uint8_t& sign_type, const uint8_t& auth_type
		, const uint8_t name_byte_size, const std::string& name
		, const uint8_t mail_addr_byte_size, const std::string& mail_addr
		, const uint8_t password_byte_size, const std::string& password);

public:
    MT001Receiver();
    ~MT001Receiver();

	void show_receive_info(const std::string& ipv4_addr, int& detect_num, int& receive_result) override;
	void show_mt001_data(const MT001Data& mt001);
	std::unique_ptr<MT001Data> create_mt001data_instance();
	std::unique_ptr<MT001Data> create_mt001data_instance(const uint8_t& sign_type
		, const uint8_t& auth_type, const uint8_t& name_byte_size, std::string& name
		, const uint8_t& mail_addr_byte_size, const std::string& mail_addr
		, const uint8_t& password_byte_size, const std::string& password);
	bool data_extraction(const std::string& data, MT001Data& mt001);
	std::tuple<uint8_t, uint8_t, uint8_t, std::string, uint8_t, std::string, uint8_t, std::string> data_extraction_(const std::string& data);

};


#endif // !MT001RECEIVER_H_
