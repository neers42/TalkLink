#pragma once
#include "Receiver.h"
class MT001Receiver :
    public Receiver
{

	class MT001Data
	{
	public:
		MT001Data();
		~MT001Data();

		//private•Ï”
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
		uint8_t m_client_gmail_byte_size;
		std::string m_client_gmail;
		uint8_t m_client_password_byte_size;
		std::string m_client_password;

	public:
		const int get_sign_type() {
			switch (m_sign_type) {
			case SIGN_TYPE::sign_up:
				return 0;
				break;
			case SIGN_TYPE::sign_in:
				return 1;
				break;
			case SIGN_TYPE::sign_out:
				return 2;
				break;
			default:
				return -1;
				break;
			}
		}

		//getter
		const uint8_t get_client_name_byte_size() {
			return this->m_client_name_byte_size;
		}

		const std::string& get_client_name() {
			return this->m_client_name;
		}

		const uint8_t get_client_gmail_byte_size() {
			return this->m_client_gmail_byte_size;
		}

		const std::string& get_client_gmail() {
			return this->m_client_gmail;
		}

		const uint8_t get_client_password_byte_size() {
			return this->m_client_password_byte_size;
		}

		const std::string& get_client_password() {
			return this->m_client_password;
		}
		
		void set_sign_type(const uint8_t& sign_type);
		void set_client_name_byte_size(const uint8_t& name_byte_size);
		void set_client_name(const std::string& name);
		void set_client_gmail_byte_size(const uint8_t& gmail_byte_size);
		void set_client_gmail(const std::string& gmail);
		void set_client_password_byte_size(const uint8_t& password_byte_size);
		void set_client_password(const std::string& password);


		//setter

		
	};

private:
	MT001Data mt001_data;

public:
    MT001Receiver();
    ~MT001Receiver();
};

