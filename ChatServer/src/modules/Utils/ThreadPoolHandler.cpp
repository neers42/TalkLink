#include "ThreadPoolHandler.h"

ThreadPoolHandler::ThreadPoolHandler(size_t threads) : stop(false)
{
    for (size_t i = 0; i < threads; ++i) {
        workers.emplace_back([this] {process_task(); });
    }
    LogHandler::get_instance().logging_info(StringFormat("thread, success, server replicating threads[%d].", threads));
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

void ThreadPoolHandler::monitor_worker()
{
    LogHandler::get_instance().logging_debug("monitoring tasks.");
    while (true) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (!tasks.empty()) {
                condition.notify_one();
            }
        }
        
    }
}


