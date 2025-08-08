#ifndef THREADPOOLHANDLER_HPP
#define THREADPOOLHANDLER_HPP
#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <string>
#include "LogHandler.hpp"
#include "spdlog/spdlog.h"


class ThreadPoolHandler
{
private:
	//変数
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;

	std::mutex queue_mutex;
	std::condition_variable condition;
	bool stop;

	//関数
	void process_task();
	

public:

	explicit ThreadPoolHandler(size_t threads);
	~ThreadPoolHandler();


	template<class F, class... Args>
	void enqueue_task(F&& f, Args&&... args) {
		try
		{
			auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

			{
				//tasksにtaskを追加するため、mutexをlockする。中かっこを抜けるとmutexのロックは外れる
				std::unique_lock<std::mutex> lock(queue_mutex);

				if (stop) {
					throw std::runtime_error("Enqueue on stopped ThreadPool.");
				}

				//taskを追加する。
				tasks.emplace(task);

			}
		}
		catch (const std::runtime_error& e)
		{
			LOG_ERROR("ThreadPoolHandler::enqueue_task: %s", e.what());
		}
		

		//新しいタスクが追加されたことをスレッドに通知する
		condition.notify_one();
	}


};


#endif // THREADPOOLHANDLER_HPP


