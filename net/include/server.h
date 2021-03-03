#include "channel.h"
#include <memory>
#include <unistd.h>
#pragma once

#include "eventloopthreadpool.h"

class Server
{
public:
    Server(EventLoop* loop, int thread_num, int port);
    ~Server(){};
    EventLoop* GetLoop() const { return loop_; }
    void Start();
    void HandNewConn();
    void HandThisConn() { loop_->UpdateEpoller(accept_channel_); }

private:
    EventLoop* loop_;           // 主线程的EventLoop对象
    int thread_num_;
    std::unique_ptr<EventLoopThreadPool> eventloopthreadpool_;
    bool start_;
    std::shared_ptr<Channel> accept_channel_;
    int port_;
    int listen_fd_;
    static const int MAXFDS = 10000;
};