// //
// // Created by xgc on 2020/10/8.
// //

// #include "thread_pool.h"

// void ThreadPool::run(ThreadPool *thread_pool) {
//   while (true) {
//     std::unique_lock<std::mutex> locker(thread_pool->mtx);
//     while (!thread_pool->stop && thread_pool->tasks.empty()) {
//       thread_pool->cond.wait(locker);
//     }
//     if (thread_pool->stop && thread_pool->tasks.empty()) {
//       return;
//     }
//     if (!thread_pool->tasks.empty()) {
//       Task task = thread_pool->tasks.front();
//       thread_pool->tasks.pop();
//       locker.unlock();
//       task();
//     }
//   }
// }

// ThreadPool::ThreadPool(int thread_num) : stop(false), threads(thread_num) {
//   for (int i = 0; i < thread_num; ++i) {
//     threads[i] = std::thread(run, this);
//   }
// }

// ThreadPool::~ThreadPool() {
//   std::unique_lock<std::mutex> locker(mtx);
//   stop = true;
//   locker.unlock();
//   cond.notify_all();
//   for (size_t i = 0; i < threads.size(); ++i) {
//     threads.at(i).join();
//   }
// }

// bool ThreadPool::pushJob(const Task &task) {
//   std::unique_lock<std::mutex> locker(mtx);
//   if (!stop) {
//     tasks.push(task);
//     locker.unlock();
//     cond.notify_one();
//     return true;
//   }
//   return false;
// }