//
// Created by xgc on 2020/10/8.
//

#include "../include/epoll.h"
#include "../include/channel.h"
#include "../../base/include/logging.h"

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sys/epoll.h>
#include <unistd.h> // unistd.h 中所定义的接口通常都是大量针对系统调用的封装, 如 fork、pipe 以及各种 I/O 原语（read、write、close 等等）
#include <vector>

const int static EVENTS_SUM = 4096;
const int EPOLLWAIT_TIME = 10000;

//typedef std::shared_ptr<Channel> ChannelPtr;

Epoll::Epoll() : epoll_fd_(epoll_create1(EPOLL_CLOEXEC)), events_(EVENTS_SUM) 
{
  assert(epoll_fd_ > 0);
}

Epoll::~Epoll() {}

// 注册新描述符
void Epoll::Add(ChannelPtr request, int timeout) {
  int fd = request->GetFd();
  if (timeout > 0)
  {
    AddTimer(request, timeout);
    fd2http_[fd] = request->GetHolder();
  }
  struct epoll_event event;
  event.data.fd = fd;
  event.events = request->GetEvents();
  request->EqualAndUpdateLastEvents();

  fd2channel_[fd] = request;
  if (epoll_ctl(fd, EPOLL_CTL_ADD, fd, &event) < 0)
  {
    perror("epoll add error\n");
    fd2channel_->reset();
  }
}

void Epoll::Mod(ChannelPtr request, int timeout) {
  if (timeout > 0)
  {
    AddTimer(request, timeout);
  }
  int fd = request->GetFd();
  if (!request->EqualAndUpdateLastEvents())
  {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->GetEvents();
    if (epoll_ctl(fd, EPOLL_CTL_MOD, fd, &event) < 0) 
    {
      perror("epoll mod error");
      fd2channel_[fd].reset();
    }
  }
}

void Epoll::Del(ChannelPtr request) {
  int fd = request->GetFd();
  struct epoll_event event;
  event.data.fd = fd;
  event.events = request->GetLastEvents();
  if (epoll_ctl(fd, EPOLL_CTL_DEL, fd, &event) < 0) {
    perror("epoll del error");
  }
  fd2channel_[fd].reset();
  fd2http_[fd].reset();
}

// 返回活跃事件数
std::vector<ChannelPtr> Epoll::Poll()
{
  while (true)
  {
    int event_count = 
      epoll_wait(epoll_fd_, &*events_.begin(), events_.size(), EPOLLWAIT_TIME);
    if (event_count < 0)
    {
      perror("epoll wait error");
    }
    std::vector<ChannelPtr> req_data = GetEventRequest(event_count);
    if (req_data.size() > 0)
    {
      return req_data;
    }
  }
}


void Epoll::HandleExpired()
{
  timer_manager_.HandleExpiredEvent();
}

// 分发处理函数
std::vector<ChannelPtr> Epoll::GetEventRequest(int events_num)
{
  std::vector<ChannelPtr> req_data;
  for(int i = 0; i < events_num; ++i)
  {
    // 获取有事件产生的描述符
    int fd = events_[i].data.fd;

    ChannelPtr cur_req = fd2channel_[fd];

    if (cur_req)
    {
      cur_req->SetRevents(events_[i].events);
      cur_req->SetEvents(0);
      req_data.push_back(cur_req);
    }
    else 
    {
      LOG << "cur_req ptr is invalid";
    }
  }
  return req_data;
}

void Epoll::AddTimer(ChannelPtr request_data, int timeout)
{
  std::shared_ptr<HttpData> http_data = request_data->GetHolder();
  if (http_data)
  {
    timer_manager_.AddTimer(http_data, timeout);
  }
  else
  {
    LOG << "timer add fail";
  }
}