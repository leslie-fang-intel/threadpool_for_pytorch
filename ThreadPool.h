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
#include <omp.h>

extern unsigned cycles_low1, cycles_high1, cycles_low2, cycles_high2;
extern unsigned cycles_low3, cycles_high3, cycles_low4, cycles_high4;
extern uint64_t timestamp1, timestamp2, timestamp3, timestamp4;

extern std::chrono::time_point<std::chrono::high_resolution_clock> timestamp1_nano;
extern std::chrono::time_point<std::chrono::high_resolution_clock> timestamp2_nano;
extern std::chrono::time_point<std::chrono::high_resolution_clock> timestamp3_nano;
extern std::chrono::time_point<std::chrono::high_resolution_clock> timestamp4_nano;

void _pin_cpu_cores(const std::vector<int32_t> &cpu_core_list);

typedef void* kmp_affinity_mask_t;
typedef void (*kmp_create_affinity_mask_p)(kmp_affinity_mask_t*);
typedef int (*kmp_set_affinity_mask_proc_p)(int, kmp_affinity_mask_t*);
typedef int (*kmp_set_affinity_p)(kmp_affinity_mask_t*);
typedef void (*kmp_destroy_affinity_mask_p)(kmp_affinity_mask_t*);
typedef int (*kmp_get_affinity_p)(kmp_affinity_mask_t*);

class TaskExecutor {
public:
    explicit TaskExecutor(const std::vector<int32_t>& cpu_core_list);
    std::mutex& get_mutex();
    std::condition_variable& get_condition();
    bool is_stop();
    std::queue<std::function<void()>>& get_tasks();
    void stop_executor();
    ~TaskExecutor();
private:
    std::queue<std::function<void()>> tasks;
    std::shared_ptr<std::thread> worker;

    // Synchronization
    bool stop;
    std::mutex worker_mutex;
    std::condition_variable worker_condition;

    // thread_pool
    std::vector<int32_t> cpu_core_list;

    // Put the deleted function in the private.
    TaskExecutor(const TaskExecutor& task_executor) =
        delete; // Not support copy or move construtor.
    TaskExecutor(TaskExecutor&& task_executor) =
        delete; // Not support copy or move construtor.
    TaskExecutor& operator=(const TaskExecutor& task_executor) =
        delete; // Not support copy or move construtor.
    TaskExecutor& operator=(TaskExecutor&& task_executor) =
        delete; // Not support copy or move construtor.
};

// refer to http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2709.html
template<class F, class... Args>
class Task {
public:
    explicit Task(F&& f, std::shared_ptr<TaskExecutor> task_executor);
    explicit Task(F&& f, Args&&... args, std::shared_ptr<TaskExecutor> task_executor);
    Task(const Task& task);
    //Task(Task&& task);
    ~Task();
    auto operator()(Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
private:
    // Task
    F f;
    std::shared_ptr<TaskExecutor> task_executor;
};

template <class F, class... Args>
Task<F, Args...>::Task(F&& f, std::shared_ptr<TaskExecutor> task_executor) {
  this->f = f;
  this->task_executor = task_executor;
}

template <class F, class... Args>
Task<F, Args...>::Task(
    F&& f,
    Args&&... args,
    std::shared_ptr<TaskExecutor> task_executor) {
  this->f = f;
  this->task_executor = task_executor;
}

template <class F, class... Args>
Task<F, Args...>::Task(const Task& task) {
  this->f = task.f;
  this->task_executor = task.task_executor;
}

template <class F, class... Args>
Task<F, Args...>::~Task() {}

template<class F, class... Args>
auto Task<F, Args...>::operator()(Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    // Leslie: here may have the overhead to recreate the task
    auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(this->f), std::forward<Args>(args)...));
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(this->task_executor->get_mutex());
        // submit task to a stopping the pool is not allowed
        if(this->task_executor->is_stop())
            throw std::runtime_error("enqueue on stopped ThreadPool");
        this->task_executor->get_tasks().emplace([task](){ (*task)(); });
    }

    this->task_executor->get_condition().notify_one();
    return res;
    // serial execution
    // (*(this->task))();
}

#endif
