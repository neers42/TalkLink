#include "ThreadPoolHandler.h"

ThreadPoolHandler::ThreadPoolHandler(size_t threads) : stop(false)
{
    for (size_t i = 0; i < threads; ++i)
        workers.emplace_back([this] {
        process_task();
       /* for (;;) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(this->queue_mutex);
                this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                if (this->stop && this->tasks.empty())
                    return;
                task = std::move(this->tasks.front());
                this->tasks.pop();
            }

            task();
        }*/
            });
}

ThreadPoolHandler::~ThreadPoolHandler()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }

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
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] { return !tasks.empty(); });
            task = tasks.front();
            tasks.pop();
        }
        task(); // タスクの実行
    }
}


//template<class F, class... Args>
//void enqueue_task(F&& f, Args&&... args) {
//    auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
//
//    {
//        //tasksにtaskを追加するため、mutexをlockする。中かっこを抜けるとmutexのロックは外れる
//        std::unique_lock<std::mutex> lock(queue_mutex);
//
//        if (stop) {
//            throw std::runtime_error("enqueue on stopped ThreadPool");
//        }
//
//        //taskを追加する
//        tasks.emplace(task);
//    }
//    condition.notify_one();
//}

