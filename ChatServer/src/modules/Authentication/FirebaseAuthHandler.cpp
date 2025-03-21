#include "FirebaseAuthHandler.h"

void FirebaseAuthHandler::initialize_firebase()
{
	//TODO
	//jsonファイルから設定値を読み込んで初期化を実行するようにする
	firebase::AppOptions options = firebase::AppOptions();
	options.set_api_key("***REMOVED***");
	options.set_app_id("1:114823736215:web:b75c724ada8be0401872d2");
	options.set_project_id("mychat-e2542");
	app = firebase::App::Create(options);
	firebase::SetLogLevel(firebase::kLogLevelError);
	this->auth = firebase::auth::Auth::GetAuth(app);
	spdlog::info("server auth, success, server auth setting complete!!.");
}

void FirebaseAuthHandler::load_initialize_info()
{
}



std::string FirebaseAuthHandler::create_user_with_email_and_password(const std::string& mail_addr, const std::string& password, const std::string& ip, void (FirebaseAuthHandler::*callback_func)(const firebase::Future<firebase::auth::AuthResult>&, std::string*, const std::string&, const std::string&))
{
	firebase::Future<firebase::auth::AuthResult> create_result = this->auth->CreateUserWithEmailAndPassword(mail_addr.c_str(), password.c_str());
	firebase::Future<firebase::auth::AuthResult> result;
	std::string user_id = "";

	while (result.status() != firebase::kFutureStatusComplete) {
		result = auth->CreateUserWithEmailAndPasswordLastResult();
		if (result.status() == firebase::kFutureStatusComplete) {
			(this->*callback_func)(result, &user_id, mail_addr, ip);
		}
	}

	return user_id;
}




void FirebaseAuthHandler::sign_in_user_with_email_and_password(const std::string& mail_addr, const std::string& password, const std::string& ip, std::string* client_id)
{
	ClientContext c_con;
	c_con.client_id = new std::string();
	c_con.mail_addr = mail_addr;
	c_con.ip = ip;
	firebase::Future<firebase::auth::AuthResult> sign_in_result = auth->SignInWithEmailAndPassword(mail_addr.c_str(), password.c_str());
	std::string user_id = "";


	// タイムアウトの設定（例: 10秒）
	const auto start_time = std::chrono::steady_clock::now();
	const std::chrono::seconds wait_duration(10);

	sign_in_result.OnCompletion([](const firebase::Future<firebase::auth::AuthResult>& sign_in_result, void* client_data){
		ClientContext* client_context = static_cast<ClientContext*>(client_data);

	
		std::string* tem_id = new std::string();
		client_context->client_id = tem_id;
		client_context->id = "";

	

		if (sign_in_result.status() == firebase::kFutureStatusComplete && sign_in_result.error() == firebase::auth::kAuthErrorNone) {
			// 認証成功ならUIDを取得
			const firebase::auth::AuthResult auth_result = *sign_in_result.result();
			*(client_context->client_id) = auth_result.user.uid();

		}else {
			// エラー処理（ログなど）
			LOG_ERROR("Sign in failed with error : {:s}.", sign_in_result.error_message());
		
		}
		client_context->id = *client_context->client_id;

		// 動的に確保したメモリを解放する
		delete client_context->client_id;
		client_context->client_id = nullptr;
	}, &c_con);

	// 非同期操作の完了を待つためのポーリング
	while (sign_in_result.status() == firebase::kFutureStatusPending) {
		// 状態を待機するために少しスリープ
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		// 経過時間をチェックしてタイムアウト処理
		auto current_time = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time) > wait_duration) {
			LOG_ERROR("Authentication interrupted due to timeout.(Client ip:{:s})", c_con.ip);
			break;

		}
	}
	//認証が成功していた場合は、UIDが格納される。失敗の場合は空文字が格納される。
	*client_id = c_con.id;

}

void FirebaseAuthHandler::sign_out(const std::string& client_id, const std::string& client_mail_addr)
{
	this->auth->SignOut();
	spdlog::info("Sign out succeeded for email : {:s}(UID : {:s}).", client_mail_addr, client_id);
	//LogHandler::logging(LogHandler::LogType::SIGN_OUT, std::cout, StringFormat("Sign out succeeded for email : %s(UID : %s).", client_mail_addr, client_id));
}

void FirebaseAuthHandler::on_auth_callback(const firebase::Future<firebase::auth::AuthResult>& result, std::string* client_id, const std::string& mail_addr, const std::string& ip)
{
	assert(result.status() == firebase::kFutureStatusComplete);
	if (result.error() == firebase::auth::kAuthErrorNone) {
		const firebase::auth::AuthResult auth_result = *result.result();
		//LogHandler::logging(LogHandler::LogType::INFO, std::cout, StringFormat("Sign in succeeded for email : %s(UID : %s).", auth_result.user.email(), auth_result.user.uid()));

		//*user_id = auth_result.user.uid();
		//LogHandler::get_instance().logging_info(StringFormat("%s, issue client id : %s, success, sign in user email(%s) and password.", ip, *user_id, mail_addr));
	}
	else {
		LOG_ERROR("Sign in failed with error : {:s}.", result.error_message());
	}
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
