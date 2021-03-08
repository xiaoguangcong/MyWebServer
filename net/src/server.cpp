
#include <cstring>
#include <arpa/inet.h>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <functional>

#include "../include/server.h"
#include "../include/util.h"
#include "../include/http_data.h"
#include "../../base/include/logging.h"



Server::Server(EventLoop* loop, int thread_num, int port)
    : loop_(loop)
    , thread_num_(thread_num)
    , eventloopthreadpool_(new EventLoopThreadPool(loop_, thread_num_))
    , start_(false)
    , accept_channel_(new Channel(loop_))
    , port_(port)
    , listen_fd_(socket_bind_listen(port))   // 完成了socket创建、绑定和监听
{
    accept_channel_->SetFd(listen_fd_);
    HandleForSigpipe();
    if (SetSocketNonBlocking(listen_fd_) < 0)
    {
        perror("set socket non block failed");
        abort();
    }
}

void Server::Start()
{
    eventloopthreadpool_->Start();           // 启动线程池管理器
    // 将监听socket的可读事件注册到EventLoop中
    accept_channel_->SetEvents(EPOLLIN | EPOLLET);   // 采用ET触发模式，在接收数据时， 如果有数据只会通知一次
    accept_channel_->SetReadHandler(std::bind(&Server::HandNewConn, this));
    accept_channel_->SetConnHandler(std::bind(&Server::HandThisConn, this));
    loop_->AddToEpoller(accept_channel_, 0);
    start_ = true;
}

void Server::HandNewConn()
{
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_fd = 0;

    /*
        accept()系统调用主要用在基于连接的套接字类型，比如SOCK_STREAM和SOCK_SEQPACKET。
        它提取出所监听套接字的等待连接队列中第一个连接请求，创建一个新的套接字，并返回指向该套接字的文件描述符。
        新建立的套接字不在监听状态，原来所监听的套接字也不受该系统调用的影响。
     */
    while((accept_fd = accept(listen_fd_, (struct sockaddr *)&client_addr, 
            &client_addr_len)) > 0)
    {
        // 从线程池中获取一个EventLoop（因为EventLoop对应一个线程），这里相当于获取了一个线程
        EventLoop* loop = eventloopthreadpool_->GetNextLoop();
        LOG << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":"
        << ntohs(client_addr.sin_port);
    
    /*
    // TCP的保活机制默认是关闭的
    int optval = 0;
    socklen_t len_optval = 4;
    getsockopt(accept_fd, SOL_SOCKET,  SO_KEEPALIVE, &optval, &len_optval);
    cout << "optval ==" << optval << endl;
    */
    // 限制服务器的最大并发连接数
        if (accept_fd >= MAXFDS)
        {
            close(accept_fd);
            continue;
        }

        // 设为非阻塞模式
        if (SetSocketNonBlocking(accept_fd) < 0)
        {
            LOG << "Set non block failed!";
            return;
        }

        SetSocketNodelay(accept_fd);

        std::shared_ptr<HttpData> req_info(new HttpData(loop, accept_fd));
        req_info->GetChannel()->SetHolder(req_info);
        loop->QueueInLoop(std::bind(&HttpData::NewEvent, req_info));
    }
    accept_channel_->SetEvents(EPOLLIN | EPOLLET);
}