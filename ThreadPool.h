#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <iostream>
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <cassert>

class ThreadPoolExecutor {
public:
    explicit ThreadPoolExecutor(int max_worker);
    std::mutex& get_mutex();
    std::condition_variable& get_condition();
    bool is_stop();
    std::queue<std::function<void()>>& get_tasks();
    ~ThreadPoolExecutor();
private:
    int max_worker;
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    // Synchronization
    bool stop;
    std::mutex worker_mutex;
    std::condition_variable worker_condition;
};

// refer to http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2709.html
template<class F, class... Args>
class Task {
public:
    explicit Task(F&& f, std::shared_ptr<ThreadPoolExecutor> thread_pool);
    explicit Task(F&& f, Args&&... args, std::shared_ptr<ThreadPoolExecutor> thread_pool);
    Task(const Task& task);
    Task(Task&& task);
    ~Task();
    auto operator()(Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
private:
    // Task
    F f;
    std::shared_ptr<ThreadPoolExecutor> thread_pool;
};

template<class F, class... Args>
Task<F, Args...>::Task(const Task& task) {
    std::cout<<"Task copy constructor"<<std::endl;
    this->f = task.f;
    this->thread_pool = task.thread_pool;
}

template<class F, class... Args>
Task<F, Args...>::Task(Task&& task) {
    std::cout<<"Task move constructor"<<std::endl;
    this->f = task.f;
    this->thread_pool = task.thread_pool;
}

template<class F, class... Args>
Task<F, Args...>::Task(F&& f, std::shared_ptr<ThreadPoolExecutor> thread_pool) {
    std::cout<<"new Task1"<<std::endl;
    this->f = f;
    this->thread_pool = thread_pool;
}

template<class F, class... Args>
Task<F, Args...>::Task(F&& f, Args&&... args, std::shared_ptr<ThreadPoolExecutor> thread_pool) {
    std::cout<<"new Task2"<<std::endl;
    this->f = f;
    this->thread_pool = thread_pool;
}

template<class F, class... Args>
Task<F, Args...>::~Task() {
    std::cout<<"delete Task"<<std::endl;
}

template<class F, class... Args>
auto Task<F, Args...>::operator()(Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    // Leslie: here may have the overhead to recreate the task
    auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(this->f), std::forward<Args>(args)...));
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(this->thread_pool->get_mutex());
        // submit task to a stopping the pool is not allowed
        if(this->thread_pool->is_stop())
            throw std::runtime_error("enqueue on stopped ThreadPool");
        this->thread_pool->get_tasks().emplace([task](){ (*task)(); });
    }
    this->thread_pool->get_condition().notify_one();
    return res;
    // serial execution
    // (*(this->task))();
}

#endif
