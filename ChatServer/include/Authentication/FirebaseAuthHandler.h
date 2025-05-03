#ifndef FIREBASEAUTHHANDLER_H_
#define FIREBASEAUTHHANDLER_H_

#include "firebase/app.h"
#include "firebase/auth.h"
#include "firebase/log.h"
#include "firebase/future.h"
#include "AppException.h"
#include "StringFormat.h"
#include "ServerTime.h"
#include "common.h"
#include "assert.h"
#include <iostream>
#include "spdlog/spdlog.h"


class FirebaseAuthHandler
{
private:
	firebase::App* app;
	firebase::auth::Auth* auth;
	void initialize_firebase();
	void load_initialize_info();
	void custom_loggin_info(firebase::LogLevel level, const char* message) {
		LogHandler::get_instance().logging_info(message);
	}

public:
	FirebaseAuthHandler();
	~FirebaseAuthHandler();
	struct ClientContext {
		std::string* client_id;
		std::string id;
		std::string mail_addr;
		std::string ip;
		std::string error_msg;
	};
	
	//std::string create_user_with_email_and_password(const std::string& email, const std::string& password);
	std::tuple<std::string, std::string> create_user_with_email_and_password(
		const std::string& mail_addr,
		const std::string& password,
		const std::string& ip,
		void (FirebaseAuthHandler::*callback_func)(const firebase::Future<firebase::auth::AuthResult>&, std::string*, std::string*, const std::string&, const std::string&));

	void create_user_with_email_and_password(
		const std::string& mail_addr,
		const std::string& password,
		const std::string& ip,
		std::string* client_id,
		std::string* error_msg);

	//std::string sign_in_user_with_email_and_password(const std::string& email, const std::string& password);
	void sign_in_user_with_email_and_password(
		const std::string& mail_addr,
		const std::string& password,
		const std::string& ip,
		std::string* client_id,
		std::string* error_msg);
	void sign_out(const std::string& client_id, const std::string& client_mail_addr);

	void on_auth_callback(const firebase::Future<firebase::auth::AuthResult>& result, std::string* client_id, std::string* error_msg, const std::string& mail_addr, const std::string& ip);

};
#endif // !FIREBASEAUTHHANDLER_H_



