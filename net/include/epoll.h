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
  void Add(ChannelPtr request, int timeout);
  void Mod(ChannelPtr request, int timeout);
  void Del(ChannelPtr request);
  // int wait(int timeouts);
  std::vector<ChannelPtr> Poll();
  std::vector<ChannelPtr> GetEventRequest(int events_num);
  void AddTimer(ChannelPtr request_data, int timeout);
  int GetEpollFd() { return epoll_fd_; };
  void HandleExpired();

private:
  int epoll_fd_;
  std::vector<epoll_event> events_;
  ChannelPtr fd2channel_[MAXEVENTS];
  std::shared_ptr<HttpData> fd2http_[MAXEVENTS];
  TimerManager timer_manager_;
};


