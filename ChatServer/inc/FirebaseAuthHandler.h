#ifndef FIREBASEAUTHHANDLER
#define FIREBASEAUTHHANDLER

#include "firebase/app.h"
#include "firebase/auth.h"
#include "firebase/future.h"
#include "AppException.h"
#include "StringFormat.h"
#include "ServerTime.h"
#include <iostream>


class FirebaseAuthHandler
{
private:
	firebase::App* app;
	firebase::auth::Auth* auth;
	void initialize_firebase();
	void sign_in_with_email_and_password(const std::string& email, const std::string& password);

public:
	FirebaseAuthHandler();
	~FirebaseAuthHandler();
	bool create_user_with_email_and_password(const std::string& email, const std::string& password);
	void create_user_callback(const firebase::Future<firebase::auth::AuthResult*>& result, void* user_data);

	

};
#endif // !FIREBASEAUTHHANDLER



