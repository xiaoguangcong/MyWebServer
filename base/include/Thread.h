#pragma once

#include <pthread.h>
#include <sched.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <functional>
#include <memory>
#include <string>

#include "CountDownLatch.h"
#include "NonCopyable.h"

class Thread : private NonCopyable
{
public:
    typedef std::function<void()> ThreadFunc;
    explicit Thread(const ThreadFunc&, const std::string& name = std::string());
    ~Thread();

    void start();
    int join();
    bool started() const { return started_; }
    pid_t tid() const { return tid_; }
    const std::string& name() const {return name_; }

private:
    void setDefaultName();
    bool started_;
    bool joined_;
    pthread_t pthread_id_;    // 用于声明线程ID
    pid_t tid_;               // 表示进程id类型
    ThreadFunc func_;
    std::string name_;
    CountDownLatch latch_;
};

/*
    Thread既有pthread_t也有pid_t，它们各有用处，pthread_t给pthread_XXX函数使用，而pid_t作为线程标识。
 */