#include "../include/eventloop.h"
#include "../../base/include/logging.h"
#include "../include/util.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <vector>

__thread EventLoop* t_loop_in_this_thread = 0;

int CreateEventFd()
{
    int event_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (event_fd < 0)
    {
        LOG << "create event fd fail!";
        abort();
    }
    return event_fd;
}

EventLoop::EventLoop()
    : looping_(false)
    , epoller_(new Epoll())
    , wakeup_fd_(CreateEventFd())
    , quit_(false)
    , event_handling_(false)
    , calling_pending_functors_(false)
    , thread_id_(CurrentThread::tid())
    , wakeup_channel_ptr_(new Channel(this, wakeup_fd_))
{
    if (!t_loop_in_this_thread)
    {
        t_loop_in_this_thread = this;
    }

    wakeup_channel_ptr_->SetEvents(EPOLLIN | EPOLLET);
    wakeup_channel_ptr_->SetReadHandler(std::bind(&EventLoop::HandleRead, this));
    wakeup_channel_ptr_->SetConnHandler(std::bind(&EventLoop::HandleConn, this));
    epoller_->Add(wakeup_channel_ptr_, 0);
}

void EventLoop::HandleConn()
{
    UpdateEpoller(wakeup_channel_ptr_, 0);
}

EventLoop::~EventLoop()
{
    close(wakeup_fd_);
    t_loop_in_this_thread = NULL;
}

void EventLoop::Wakeup()
{
    uint64_t one = 1;
    ssize_t n = Writen(wakeup_fd_, (char*)(&one), sizeof(one));
    if (n != sizeof(one))
    {
        LOG << "EventLoop::Wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::HandleRead()
{
    uint64_t one = 1;
    ssize_t n = Readn(wakeup_fd_, &one, sizeof(one));
    if (n != sizeof one)
    {
        LOG << "EventLoop::HandleRead() reads " << n << " bytes instead of 8";
    }
    wakeup_channel_ptr_->SetEvents(EPOLLIN | EPOLLET);
}

void EventLoop::RunInLoop(Functor &&cb)
{
    if (IsInLoopThread()) {
        cb();
    }
    else {
        QueueInLoop(std::move(cb));
    }
}

void EventLoop::QueueInLoop(Functor &&cb)
{
    {
        MutexLockGuard lock(mutex_);
        pending_functors_.emplace_back(std::move(cb));
    }

    if (!IsInLoopThread() || calling_pending_functors_)
    {
        Wakeup();
    }
}

void EventLoop::Loop()
{
    assert(!looping_);
    assert(IsInLoopThread());
    looping_ = true;
    quit_ = false;
    std::vector<ChannelPtr> ret;
    while(!quit_)
    {
        ret.clear();
        ret = epoller_->Poll();
        event_handling_ = true;
        for (auto& it : ret)
        {
            it->HandleEvents();
        }
        event_handling_ = false;
        DoPendingFunctors();
        epoller_->HandleExpired();
    }
    looping_ = false;
}

void EventLoop::DoPendingFunctors()
{
    std::vector<Functor> functors;
    calling_pending_functors_ = true;

    {
        MutexLockGuard lock(mutex_);
        functors.swap(pending_functors_);
    }

    for (size_t i = 0; i < functors.size(); ++i)
    {
        functors[i]();
    }
    calling_pending_functors_ = true;
}

void EventLoop::Quit()
{
    quit_ = true;
    if (!IsInLoopThread())
    {
        Wakeup();
    }
}




