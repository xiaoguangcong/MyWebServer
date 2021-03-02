#pragma once

#include "timer.h"
#include <functional>
#include <memory>
#include <sys/types.h>
#include <sys/epoll.h>

/*
    Channel类
        channel封装fd(也就是socket)，从创建至销毁只属于一个io eventloop线程
        channel由TcpConnection管理和销毁
        channel对象读写、更新都在一个io线程，读写不需要加锁
    
    Channel的作用和成员变量
        1.  Channel fd 封装类，封装套接字、timefd等fd。
        2.  Channel类一般不单独使用，它常常包含在其他类中（Acceptor、Connector、EventLoop、TimerQueue、TcpConnection）使用。Channel对象生存期由这些类控制。
        3.  Channel类有EventLoop的指针 loop_，通过这个指针可以向EventLoop中添加当前Channel事件。
*/


class EventLoop;
class HttpData;

class Channel
{
private:
    typedef std::function<void()> CallBack;
    EventLoop* loop_;
    int fd_;
    __uint32_t events_;       
    __uint32_t revents_;
    __uint32_t last_events_;

    // 方便找到上层持有该Channel的对象
    std::weak_ptr<HttpData> holder_;

private:
    int ParseURI();
    int ParseHeaders();
    int AnalysisRequest();

    CallBack read_handler_;
    CallBack write_handler_;
    CallBack error_handler_;
    CallBack conn_handler_;

public:
    Channel(EventLoop* loop);
    Channel(EventLoop* loop, int fd);
    ~Channel();

    int GetFd();
    void SetFd(int fd);

    void SetHolder(std::shared_ptr<HttpData> holder)
    {
        holder_ = holder;
    }

    std::shared_ptr<HttpData> GetHolder()
    {
        std::shared_ptr<HttpData> ret(holder_.lock());
        return ret;
    }

    void SetReadHandler(CallBack&& read_handler) 
    { 
        read_handler_ = read_handler; 
    }
    
    void SetWriteHandler(CallBack&& write_handler) 
    {
        write_handler_ = write_handler;
    }
  
    void SetErrorHandler(CallBack&& error_handler) 
    {
        error_handler_ = error_handler;
    }
    
    void SetConnHandler(CallBack&& conn_handler) 
    { 
        conn_handler_ = conn_handler; 
    }

    void HandleRead();
    void HandleWrite();
    void HandleError(int fd, int err_num, std::string short_msg);
    void HandleConn();

    /*
        EPOLLIN ：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
        EPOLLOUT：表示对应的文件描述符可以写；
        EPOLLPRI：表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）；
        EPOLLERR：表示对应的文件描述符发生错误；
        EPOLLHUP：表示对应的文件描述符被挂断；
        EPOLLET： 将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的。
        EPOLLONESHOT：只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里
     */ 

    void HandleEvents()
    {
        events_ = 0;
        if ((revents_ & EPOLLHUP) & !(revents_ & EPOLLIN))  // 判断返回的事件 为挂断 close
        {
            events_ = 0;
            return;
        }
        if (revents_ & EPOLLERR)
        {
            if (error_handler_)
            {
                error_handler_();
            }
            events_ = 0;
            return;
        }
        if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
        {
            HandleRead();
        }
        if (revents_ & EPOLLOUT)
        {
            HandleWrite();
        }
        HandleConn();
    }

    void SetRevents(__uint32_t ev) 
    { 
        revents_ = ev; 
    }

    void SetEvents(__uint32_t ev) 
    { 
        events_ = ev; 
    }
  
    __uint32_t& GetEvents() 
    {
        return events_;
    }
    
    bool EqualAndUpdateLastEvents()
    {
        bool ret = (last_events_ == events_);
        last_events_ = events_;
        return ret;
    }

    __uint32_t GetLastEvents()
    {
        return last_events_;
    }
};

typedef std::shared_ptr<Channel> ChannelPtr;