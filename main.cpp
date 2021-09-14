#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <unistd.h>
#include "ThreadPool.h"

#include <torch/torch.h>
#include <torch/script.h>
#include <c10/util/Exception.h>

#include <malloc.h>

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

at::Tensor taskfunction3(at::Tensor input) {
    at::Tensor output;
    for (size_t i = 0; i < 50000; i++) {
        output = at::softmax(input, -1);
    }
    return input;
}

int main(int argc, char ** argv) {
    // typedef int (*FunType)(int);
    // auto b = Task<FunType, int>(taskfunction, 12);
    //auto b = Task<int (*)(int), int>(taskfunction, 12);
    //Task<int (*)(int), int>(taskfunction);
    //auto b = Task<int (*)(int), int>(taskfunction);

    std::vector<int32_t> cpu_core_list({0, 1, 13, 26, 27});
    std::shared_ptr<ThreadPoolExecutor> thread_pool = std::make_shared<ThreadPoolExecutor>(1, cpu_core_list);
    // Task<int (*)(int), int> b(taskfunction, thread_pool);
    // // auto c = b; // copy constructors
    // // auto d(std::move(b)); // move constructors


    // Task<int (*)(float*, float*), float*, float*> b(taskfunction2, thread_pool);
    // results.emplace_back(b(std::move(input), std::move(output1)));


    std::vector< std::future<at::Tensor> > results;
    //at::Tensor input_tensor = at::rand({32, 3, 224, 224});
    at::Tensor input_tensor = at::rand({100, 8276});
    //std::cout<<compare_result.dtype()<<std::endl;
    Task<at::Tensor (*)(at::Tensor), at::Tensor> b(taskfunction3, thread_pool);
    results.emplace_back(b(std::move(input_tensor)));

    at::Tensor res;
    for(auto && result: results) {
        std::cout<<"waiting to get result"<<std::endl;
        res = result.get();
    }
    //std::cout << res << ' ' << std::endl;
    return 0;

}