//
// Created by xgc on 2020/10/8.
//

#include "http_connect.h"
#include <iostream>
#include <unistd.h>
#include <cstring>

const std::unordered_set<std::string> HttpConnect::Method = {
    "GET", "POST", "HEAD", "PUT", "DELETE"};

const std::unordered_map<int, std::string> HttpConnect::code_to_message = {
    {200, "OK"}, {400, "Bad Request"}, {403, "Forbidden"}, {404, "NOT FOUND"},
};

const std::unordered_map<std::string, std::string> HttpConnect::suffix_to_type =
    {{".html", "text/html"},
     {".xml", "text/xml"},
     {".xhtml", "application/xhtml+xml"},
     {".txt", "text/plain"},
     {".rtf", "application/rtf"},
     {".pdf", "application/pdf"},
     {".word", "application/nsword"},
     {".png", "image/png"},
     {".gif", "image/gif"},
     {".jpg", "image/jpeg"},
     {".jpeg", "image/jpeg"},
     {".au", "audio/basic"},
     {".mpeg", "video/mpeg"},
     {".mpg", "video/mpeg"},
     {".avi", "video/x-msvideo"},
     {".gz", "application/x-gzip"},
     {".tar", "application/x-tar"},
     {".css", "text/css"}};

HttpConnect::HttpConnect(int sock_fd, int time_out, std::string root)
    : working(false), socket_fd(sock_fd), bytes_have_read(0),
      read_buffer(new char[BUFFER_SIZE_FOR_READ]()), timeout(time_out),
      timer(nullptr), basic_path(root), state(ExpectRequestLine),
      version(UNKNOWN) {}

HttpConnect::~HttpConnect() {
  close(socket_fd);
  if (read_buffer != nullptr) {
    delete[] read_buffer;
    read_buffer = nullptr;
  }
}

//读入数据,断开连接返回0，读入成功返回1，没读入字符连接未断返回-1
//由于ET一次性读完
HttpConnect::RetCode HttpConnect::readSock() {
  int read_size = 0;
  char *buff = read_buffer;
  while (true) {
    read_size = recv(socket_fd, buff, BUFFER_SIZE_FOR_READ - read_size, 0);
    if (read_size == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
        break;
    }
    if (read_size == 0)
      return NOTCONNECTED;
    buff = buff + read_size;
    bytes_have_read += read_size;
  }
  return buff == read_buffer ? FAIL : SUCCESS;
}
//写出数据
HttpConnect::RetCode HttpConnect::writeSock() {
  if (write_buffers.empty()) {
    std::cout << "[HttpConnect::WriteSock()]: no data to write" << std::endl;
    return FAIL;
  }

  const char *data = write_buffers.front().data();
  size_t data_size = write_buffers.front().size();
  write_buffers.pop();

  while (data_size) {
    ssize_t nwrite = write(socket_fd, data, data_size);
    if (nwrite == -1) {
      if (errno != EAGAIN) {
        std::cout << "[HttpConnect::WriteSock()]: write error" << std::endl;
        return FAIL;
      }
      break;
    }
    data += nwrite;
    data_size -= nwrite;
  }
  return SUCCESS;
}

int HttpConnect::findCRLF(const char *str, int begin, int end) {
  int index = begin;
  while (index < end && str[index]) {
    if (str[index] == '\r' && index + 1 < end && str[index + 1] == '\n') {
      return index + 1;
    }
    index++;
  }
  return end;
}

//分割字符串
std::vector<std::string> HttpConnect::parse(const char *str, int begin, int end,
                                            char ch) {
  std::vector<std::string> ans;
  int index = begin;
  while (index != end) {
    if (str[index] == ch) {
      ans.emplace_back(std::string(str + begin, str + index));
      begin = index = index + 1;
    } else {
      index++;
    }
  }
  if (begin != end) {
    ans.emplace_back(std::string(str + begin, str + end));
  }
  return ans;
}

//是否保持连接
bool HttpConnect::keepAilive() {
  if (headers.find("Connection") == headers.cend()) {
    return false;
  }
  return headers.at("Connection") == "keep-alive" ||
         (version == HTTP1_1 && headers.at("Connection") != "close");
}

//重置状态
void HttpConnect::reset() {
    method = "";
    file_path = "";
    query = "";
    memset(read_buffer, 0, bytes_have_read);
    bytes_have_read = 0;
    state = ExpectRequestLine;
    version = UNKNOWN;
}