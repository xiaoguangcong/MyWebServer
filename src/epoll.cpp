//
// Created by xgc on 2020/10/8.
//

#include "epoll.h"

#include <iostream>
#include <unistd.h> // unistd.h 中所定义的接口通常都是大量针对系统调用的封装, 如 fork、pipe 以及各种 I/O 原语（read、write、close 等等）

Epoll::Epoll() : epoll_fd(epoll_create(MAXEVENTS)), event_list(MAXEVENTS) {}

Epoll::~Epoll() { close(epoll_fd); }

int Epoll::add(int fd, void *ptr, int events) {
  struct epoll_event event;
  event.data.ptr = ptr;
  event.events = events;
  return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
}

int Epoll::mod(int fd, void *ptr, int events) {
  struct epoll_event event;
  event.data.ptr = ptr;
  event.events = events;
  return epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

int Epoll::del(int fd, void *ptr, int events) {
  struct epoll_event event;
  event.data.ptr = ptr;
  event.events = events;
  return epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event);
}

// Need to fix
int Epoll::wait(int timeouts) {
  int event_nums = epoll_wait(epoll_fd, event_list.data(),
                              static_cast<int>(event_list.size()), timeouts);
  if (event_nums < 0) {
    std::cout << "[Epoll::wait(): events error]" << std::endl;
  }
  return event_nums;
}
