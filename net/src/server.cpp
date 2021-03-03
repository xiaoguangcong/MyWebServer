
#include <cstring>
#include <arpa/inet.h>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <functional>

#include "server.h"
#include "util.h"
#include "http_data.h"
#include "logging.h"

Server::Server(EventLoop* loop, int thread_num, int port)
    : loop_(loop)
    , thread_num_(thread_num)
    , eventloopthreadpool_(new EventLoopThreadPool(loop_, thread_num_))
    , start_(false)
    , accept_channel_(new Channel(loop_))
    , port_(port)
    , listen_fd_(socket_bind_listen(port))
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
    eventloopthreadpool_->Start();
    accept_channel_->SetEvents(EPOLLIN | EPOLLET);
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
    while((accept_fd = accept(listen_fd_, (struct sockaddr *)&client_addr, 
            &client_addr_len)) > 0)
    {
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