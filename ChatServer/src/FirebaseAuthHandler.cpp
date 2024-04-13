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

std::string FirebaseAuthHandler::create_user_with_email_and_password(const std::string& email, const std::string& password)
{
	//TODO
	//create_resultの結果を利用してエラー処理を記載する
	firebase::Future<firebase::auth::AuthResult> create_result = this->auth->CreateUserWithEmailAndPassword(email.c_str(), password.c_str());
	firebase::Future<firebase::auth::AuthResult> result;
	std::string user_id = "";

	while (result.status() != firebase::kFutureStatusComplete) {
		result = auth->CreateUserWithEmailAndPasswordLastResult();
		if (result.status() == firebase::kFutureStatusComplete) {
			if (result.error() == firebase::auth::kAuthErrorNone) {
				const firebase::auth::AuthResult auth_result = *result.result();
				std::cout << StringFormat("%s[INFO]Create user succeeded for email(UID : %s) : %s",
					ServerTime::get_current_time(), auth_result.user.uid().c_str(), auth_result.user.email().c_str()) << std::endl;
				user_id = auth_result.user.uid();
			}
			else {
				std::cerr <<StringFormat("%s\033[31m[ERROR]\033[mCreated user failed with error : %s", 
					ServerTime::get_current_time(), result.error_message()) << std::endl;
			}
		}
	}

	return user_id;
}

void FirebaseAuthHandler::create_user_callback(const firebase::Future<firebase::auth::AuthResult*>& result, void* user_data)
{
	//if (result.error() == firebase::auth::kAuthErrorNone) {
	//	// ユーザーの作成に成功した場合、UIDを取得
	//	firebase::auth::User* user = *result.result();
	//	std::string uid = user->uid();
	//	std::cout << "UID: " << uid << std::endl;
	//}
	//else {
	//	// ユーザーの作成に失敗した場合、エラーを表示
	//	std::cout << "Error creating user: " << result.error_message() << std::endl;
	//}
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
