#include <cassert>
#include <cstddef>
#include <cstdint>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <vector>

#include "../include/eventloop.h"
#include "../../base/include/logging.h"
#include "../include/util.h"

__thread EventLoop* t_loop_in_this_thread = 0;   // 当前线程运行的EventLoop对象

int CreateEventFd()
{
    // EFD_CLOEXEC  fork子进程时不继承
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

    // 检查当前线程是否已经创建了其他EventLoop对象
    if (t_loop_in_this_thread)
    {
        //LOG << "Another EventLoop " << t_loop_in_this_thread << " exists in this thread " << thread_id_;
    }
    else
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

// 采用了对wakeup_fd_的读写来实现对EventLoop的唤醒
// 在EventLoop建立之后，就创建一个wakeup_fd_，并将其可读事件注册到EventLoop中。
// Wakeup()的过程本质上是对这个wakeup_fd_进行写操作，以触发该wakeup_fd_的可读事件。这样就起到了唤醒EventLoop的作用。
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

// 保证对buffer和socket的写动作是在io线程中进行，使用了一个RunInLoop函数，将该写任务抛给了io线程处理。
// 如果调用时是此EventLoop的运行线程，则直接执行此函数。 否则调用QueueInLoop函数。
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
    // 加锁，然后将该函数放到该EventLoop的pending_functors_队列中。
    {
        MutexLockGuard lock(mutex_);
        pending_functors_.emplace_back(std::move(cb));
    }

    //  判断是否要唤醒EventLoop，如果是则调用Wakeup()唤醒该EventLoop
    if (!IsInLoopThread() || calling_pending_functors_)
    {
        Wakeup();
    }
}

// 主线程的EventLoop对象一般是在客户代码中被创建
// 对应的EventLoop::Loop()也是在客户代码中显示调用
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
        // 轮询，得到发生状态变化的Channel对象的集合
        ret = epoller_->Poll();
        event_handling_ = true;

        // 执行每个Channel对象的状态处理函数
        for (auto& it : ret)
        {
            it->HandleEvents();
        }
        event_handling_ = false;

        // 此函数执行PendingFunctors容器中的函数
        DoPendingFunctors();
        epoller_->HandleExpired();
    }
    looping_ = false;
}

void EventLoop::DoPendingFunctors()
{
    std::vector<Functor> functors;
    calling_pending_functors_ = true;

    /*
        这个作用是pendingFunctors_和functors的内容进行交换，实际上就是此时functors持有了pendingFunctors_的内容，而pendingFunctors_被清空了。
        优点：
        如果不这么做，直接遍历pending_functors_,然后处理对应的函数。这样的话，锁会一直等到所有函数处理完才会被释放。在此期间，queueInLoop将不可用。
        而以上的写法，可以极大减小锁范围，整个锁的持有时间就是swap那一下的时间。待处理函数执行的时候，其他线程还是可以继续调用queueInLoop。

     */
    {
        MutexLockGuard lock(mutex_);
        functors.swap(pending_functors_);
    }

    for (size_t i = 0; i < functors.size(); ++i)
    {
        functors[i]();
    }
    calling_pending_functors_ = false;
}

void EventLoop::Quit()
{
    quit_ = true;
    if (!IsInLoopThread())
    {
        Wakeup();
    }
}




