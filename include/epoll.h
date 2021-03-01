//
// Created by xgc on 2020/10/8.
//

#pragma once

#include "channel.h"
#include "timer.h"
#include <memory>
#include <sys/epoll.h>
#include <vector>


static const int MAXEVENTS = 1024;

class Epoll {
public:
  Epoll();
  ~Epoll();
  int add(SP_Channel request, int timeout);
  int mod(SP_Channel request, int timeout);
  int del(SP_Channel request);
  // int wait(int timeouts);
  std::vector<std::shared_ptr<Channel>> poll();
  std::vector<std::shared_ptr<Channel>> GetEventRequest(int events_num);
  void AddTimer(std::shared_ptr<Channel> request_data, int timeout);
  int GetEpollFd() { return epoll_fd_; };
  void HandleExpired();

private:
  int epoll_fd_;
  std::vector<epoll_event> events_;
  std::shared_ptr<Channel> fd2channel_[MAXEVENTS];
  std::shared_ptr<HttpData> fd2http_[MAXEVENTS];
  TimeManager timer_manager_;
};


