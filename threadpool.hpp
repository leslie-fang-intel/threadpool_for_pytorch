#pragma once
#include <queue>
#include <memory>

template <class ReturnType>
class Future {
 public:
  explicit Future();
  ReturnType result();
  void set_result(ReturnType obj);
 private:
};

class ThreadPoolExecutor {
 public:
  explicit ThreadPoolExecutor(int max_workers);
 private:
  int max_workers;
};

// template <class F, class ReturnType>
// class Task {
//  public:
//   explicit Task(const F&& f);
//   // Will return a future Tensor
//   Future<ReturnType> submit();
//  private:
//   F f;
//   std::unique_ptr<ThreadPoolExecutor> thread_pool;
// };

template <class F>
class Task {
 public:
  explicit Task(F f);
  // Will return a future Tensor
  void submit();
 private:
  F f;
  //std::unique_ptr<ThreadPoolExecutor> thread_pool;
};