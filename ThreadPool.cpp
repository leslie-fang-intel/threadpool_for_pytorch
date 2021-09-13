#include "ThreadPool.h"

ThreadPoolExecutor::ThreadPoolExecutor(int max_worker){
    this->max_worker = max_worker;
    for(size_t i = 0; i<this->max_worker; ++i) {
        workers.emplace_back(
            [this] {
                while(true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->worker_mutex);
                        this->worker_condition.wait(lock,
                            [this]{ return this->stop || !this->tasks.empty(); });
                        if(this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            }
        );
    }
}

std::mutex& ThreadPoolExecutor::get_mutex() {
    return this->worker_mutex;
}

std::condition_variable& ThreadPoolExecutor::get_condition() {
    return this->worker_condition;
}

bool ThreadPoolExecutor::is_stop() {
    return this->stop;
}

std::queue<std::function<void()>>& ThreadPoolExecutor::get_tasks() {
    return this->tasks;
}

ThreadPoolExecutor::~ThreadPoolExecutor() {
    std::cout<<"delete ThreadPoolExecutor"<<std::endl;
    {
        std::unique_lock<std::mutex> lock(this->worker_mutex);
        this->stop = true;
    }
    this->worker_condition.notify_all();
    for(std::thread &worker: this->workers)
        worker.join();
}
