// //
// // Created by xgc on 2020/10/8.
// //

// #ifndef MYWEBSERVER_HTTP_CONNECT_H
// #define MYWEBSERVER_HTTP_CONNECT_H

// #include "timer.h"
// #include <functional>
// #include <memory>
// #include <queue>
// #include <string>
// #include <sys/socket.h>
// #include <unordered_map>
// #include <unordered_set>
// #include <vector>

// static const int BUFFER_SIZE_FOR_READ = 2048;

// class HttpConnect {
// public:
//   // http 处理状态
//   enum HttpProcessState {
//     ExpectRequestLine,
//     ExpectHeaders,
//     ExpectBody,
//     GotAll,
//   };

//   // http 版本
//   enum Version {
//     UNKNOWN,
//     HTTP1_0,
//     HTTP1_1,
//   };

//   // 读写函数返回码
//   enum RetCode { SUCCESS, FAIL, NOTCONNECTED };

// public:
//   HttpConnect(int sock_fd, int timeout = 500, std::string root = "");
//   // 读入数据
//   RetCode readSock();
//   // 处理数据
//   bool processRequest();
//   // 写出
//   RetCode writeSock();
//   // 获取fd
//   int getFd() const { return socket_fd; }
//   //添加响应报文
//   bool addResponse(int code, std::string msg = "");
//   //查看有多少个报文没发
//   size_t checkNumToSend() { return write_buffers.size(); }
//   //查看是否是Keep-Alive
//   bool keepAilive();
//   // work的set/get
//   bool isWorking() { return working; }
//   void setWorking(bool on) { working = on; }
//   //计时器set/get
//   void setTimer(std::shared_ptr<Timer> t) { timer = t; }
//   std::shared_ptr<Timer> getTimer() const { return timer; }
//   //重置状态
//   void reset();
//   ~HttpConnect();

// private:
//   //查找\r\n
//   int findCRLF(const char *str, int begin, int end);
//   //分割字符串
//   std::vector<std::string> parse(const char *str, int begin, int end, char ch);
//   //处理请求行,ProcessRequset调用
//   bool processRequsetLine(int start, int end);
//   //添加成功报文 code:200-299
//   void addSuccessResponse(int code, std::string msg, long file_size);
//   //添加失败报文 code:400-499
//   void addClientFailResponse(int code, std::string msg);

// private:
//   static const std::unordered_set<std::string> Method; //方法集合
//   static const std::unordered_map<int, std::string>
//       code_to_message; //状态码转字符串
//   static const std::unordered_map<std::string, std::string>
//       suffix_to_type; //扩展名转类型

//   bool working; //是否处于工作状态
//   int socket_fd;
//   int bytes_have_read; //读入缓存区大小
//   char *read_buffer;   //读入缓存区
//   int timeout;
//   std::shared_ptr<Timer> timer;

//   std::string basic_path; //资源路径
//   std::string file_path;  //文件路径
//   std::string query;      //参数
//   std::string method;

//   std::unordered_map<std::string, std::string> headers;
//   std::queue<std::string> write_buffers;
//   HttpProcessState state;
//   Version version;
// };

// #endif // MYWEBSERVER_HTTP_CONNECT_H
