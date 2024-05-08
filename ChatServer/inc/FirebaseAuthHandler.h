#ifndef FIREBASEAUTHHANDLER
#define FIREBASEAUTHHANDLER

#include "firebase/app.h"
#include "firebase/auth.h"
#include "firebase/future.h"
#include "AppException.h"
#include "StringFormat.h"
#include "ServerTime.h"
#include "common.h"
#include "assert.h"
#include <iostream>


class FirebaseAuthHandler
{
private:
	firebase::App* app;
	firebase::auth::Auth* auth;
	void initialize_firebase();
	void load_initialize_info();

public:
	FirebaseAuthHandler();
	~FirebaseAuthHandler();
	
	std::string create_user_with_email_and_password(const std::string& email, const std::string& password);
	std::string create_user_with_email_and_password(const std::string& email, const std::string& password, void (FirebaseAuthHandler::*callback_func)(const firebase::Future<firebase::auth::AuthResult>&, std::string*));
	std::string sign_in_user_with_email_and_password(const std::string& email, const std::string& password);
	void sign_out(const std::string& client_id, const std::string& client_email_addr);

	void on_create_callback(const firebase::Future<firebase::auth::AuthResult>& result, std::string* user_id);

};
#endif // !FIREBASEAUTHHANDLER



