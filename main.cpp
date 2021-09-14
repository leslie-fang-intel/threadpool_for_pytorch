#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <unistd.h>
#include "ThreadPool.h"

#include <malloc.h>
// std::mutex Queue_Mutex;
// std::queue<int> Queue;

// std::condition_variable worker_main_cv;
// std::condition_variable main_worker_cv;

// auto timestamp1 = std::chrono::high_resolution_clock::now();
// auto timestamp2 = std::chrono::high_resolution_clock::now();

// bool start = false;
// int initialized = 0;

int taskfunction(int i) {
	std::cout << "taskfunction thread num : " << std::this_thread::get_id() << ", arg : " << i << std::endl;
	return i;
}

#define length 8*28*15130*4*3

int taskfunction2(float* input, float* output) {
	std::cout << "taskfunction2 thread num : " << std::this_thread::get_id() << std::endl;

#pragma omp parallel
{
    // __m256 m256_input;
    // __m256 m256_output;
    int64_t num_threads = omp_get_num_threads();
    std::cout<<"num_threads: "<<num_threads<<std::endl;
    int64_t chunk = length/num_threads;
    int64_t tid = omp_get_thread_num();
    int64_t begin = tid * chunk;
    int64_t end = tid * chunk + chunk;
    std::cout<<"begin: "<<begin<<std::endl;
    std::cout<<"end: "<<end<<std::endl;
    // for (int64_t i=begin; i<end; i+=8) {
    //   m256_input = _mm256_loadu_ps(input+i);
    //   for (int64_t j=0; j<10000000; j+=1) {
    //     for (int64_t k=0; k<10000000; k+=1) {
    //         m256_output = _mm256_add_ps(m256_input, m256_input);
    //         m256_output = _mm256_addsub_ps(m256_output, m256_input);
    //         //usleep(10);
    //     }
    //   }
    //   _mm256_storeu_ps(output+i, m256_output);
    // }
}

	return 0;
}

int main(int argc, char ** argv){
    // typedef int (*FunType)(int);
    // auto b = Task<FunType, int>(taskfunction, 12);
    //auto b = Task<int (*)(int), int>(taskfunction, 12);
    //Task<int (*)(int), int>(taskfunction);
    //auto b = Task<int (*)(int), int>(taskfunction);
    size_t align_size = 64;
    float* input = (float*)memalign(align_size, sizeof(float) * length);
    float* output1 = (float*)memalign(align_size, sizeof(float) * length);
    float* output2 = (float*)memalign(align_size, sizeof(float) * length);
    float* output3 = (float*)memalign(align_size, sizeof(float) * length);
    float* output4 = (float*)memalign(align_size, sizeof(float) * length);

    for (size_t i = 0; i < length; i++) {
        input[i] = rand() / (float)RAND_MAX * 100.f - 50.f;
        output1[i] = 0;
        output2[i] = 0;
        output3[i] = 0;
        output4[i] = 0;
    }

    std::vector<int32_t> cpu_core_list({1, 26});

    std::shared_ptr<ThreadPoolExecutor> thread_pool = std::make_shared<ThreadPoolExecutor>(1, cpu_core_list);
    // Task<int (*)(int), int> b(taskfunction, thread_pool);
    // // auto c = b; // copy constructors
    // // auto d(std::move(b)); // move constructors

    std::vector< std::future<int> > results;
    // // // auto resf1 = b(1);
    // // // auto resf2 = b(2);
    // results.emplace_back(b(1));
    // // results.emplace_back(b(input, output2));


    Task<int (*)(float*, float*), float*, float*> b(taskfunction2, thread_pool);
    results.emplace_back(b(std::move(input), std::move(output1)));


    for(auto && result: results)
         std::cout << result.get() << ' ' << std::endl;
    return 0;

    // std::vector<std::thread> thread_pool;
    // // run([&](){std::cout<<"hello world"<<std::endl;});


    // const auto thread_id = 0;
    // thread_pool.emplace_back([&, thread_id]() {
    //     {
    //         std::unique_lock<std::mutex> lock(Queue_Mutex);
    //         ++initialized;
    //         worker_main_cv.notify_one();
    //         // NOLINTNEXTLINE(bugprone-infinite-loop)
    //         while (!start) {
    //             main_worker_cv.wait(lock);
    //         }
    //     }
    //     //std::cout<<"Sub thead finish init"<<std::endl;
    //     while(true){
    //         if (!Queue.empty()) {
    //             timestamp2 = std::chrono::high_resolution_clock::now();
    //             int data = Queue.front();
    //             //std::cout<<"sub thread get data: "<<data<<std::endl;
    //             Queue.pop();
    //             if(data == 100){
    //                 break;
    //             }
    //         }
    //     }
    // });

    // {
    //     std::unique_lock<std::mutex> lock(Queue_Mutex);
    //     while (initialized != 1) {
    //         worker_main_cv.wait(lock);
    //     }
    // }
    // start = true;
    // main_worker_cv.notify_all();
    // //std::cout<<"Main thead finish init"<<std::endl;
    // usleep(1000);

    // timestamp1 = std::chrono::high_resolution_clock::now();
    // Queue.push(100);

    // for (auto& t : thread_pool) {
    //     t.join();
    // }
    // float submit_time_us = std::chrono::duration_cast<std::chrono::nanoseconds>(
    //                         timestamp2 - timestamp1).count() / 1000.0;
    // std::cout<<"submit_time_us is: "<<submit_time_us<<" us"<<std::endl;

    // return 0;

}