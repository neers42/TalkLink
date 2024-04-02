#ifndef FIREBASEAUTHHANDLER
#define FIREBASEAUTHHANDLER

#include "firebase/app.h"
#include "firebase/auth.h"
#include "firebase/future.h"
#include "AppException.h"
#include "StringFormat.h"
#include "ServerTime.h"


class FirebaseAuthHandler
{
private:
	firebase::App* app;
	firebase::auth::Auth* auth;
	void initialize_firebase();
	void create_user_with_email_and_password(const std::string& email, const std::string& password);
	void sign_in_with_email_and_password(const std::string& email, const std::string& password);

public:
	FirebaseAuthHandler();
	~FirebaseAuthHandler();

	

};
#endif // !FIREBASEAUTHHANDLER



