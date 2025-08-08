#include "ThreadPoolHandler.hpp"

ThreadPoolHandler::ThreadPoolHandler(size_t threads) : stop(false)
{
    for (size_t i = 0; i < threads; ++i) {
        workers.emplace_back([this] {process_task(); });
    }
	spdlog::debug("server replicating threads:{:d}.", threads);

}

ThreadPoolHandler::~ThreadPoolHandler()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    //スレッドをすべて起動し、破棄する
    condition.notify_all();
    for (std::thread& worker : workers) {
        worker.join();
    }

}

void ThreadPoolHandler::process_task()
{
    while (true) {
        std::function<void()> task;
        {
            //queue_mutexを獲得して、排他制御する。
            std::unique_lock<std::mutex> lock(queue_mutex);

            //taskが追加されるまで待機する
            condition.wait(lock, [this] { return !tasks.empty(); });

            //tasuksからtaskを取り出す
            task = tasks.front();
            tasks.pop();
        }
        task(); // タスクの実行
    }
}



