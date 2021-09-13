#ifndef THREAD_POOL_H
#define THREAD_POOL_H

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

// refer to http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2709.html
template<class F, class... Args>
class Task {
public:
    explicit Task(F&& f);
    explicit Task(F&& f, Args&&... args);
    Task(const Task& task) = delete;
     Task(Task&& task) = delete;
    ~Task();
    auto operator()(Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
private:
    // Task
    F f;

    // Threadpool
    int max_worker;
    std::vector<std::thread> workers;
    void start_threadpool();
    std::queue<std::function<void()>> tasks;

    // Synchronization
    bool stop;
    std::mutex worker_mutex;
    std::condition_variable worker_condition;
};

// template<class F, class... Args>
// Task<F, Args...>::Task(Task&& task) {
//     std::cout<<"Task move constructors"<<std::endl;
//     // this->f = std::move(task.f);
//     // this->max_worker = task.max_worker;
//     // this->stop = false;
//     // start_threadpool();
// }

// template<class F, class... Args>
// Task<F, Args...>::Task(const Task& task) {
//     std::cout<<"Task copy constructors"<<std::endl;
//     // The copy object will share the thread pool with origin object
//     this->f = task.f;
//     this->max_worker = 1;
    
//     this->stop = task.stop;
// }

template<class F, class... Args>
Task<F, Args...>::Task(F&& f) {
    std::cout<<"new Task1"<<std::endl;
    this->f = f;
    this->max_worker = 1;
    this->stop = false;
    start_threadpool();
}

template<class F, class... Args>
Task<F, Args...>::Task(F&& f, Args&&... args) {
    std::cout<<"new Task2"<<std::endl;
    this->f = f;
    this->max_worker = 1;
    this->stop = false;
    start_threadpool();
}

template<class F, class... Args>
Task<F, Args...>::~Task() {
    std::cout<<"delete Task"<<std::endl;
    {
        std::unique_lock<std::mutex> lock(this->worker_mutex);
        this->stop = true;
    }
    this->worker_condition.notify_all();
    for(std::thread &worker: this->workers)
        worker.join();
}

template<class F, class... Args>
inline void Task<F, Args...>::start_threadpool() {
    std::cout<<"start threadpool"<<std::endl;
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

template<class F, class... Args>
auto Task<F, Args...>::operator()(Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    // Leslie: here may have the overhead to recreate the task
    auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(this->f), std::forward<Args>(args)...));
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(this->worker_mutex);
        // don't allow enqueueing after stopping the pool
        if(this->stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");
        this->tasks.emplace([task](){ (*task)(); });
    }
    this->worker_condition.notify_one();
    return res;
    // serial execution
    // (*(this->task))();
}

#endif
