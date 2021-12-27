#include "ThreadPool.h"
#include <dlfcn.h>

std::chrono::time_point<std::chrono::high_resolution_clock> timestamp1_nano = std::chrono::high_resolution_clock::now();
std::chrono::time_point<std::chrono::high_resolution_clock> timestamp2_nano = std::chrono::high_resolution_clock::now();
std::chrono::time_point<std::chrono::high_resolution_clock> timestamp3_nano = std::chrono::high_resolution_clock::now();
std::chrono::time_point<std::chrono::high_resolution_clock> timestamp4_nano = std::chrono::high_resolution_clock::now();

kmp_create_affinity_mask_p kmp_create_affinity_mask_ext;
kmp_set_affinity_mask_proc_p kmp_set_affinity_mask_proc_ext;
kmp_set_affinity_p kmp_set_affinity_ext;
kmp_destroy_affinity_mask_p kmp_destroy_affinity_mask_ext;
kmp_get_affinity_p kmp_get_affinity_ext;

std::once_flag
    iomp_symbol_loading_call_once_flag; // call_once_flag to ensure the iomp
                                        // symbol loaded once globally
bool iomp_symbol_loaded{
    false}; // Notice: iomp_symbol_loaded is not thread safe.

void loading_iomp_symbol() {
  void* handle = dlopen(NULL, RTLD_NOW | RTLD_GLOBAL);
  if (handle == NULL || dlsym(handle, "kmp_create_affinity_mask") == NULL ||
      dlsym(handle, "kmp_set_affinity_mask_proc") == NULL ||
      dlsym(handle, "kmp_set_affinity") == NULL ||
      dlsym(handle, "kmp_get_affinity") == NULL ||
      dlsym(handle, "kmp_destroy_affinity_mask") == NULL) {
    iomp_symbol_loaded = false;
    return;
  }

  kmp_create_affinity_mask_ext =
      (kmp_create_affinity_mask_p)dlsym(handle, "kmp_create_affinity_mask");
  kmp_set_affinity_mask_proc_ext =
      (kmp_set_affinity_mask_proc_p)dlsym(handle, "kmp_set_affinity_mask_proc");
  kmp_set_affinity_ext = (kmp_set_affinity_p)dlsym(handle, "kmp_set_affinity");
  kmp_get_affinity_ext = (kmp_get_affinity_p)dlsym(handle, "kmp_get_affinity");
  kmp_destroy_affinity_mask_ext =
      (kmp_destroy_affinity_mask_p)dlsym(handle, "kmp_destroy_affinity_mask");

  iomp_symbol_loaded = true;
  return;
}

bool is_runtime_ext_enabled() {
  std::call_once(iomp_symbol_loading_call_once_flag, loading_iomp_symbol);
  return iomp_symbol_loaded;
}

TaskExecutor::TaskExecutor(const std::vector<int32_t>& cpu_core_list) {
    std::cout<<"inside ThreadPoolExecutor constructor"<<std::endl;
    this->cpu_core_list = cpu_core_list;
    //for(size_t i = 0; i<this->max_worker; ++i) {
        this->worker = std::make_shared<std::thread>(
            [this] {
                _pin_cpu_cores(this->cpu_core_list);
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
                    // Measure time2
                    // asm volatile ( "CPUID\n\t"
                    //         "RDTSC\n\t"
                    //         "mov %%edx, %0\n\t"
                    //         "mov %%eax, %1\n\t": "=r" (cycles_high2), "=r" (cycles_low2)::"%rax", "%rbx", "%rcx", "%rdx");
                    timestamp2_nano = std::chrono::high_resolution_clock::now();
                    task();
                    // Measure time3
                    // asm volatile ( "CPUID\n\t"
                    //         "RDTSC\n\t"
                    //         "mov %%edx, %0\n\t"
                    //         "mov %%eax, %1\n\t": "=r" (cycles_high3), "=r" (cycles_low3)::"%rax", "%rbx", "%rcx", "%rdx");
                    timestamp3_nano = std::chrono::high_resolution_clock::now();
                }
            }
        );
    //}
}

std::mutex& TaskExecutor::get_mutex() {
    return this->worker_mutex;
}

std::condition_variable& TaskExecutor::get_condition() {
    return this->worker_condition;
}

bool TaskExecutor::is_stop() {
    return this->stop;
}

std::queue<std::function<void()>>& TaskExecutor::get_tasks() {
    return this->tasks;
}

void TaskExecutor::stop_executor() {
  bool should_wait_worker_join = false;
  {
    std::unique_lock<std::mutex> lock(this->worker_mutex);
    if (this->stop == false) {
      should_wait_worker_join = true;
      this->stop = true;
    }
  }
  if (should_wait_worker_join) {
    this->worker_condition.notify_all();
    this->worker->join();
  }
  return;
}

TaskExecutor::~TaskExecutor() {
    this->stop_executor();
}

void _pin_cpu_cores(const std::vector<int32_t> &cpu_core_list) {
    if (!is_runtime_ext_enabled()) {
        throw std::runtime_error(
            "Didn't preload IOMP before using the runtime API");
    }
    // Create the OMP thread pool and bind to cores of cpu_pools one by one
    omp_set_num_threads(cpu_core_list.size());
    #pragma omp parallel num_threads(cpu_core_list.size())
    {
        // set the OMP thread affinity
        int thread_id = omp_get_thread_num(); // Suppose the ids are [0, 1, 2, 3] if the len of cpu_core_list is 4
        int phy_core_id = cpu_core_list[thread_id];
        kmp_affinity_mask_t mask;
        kmp_create_affinity_mask_ext(&mask);
        kmp_set_affinity_mask_proc_ext(phy_core_id, &mask);
        kmp_set_affinity_ext(&mask);
        kmp_destroy_affinity_mask_ext(&mask);
    }
    return;
}
