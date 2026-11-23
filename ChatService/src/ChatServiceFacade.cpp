#include "ChatServiceFacade.hpp"
#include "Application/Runtime/ServerBootstrapper.hpp"
#include "Application/Runtime/RequestDispatcher.hpp"

ChatServiceFacade::ChatServiceFacade() : 
	_app_container(AppContainer::get_instance())
{
	// ログレベルの設定
	// NDEBUGはCMakeがRelease/MinSizeRelビルドで自動的に定義するマクロ。
	// Debugビルド(NDEBUG未定義)ではdebug、それ以外(本番相当)ではinfoにする。
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#else
	spdlog::set_level(spdlog::level::info);
#endif // NDEBUG

	////ログのフォーマット設定
	// Use fixed-width level field so info/debug align visually.
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread id : %t] [%^%-5l%$] %v");
	LOG_INFO("[ChatServiceFacade] Starting TalkLink chat service version {} ...", TALKLINK_VERSION);
	
	// AppContainerを初期化
	try {
		_app_container.initialize();
		LOG_INFO("[Facade] ChatServiceFacade initialized successfully with AppContainer");
	}
	catch (const std::exception& e) {
		LOG_ERROR("[Facade] Failed to initialize ChatServiceFacade: {}", e.what());
		exit(1);
	}
}

ChatServiceFacade::~ChatServiceFacade()
{
	// クリーンアップ処理
	try {
		_app_container.cleanup();
		LOG_INFO("[Facade] ChatServiceFacade cleaned up successfully");
	} catch (const std::exception& e) {
		LOG_ERROR("[Facade] Exception during ChatServiceFacade cleanup: {}", e.what());
	}
}

// サーバプログラムの実行
void ChatServiceFacade::run()
{
	LOG_INFO("[Facade] Starting server using ServerBootstrapper");
	
	try {
		// ServerBootstrapperを使用してサーバーを起動
		auto server_bootstrapper = _app_container.get_server_bootstrapper();
		if (!server_bootstrapper->execute_server_startup()) {
			LOG_ERROR("[Facade] Server startup failed - exiting");
			exit(1);
		}
		
		// RequestDispatcherを使用してソケット監視を開始
		auto dispatcher = _app_container.get_request_dispatcher();
		LOG_INFO("[Facade] Starting user request monitoring...");
		
		// ディスクリプション監視スタート
		while (server_bootstrapper->is_system_healthy())
		{
#ifdef USE_POLL
			dispatcher->start_socket_monitoring();
#elif defined(USE_SELECT)
			// TODO: SELECT方式のモニタリング実装
			auto abs_time = std::chrono::system_clock::now() + std::chrono::milliseconds(1000);
			_app_container.get_socket_handler()->initialize_select_param();
			// 従来のSELECTベースの処理は後で実装
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif
		}
		
		LOG_INFO("[Facade] Server monitoring loop ended - shutting down");
		
	} catch (const std::exception& e) {
		LOG_ERROR("[Facade] Exception in server run loop: {}", e.what());
		exit(1);
	}
}
