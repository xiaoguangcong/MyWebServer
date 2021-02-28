#pragma once

#include "Condition.h"
#include "CountDownLatch.h"
#include "LogStream.h"
#include "MutexLock.h"
#include "NonCopyable.h"
#include "Thread.h"
#include <memory>
#include <vector>

// AsyncLogging 负责实现多缓冲技术 协调前后端

class AsyncLogging : private NonCopyable
{
public:
    AsyncLogging(const std::string basename, int flushInterval = 2);
    ~AsyncLogging()
    {
        if (running_) stop();
    }
    
    void append(const char* logline, int len);

    void start()
    {
        running_ = true;
        thread_.start();
        latch_.wait();
    }

    void stop()
    {
        running_ = false;
        condition_.notify();
        thread_.join();
    }

private:
    void threadFunc();
    typedef FixedBuffer<kLargeBuffer> Buffer;
    typedef std::vector<std::shared_ptr<Buffer>> BufferVector;
    typedef std::shared_ptr<Buffer> BufferPtr;
    const int flushInterval_;        // 超时时间，在flushInterval_秒内，缓冲区没写满，仍将缓冲区的数据写到文件中
    bool running_;                   // 是否正在运行
    std::string basename_;           // 日志名字
    Thread thread_;                  // 执行该异步日志记录器的线程
    MutexLock mutex_;
    Condition condition_;
    BufferPtr currentBuffer_;        // 当前的缓冲区 
    BufferPtr nextBuffer_;           // 预备缓冲区
    BufferVector buffers_;           // 缓冲区队列，待写入文件  
    CountDownLatch latch_;           // 倒数计数，用于指示什么时候日志记录器才能开始正常工作,用于等待线程启动
};