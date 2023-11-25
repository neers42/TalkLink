#pragma once
#include "Receiver.h"
#define CLIENT_NAME_START_POS 6
#define CLIENT_NAME_BYTE_SIZE 2
#define CLIENT_MAIL_ADDRESS_BYTE_SIZE 3
#define CLIENT_PASSWORD_BYTE_SIZE 3

#define DEBUG_MT001_CONTAIN

class MT001Receiver :
    public Receiver
{

	class MT001Data
	{
	public:
		MT001Data();
		~MT001Data();

		//privateïœêî
	private:
		enum class SIGN_TYPE
		{
			sign_up,
			sign_in,
			sign_out
		};
		SIGN_TYPE m_sign_type;
		uint8_t m_client_name_byte_size;
		std::string m_client_name;
		uint8_t m_client_mail_byte_size;
		std::string m_client_mail_addr;
		uint8_t m_client_password_byte_size;
		std::string m_client_password;

	public:
		
		//getter
		const int get_sign_type();
		const uint8_t get_client_name_byte_size();
		const std::string& get_client_name();
		const uint8_t get_client_mail_addr_byte_size();
		const std::string& get_client_mail_addr();
		const uint8_t get_client_password_byte_size();
		const std::string& get_client_password();
		
		//setter
		void set_sign_type(const uint8_t& sign_type);
		void set_client_name_byte_size(const uint8_t& name_byte_size);
		void set_client_name(const std::string& name);
		void set_client_mail_addr_byte_size(const uint8_t& mail_addr_byte_size);
		void set_client_mail_addr(const std::string& mail_addr);
		void set_client_password_byte_size(const uint8_t& password_byte_size);
		void set_client_password(const std::string& password);

	};

private:
	MT001Data m_mt001_data;

private:
	bool extraction_sign_type(const std::string& data);
	bool extraction_client_name_byte_size(const std::string& data);
	bool extraction_client_name(const std::string& data);
	bool extraction_client_mail_addr_byte_size(const std::string& data);
	bool extraction_client_mail_addr(const std::string& data);
	bool extraction_client_password_byte_size(const std::string& data);
	bool extraction_client_password(const std::string& data);

public:
    MT001Receiver();
    ~MT001Receiver();

	bool data_extraction(const std::string& data) override;
	MT001Data& get_mt001_data();
};

