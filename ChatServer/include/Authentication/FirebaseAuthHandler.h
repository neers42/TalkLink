#ifndef FIREBASEAUTHHANDLER_H_
#define FIREBASEAUTHHANDLER_H_
#ifdef _WIN32
#include "Authentication/firebase_windows/firebase/app.h"
#include "Authentication/firebase_windows/firebase/auth.h"
#include "Authentication/firebase_windows/firebase/log.h"
#include "Authentication/firebase_windows/firebase/future.h"
#else
#include "Authentication/firebase_linux/firebase/app.h"
#include "Authentication/firebase_linux/firebase/auth.h"
#include "Authentication/firebase_linux/firebase/log.h"
#include "Authentication/firebase_linux/firebase/future.h"
#endif // _WIN32
#include "AppException.h"
#include "StringFormat.h"
#include "ServerTime.h"
#include "Utils/common.h"
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



