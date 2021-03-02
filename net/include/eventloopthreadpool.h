#pragma once


#include "../../base/include/noncopyable.h"
#include "../../base/include/logging.h"
#include "channel.h"
#include "eventloopthread.h"
#include <memory>
#include <vector>
class EventLoopThreadPool : private NonCopyable
{
public:
    EventLoopThreadPool(EventLoop* base_loop, int threads_num);

    ~EventLoopThreadPool() { LOG << "~EventLoopThreadPool()"; }

    void Start();

    EventLoop* GetNextLoop();

private:
    EventLoop* base_loop_;
    bool start_;
    int threads_num_;
    int next_;
    std::vector<std::shared_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};