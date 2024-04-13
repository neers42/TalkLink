#ifndef THREADPOOLHANDLER_H_
#define THREADPOOLHANDLER_H_
#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "StringFormat.h"
#include "ServerTime.h"
#include "AppException.h"
#include <string>


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
		catch (const std::runtime_error& re_ex)
		{
			THROW_APP_EXCEPTION(StringFormat("%s", re_ex.what()));
		}
		
		catch (const exception::ExceptionBase& ex) {
			std::cerr << StringFormat("%s\033[31m[ERROR]\033[m%s", ServerTime::get_current_time(), ex.what_for_str());
		}
	

		//新しいタスクが追加されたことをスレッドに通知する
		condition.notify_one();
	}



	void exampleTask() {
		//std::cout << StringFormat("%s[INFO]Processing task by PID : %s", ServerTime::get_current_time(), std::this_thread::get_id() )<< std::endl;
		std::cout << StringFormat("%s[INFO]Processing task by PID ", ServerTime::get_current_time()) << std::endl;
	
	}


};


#endif // THREADPOOLHANDLER_H_


