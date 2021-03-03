#include <cassert>
#include <functional>

#include "eventloopthread.h"

EventLoopThread::EventLoopThread()
    : loop_(NULL)
    , exiting_(false)
    , thread_(std::bind(&EventLoopThread::ThreadFunc, this), "EventLoopThread")
    , mutex_()
    , condition_(mutex_)
{}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->Quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::StartLoop()
{
    assert(!thread_.started());
    thread_.started();
    {
        MutexLockGuard lock(mutex_);
        // 一直等到ThreadFun在Thread里真正跑起来
        while(loop_ == NULL) condition_.wait();
    }
    return loop_;
} 

void EventLoopThread::ThreadFunc()
{
    EventLoop loop;
    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        condition_.notify();
    }

    loop.Loop();
    loop_ = NULL;
}