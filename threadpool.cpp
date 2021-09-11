#include "threadpool.hpp"

void _work(){

}

ThreadPoolExecutor::ThreadPoolExecutor(int max_worker):max_workers(max_worker) {}

// template <class F, class ReturnType>
// Task<F, ReturnType>::Task(const F&& f):f(std::move(f)){
//     thread_pool(new ThreadPoolExecutor(1));
// }

template <class F>
Task<F>::Task(F f){
    f = f;
    //thread_pool(new ThreadPoolExecutor(1));
}


