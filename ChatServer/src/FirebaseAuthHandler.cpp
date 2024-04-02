#include "FirebaseAuthHandler.h"

void FirebaseAuthHandler::initialize_firebase()
{
	std::cout << ServerTime::get_current_time() << "[INFO]Creating Firebase Auth ...";
	firebase::AppOptions options = firebase::AppOptions();
	options.set_api_key("***REMOVED***");
	options.set_app_id("1:114823736215:web:b75c724ada8be0401872d2");
	options.set_project_id("mychat-e2542");
	app = firebase::App::Create(options);
	this->auth = firebase::auth::Auth::GetAuth(app);
	std::cout << "\033[32m [OK] \033[m" << std::endl;
}

void FirebaseAuthHandler::create_user_with_email_and_password(const std::string& email, const std::string& password)
{
	firebase::Future<firebase::auth::AuthResult> create_result = this->auth->CreateUserWithEmailAndPassword(email.c_str(), password.c_str());
	
}

void FirebaseAuthHandler::sign_in_with_email_and_password(const std::string& email, const std::string& password)
{
	firebase::Future<firebase::auth::AuthResult> sign_in_result = auth->SignInWithEmailAndPassword(email.c_str(), password.c_str());
	
}


FirebaseAuthHandler::FirebaseAuthHandler()
{
	this->initialize_firebase();
}

FirebaseAuthHandler::~FirebaseAuthHandler()
{
	delete this->app;
	delete this->auth;
}
