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

bool FirebaseAuthHandler::create_user_with_email_and_password(const std::string& email, const std::string& password)
{
	//TODO
	//create_resultの結果を利用してエラー処理を記載する

	//try
	//{
	//	firebase::Future<firebase::auth::AuthResult> create_result = this->auth->CreateUserWithEmailAndPassword(email.c_str(), password.c_str());
	//	firebase::auth::AuthResult auth_result = *create_result.result();
	//	std::cout << StringFormat("Create_success : uid(%s)", auth_result.user.uid()) << std::endl;
	///*	if (create_result.status() == firebase::kFutureStatusComplete) {
	//		if (create_result.error() == firebase::auth::kAuthErrorNone) {
	//			auth_result = *create_result.result();
	//			printf("Create user succeeded for email %s\n",
	//				auth_result.user.email().c_str());
	//		}
	//		else {
	//			printf("Created user failed with error '%s'\n", create_result.error_message());
	//		}
	//	}*/
	//}
	//catch (const std::exception& e)
	//{

	//}
	firebase::Future<firebase::auth::AuthResult> create_result = this->auth->CreateUserWithEmailAndPassword(email.c_str(), password.c_str());
	firebase::Future<firebase::auth::AuthResult> result =
		auth->CreateUserWithEmailAndPasswordLastResult();
	if (result.status() == firebase::kFutureStatusComplete) {
		if (result.error() == firebase::auth::kAuthErrorNone) {
			const firebase::auth::AuthResult auth_result = *result.result();
			std::cout << StringFormat("%s[INFO]Create user succeeded for email(UID : &s) : %s",
				ServerTime::get_current_time(), auth_result.user.uid().c_str(), auth_result.user.email().c_str());
		}
		else {

			printf("Created user failed with error '%s'\n", result.error_message());
		}
	}


	//firebase::Future<firebase::auth::AuthResult> create_result = this->auth->CreateUserWithEmailAndPassword(email.c_str(), password.c_str());
	////const firebase::Future<firebase::auth::User*> result;
	//if (create_result.error() == firebase::auth::kAuthErrorNone) {
	//	firebase::auth::AuthResult* auth_result = *create_result.result();
	//	printf("create user succeeded for email %s\n",
	//		auth_result.user.email().c_str());
	//}else {
	//		printf("created user failed with error '%s'\n", create_result.error_message());
	//}
	////std::cout << "success" << std::endl;
	////std::cout << StringFormat("%s[info]create user succeeded for email %s", ServerTime::get_current_time(), auth_result.user.email()) << std::endl;
	return true;
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
