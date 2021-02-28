//
// Created by xgc on 2020/10/8.
//

#ifndef MYWEBSERVER_THREAD_POOL_H
#define MYWEBSERVER_THREAD_POOL_H

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
  using Task = std::function<void()>;
  ThreadPool(int);
  bool pushJob(const Task &);
  ~ThreadPool();

private:
  static void run(ThreadPool *);

private:
  bool stop;
  std::vector<std::thread> threads;
  std::mutex mtx;
  std::condition_variable cond;
  std::queue<Task> tasks;
};

#endif // MYWEBSERVER_THREAD_POOL_H
