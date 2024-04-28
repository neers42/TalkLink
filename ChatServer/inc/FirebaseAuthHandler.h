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

public:
	FirebaseAuthHandler();
	~FirebaseAuthHandler();
	
	std::string create_user_with_email_and_password(const std::string& email, const std::string& password);
	std::string sign_in_user_with_email_and_password(const std::string& email, const std::string& password);
	bool sign_out();

	

};
#endif // !FIREBASEAUTHHANDLER



