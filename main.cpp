#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <unistd.h>

std::mutex Queue_Mutex;
std::queue<int> Queue;

std::condition_variable worker_main_cv;
std::condition_variable main_worker_cv;

auto timestamp1 = std::chrono::high_resolution_clock::now();
auto timestamp2 = std::chrono::high_resolution_clock::now();

bool start = false;
int initialized = 0;

int main(int argc, char ** argv){
    std::vector<std::thread> thread_pool;
    const auto thread_id = 0;
    thread_pool.emplace_back([&, thread_id]() {
        {
            std::unique_lock<std::mutex> lock(Queue_Mutex);
            ++initialized;
            worker_main_cv.notify_one();
            // NOLINTNEXTLINE(bugprone-infinite-loop)
            while (!start) {
                main_worker_cv.wait(lock);
            }
        }
        //std::cout<<"Sub thead finish init"<<std::endl;
        while(true){
            if (!Queue.empty()) {
                timestamp2 = std::chrono::high_resolution_clock::now();
                int data = Queue.front();
                //std::cout<<"sub thread get data: "<<data<<std::endl;
                Queue.pop();
                if(data == 100){
                    break;
                }
            }
        }
    });

    {
        std::unique_lock<std::mutex> lock(Queue_Mutex);
        while (initialized != 1) {
            worker_main_cv.wait(lock);
        }
    }
    start = true;
    main_worker_cv.notify_all();
    //std::cout<<"Main thead finish init"<<std::endl;
    usleep(1000);

    timestamp1 = std::chrono::high_resolution_clock::now();
    Queue.push(100);

    for (auto& t : thread_pool) {
        t.join();
    }
    float submit_time_us = std::chrono::duration_cast<std::chrono::nanoseconds>(
                            timestamp2 - timestamp1).count() / 1000.0;
    std::cout<<"submit_time_us is: "<<submit_time_us<<" us"<<std::endl;

    return 0;

}