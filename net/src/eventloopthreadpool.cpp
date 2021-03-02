#include "../include/eventloopthreadpool.h"
#include <cassert>
#include <memory>

EventLoopThreadPool::EventLoopThreadPool(EventLoop* base_loop, int threads_num)
    : base_loop_(base_loop)
    , start_(false)
    , threads_num_(threads_num)
    , next_(0)
{
    if (threads_num_ <= 0)
    {
        LOG << "The number of threas is less than(equal) zero.";
        abort();
    }
}

void EventLoopThreadPool::Start()
{
    base_loop_->AssertInLoopThread();
    start_ = true;
    for (int i = 0; i < threads_num_; ++i)
    {
        std::shared_ptr<EventLoopThread> t(new EventLoopThread());
        threads_.push_back(t);
        loops_.push_back(t->StartLoop());
    }
}

EventLoop* EventLoopThreadPool::GetNextLoop()
{
    base_loop_->AssertInLoopThread();
    assert(start_);
    EventLoop *loop = base_loop_;
    if (!loops_.empty())
    {
        loop = loops_[next_];
        next_ = (next_ + 1) % threads_num_;
    }
    return loop;
}