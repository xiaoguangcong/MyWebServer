#pragma once

#include <cstddef>
#include <cstdlib>
#include <string>
#include <sys/types.h>

ssize_t Readn(int fd, void* buff, size_t n);
ssize_t Readn(int fd, std::string &inBuffer, bool &zero);
ssize_t Readn(int fd, std::string &inBuffer);
ssize_t Writen(int fd, void* buff, size_t n);
ssize_t Writen(int fd, std::string &sbuff);
void HandleForSigpipe();
int SetSocketNonBlocking(int fd);
void SetSocketNodelay(int fd);
void SetSocketNoLinger(int fd);
void ShutDownWR(int fd);
int socket_bind_listen(int port);