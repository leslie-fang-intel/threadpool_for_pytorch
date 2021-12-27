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

unsigned cycles_low1, cycles_high1, cycles_low2, cycles_high2;
unsigned cycles_low3, cycles_high3, cycles_low4, cycles_high4;
uint64_t timestamp1, timestamp2, timestamp3, timestamp4;

int taskfunction(int i) {
	//std::cout << "taskfunction thread num : " << std::this_thread::get_id() << ", arg : " << i << std::endl;
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

at::Tensor taskfunction3(const at::Tensor& input) {
  at::Tensor output;
  output = at::softmax(input, -1);
  return output;
}

at::Tensor taskfunction4(at::Tensor input) {
  at::Tensor output;
  output = at::softmax(input, -1);
  return output;
}

int main(int argc, char ** argv) {
    std::vector<int32_t> cpu_core_list({1});
    std::shared_ptr<TaskExecutor> task_executor =
      std::make_shared<TaskExecutor>(cpu_core_list);

    at::Tensor input_tensor = at::rand({100, 8276});
    Task<at::Tensor (*)(const at::Tensor&), at::Tensor> task(
        taskfunction3, task_executor);
    // Task<at::Tensor (*)(at::Tensor), at::Tensor> task(
    //     taskfunction4, task_executor);
    // Warm up time measurement
    asm volatile ( "CPUID\n\t"
                "RDTSC\n\t"
                "mov %%edx, %0\n\t"
                "mov %%eax, %1\n\t": "=r" (cycles_high1), "=r" (cycles_low1)::"%rax", "%rbx", "%rcx", "%rdx");

    asm volatile ( "CPUID\n\t"
                "RDTSC\n\t"
                "CPUID\n\t"
                "RDTSC\n\t"
                "mov %%edx, %0\n\t"
                "mov %%eax, %1\n\t": "=r" (cycles_high1), "=r" (cycles_low1)::"%rax", "%rbx", "%rcx", "%rdx");

    asm volatile ("CPUID\n\t"
    "RDTSC\n\t"::: "%rax", "%rbx", "%rcx", "%rdx");

    std::this_thread::sleep_for(std::chrono::seconds(1)); // to test time, we need to make sure thread pool finish init

    int iteration = 100;
    float total_submit_time_us = 0.0;
    float total_execution_time_us = 0.0;
    float total_join_time_us = 0.0;
    for(int i=0;i<iteration;i++) {
        input_tensor = at::rand({100, 8276});
        // Measure time1
        // asm volatile ( "CPUID\n\t"
        //         "RDTSC\n\t"
        //         "mov %%edx, %0\n\t"
        //         "mov %%eax, %1\n\t": "=r" (cycles_high1), "=r" (cycles_low1)::"%rax", "%rbx", "%rcx", "%rdx");
        timestamp1_nano = std::chrono::high_resolution_clock::now();

        auto resf = task(std::move(input_tensor));
        //std::cout<<"waiting to get result"<<std::endl;
        auto res = resf.get();

        // Measure time4
        // asm volatile ( "CPUID\n\t"
        //         "RDTSC\n\t"
        //         "mov %%edx, %0\n\t"
        //         "mov %%eax, %1\n\t": "=r" (cycles_high4), "=r" (cycles_low4)::"%rax", "%rbx", "%rcx", "%rdx");
        timestamp4_nano = std::chrono::high_resolution_clock::now();

        // timestamp1 = ( ((uint64_t)cycles_high1 << 32) | cycles_low1 );
        // timestamp2 = ( ((uint64_t)cycles_high2 << 32) | cycles_low2 );
        // timestamp3 = ( ((uint64_t)cycles_high3 << 32) | cycles_low3 );
        // timestamp4 = ( ((uint64_t)cycles_high4 << 32) | cycles_low4 );

        // std::cout<<"submit time clock(timestamp2-timestamp1): "<<timestamp2-timestamp1<<std::endl;
        // std::cout<<"execution time clock(timestamp3-timestamp2): "<<timestamp3-timestamp2<<std::endl;
        // std::cout<<"join time clock(timestamp4-timestamp3): "<<timestamp4-timestamp3<<std::endl;

        float submit_time_us = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                timestamp2_nano - timestamp1_nano).count() / 1000.0;
        float execution_time_us = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                timestamp3_nano - timestamp2_nano).count() / 1000.0;
        float join_time_us = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                timestamp4_nano - timestamp3_nano).count() / 1000.0;
        total_submit_time_us += submit_time_us;
        total_execution_time_us += execution_time_us;
        total_join_time_us += join_time_us;
        std::cout<<"submit_time_us is: "<<submit_time_us<<" us"<<std::endl;
        std::cout<<"execution_time_us is: "<<execution_time_us<<" us"<<std::endl;
        std::cout<<"join_time_us is: "<<join_time_us<<" us"<<std::endl;
    }
    std::cout<<"average submit_time_us is: "<<total_submit_time_us/iteration<<" us"<<std::endl;
    std::cout<<"average execution_time_us is: "<<total_execution_time_us/iteration<<" us"<<std::endl;
    std::cout<<"average join_time_us is: "<<total_join_time_us/iteration<<" us"<<std::endl;

    return 0;

}