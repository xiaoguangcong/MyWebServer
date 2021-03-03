#pragma once

#include "../include/eventloop.h"
#include "../../base/include/thread.h"
class EventLoopThread : private NonCopyable
{
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* StartLoop();

private:
    void ThreadFunc();
    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    MutexLock mutex_;
    Condition condition_;
};