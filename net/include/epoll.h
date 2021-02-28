//
// Created by xgc on 2020/10/8.
//

#ifndef MYWEBSERVER_EPOLL_H
#define MYWEBSERVER_EPOLL_H

#include <sys/epoll.h>
#include <vector>

static const int MAXEVENTS = 1024;

class Epoll {
public:
  Epoll();
  ~Epoll();
  int add(int fd, void *ptr, int events);
  int mod(int fd, void *ptr, int events);
  int del(int fd, void *ptr, int events);
  int wait(int timeouts);
  epoll_event GetEvent(int index) { return event_list.at(index); }

private:
  int epoll_fd;
  std::vector<struct epoll_event> event_list;
};

#endif // MYWEBSERVER_EPOLL_H
