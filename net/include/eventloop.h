#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <sched.h>
#include <vector>

#include "../include/epoll.h"
#include "../../base/include/mutexlock.h"
#include "../../base/include/current_thread.h"
#include "../include/util.h"
#include "channel.h"

class EventLoop
{
public:
    typedef std::function<void()> Functor;
    EventLoop();
    ~EventLoop();

    void Loop();
    void Quit();
    void RunInLoop(Functor&& cb);
    void QueueInLoop(Functor&& cb);
    bool IsInLoopThread() const { return thread_id_ == CurrentThread::tid(); }
    void AssertInLoopThread() { assert(IsInLoopThread()); }
    void ShutDown(std::shared_ptr<Channel> channel) { ShutDownWR(channel->GetFd()); }
    void RemoveFromEpoller(std::shared_ptr<Channel> channel)
    {
        epoller_->del(channel);
    }
    void UpdateEpoller(std::shared_ptr<Channel> channel, int timeout = 0)
    {
        epoller_->mod(channel, timeout);
    }
    void AddToEpoller(std::shared_ptr<Channel> channel, int timeout = 0)
    {
        epoller_->add(channel, timeout);
    }

private:
    bool looping_;
    std::shared_ptr<Epoll> epoller_;
    int wakeup_fd_;
    bool quit_;
    bool event_handling_;
    mutable MutexLock mutex_;

    std::vector<Functor> pending_functors_;
    const pid_t thread_id_;
    std::shared_ptr<Channel> wakeup_channel_ptr_;

    void Wakeup();
    void HandleRead();
    void DoPendingFunctors();
    void HandleConn();
};