#pragma once

#include <cstddef>
#include <cstdlib>
#include <string>
#include <sys/types.h>

ssize_t readn(int fd, void* buff, size_t n);
ssize_t readn(int fd, std::string &inBuffer, bool &zero);
ssize_t readn(int fd, std::string &inBuffer);
ssize_t writen(int fd, void* buff, size_t n);
ssize_t writen(int fd, std::string &sbuff);
void handleForSigpipe();
int setSocketNonBlocking(int fd);
void setSocketNodelay(int fd);
void setSocketNoLinger(int fd);
void shetDownWR(int fd);
int socket_bind_listen(int port);