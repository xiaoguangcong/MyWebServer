#include "../include/channel.h"

Channel::Channel(EventLoop* loop)
    : loop_(loop)
    , events_(0)
    , last_events_(0)
    , fd_(0)
{}

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop)
    , events_(0)
    , last_events_(0)
    , fd_(fd)
{}

Channel::~Channel()
{}

int Channel::GetFd() { return fd_; }

void Channel::SetFd(int fd) { fd_ = fd;}

void Channel::HandleRead()
{
    if (read_handler_)
    {
        read_handler_();
    }
}

void Channel::HandleWrite()
{
    if (write_handler_)
    {
        write_handler_();
    }
}

void Channel::HandleConn()
{
    if (conn_handler_)
    {
        conn_handler_();
    }
}
