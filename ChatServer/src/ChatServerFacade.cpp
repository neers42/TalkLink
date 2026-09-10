#include "ChatServerFacade.hpp"
#include "Application/Services/StartupService.hpp"
#include "Application/Services/RequestDispatcherService.hpp"

ChatServerFacade::ChatServerFacade() : 
	_service_container(ServiceContainer::get_instance())
{
	// ログレベルの設定
#ifdef DEBUG_MODE
	spdlog::set_level(spdlog::level::debug);
#else
	// ログのレベル設定
	spdlog::set_level(spdlog::level::info);
#endif // DEBUG_MODE

	////ログのフォーマット設定
	// Use fixed-width level field so info/debug align visually.
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread id : %t] [%^%-5l%$] %v");
	LOG_INFO("[Facade] Starting TalkLink chat server version 1.0.0!!");
	
	// ServiceContainerを初期化
	try {
		_service_container.initialize();
		LOG_INFO("[Facade] ChatServerFacade initialized successfully with ServiceContainer");
	}
	catch (const std::exception& e) {
		LOG_ERROR("[Facade] Failed to initialize ChatServerFacade: {}", e.what());
		exit(1);
	}
}

ChatServerFacade::~ChatServerFacade()
{
	// クリーンアップ処理
	try {
		_service_container.cleanup();
		LOG_INFO("[Facade] ChatServerFacade cleaned up successfully");
	} catch (const std::exception& e) {
		LOG_ERROR("[Facade] Exception during ChatServerFacade cleanup: {}", e.what());
	}
}

// サーバプログラムの実行
void ChatServerFacade::run()
{
	LOG_INFO("[Facade] Starting server using StartupService");
	
	try {
		// StartupServiceを使用してサーバーを起動
		auto startup_service = _service_container.get_startup_service();
		if (!startup_service->execute_server_startup()) {
			LOG_ERROR("[Facade] Server startup failed - exiting");
			exit(1);
		}
		
		// RequestDispatcherServiceを使用してソケット監視を開始
		auto dispatcher = _service_container.get_request_dispatcher_service();
		LOG_INFO("[Facade] Starting user request monitoring...");
		
		// ディスクリプション監視スタート
		while (startup_service->is_system_healthy())
		{
#ifdef USE_POLL
			dispatcher->start_socket_monitoring();
#elif defined(USE_SELECT)
			// TODO: SELECT方式のモニタリング実装
			auto abs_time = std::chrono::system_clock::now() + std::chrono::milliseconds(1000);
			_service_container.get_socket_handler()->initialize_select_param();
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
