#include "FirebaseAuthHandler.h"

void FirebaseAuthHandler::initialize_firebase()
{

	//TODO
	//jsonファイルから設定値を読み込んで初期化を実行するようにする
	std::cout << ServerTime::get_current_time() << "[INFO]Creating Firebase Auth ...";
	firebase::AppOptions options = firebase::AppOptions();
	options.set_api_key("***REMOVED***");
	options.set_app_id("1:114823736215:web:b75c724ada8be0401872d2");
	options.set_project_id("mychat-e2542");
	app = firebase::App::Create(options);
	this->auth = firebase::auth::Auth::GetAuth(app);
	std::cout << "\033[32m [OK] \033[m" << std::endl;
}

void FirebaseAuthHandler::load_initialize_info()
{
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
				LogHandler::logging_info(LogHandler::LogType::INFO, std::cout, StringFormat("Create user succeeded for email : %s(UID : %s)", auth_result.user.email(), auth_result.user.uid()));
				user_id = auth_result.user.uid();
			}
			else {
				LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("Created user failed with error : %s", result.error_message()));
			}
		}
	}

	return user_id;
}


std::string FirebaseAuthHandler::sign_in_user_with_email_and_password(const std::string& email, const std::string& password)
{
	firebase::Future<firebase::auth::AuthResult> sign_in_result = auth->SignInWithEmailAndPassword(email.c_str(), password.c_str());
	firebase::Future<firebase::auth::AuthResult> result;
	std::string user_id = "";

	while (result.status() != firebase::kFutureStatusComplete)
	{
		result = auth->SignInWithEmailAndPasswordLastResult();
		if (result.status() == firebase::kFutureStatusComplete) {
			if (result.error() == firebase::auth::kAuthErrorNone) {
				const firebase::auth::AuthResult auth_result = *result.result();
				LogHandler::logging_info(LogHandler::LogType::INFO, std::cout, StringFormat("Sign in succeeded for email : %s(UID : %s)", auth_result.user.email(), auth_result.user.uid()));
				user_id = auth_result.user.uid();
			}
			else {
				LogHandler::logging_info(LogHandler::LogType::SERVER_ERROR, std::cerr, StringFormat("Sign in failed with error : %s", result.error_message()));
			}
		}
	}

	return user_id;
}

void FirebaseAuthHandler::sign_out(const std::string& client_id, const std::string& client_email_addr)
{
	this->auth->SignOut();
	LogHandler::logging_info(LogHandler::LogType::SIGN_OUT, std::cout, StringFormat("Sign out succeeded for email : %s(UID : %s)", client_email_addr, client_id));
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
