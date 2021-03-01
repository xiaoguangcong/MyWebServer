#include <cassert>
#include <cstddef>

#include "asynclogging.h"
#include "logfile.h"

AsyncLogging::AsyncLogging(std::string logFileName_, int flushInterval)
    : flushInterval_(flushInterval)
    , running_(false)
    , basename_(logFileName_)
    , thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging")
    , mutex_()
    , condition_(mutex_)
    , currentBuffer_(new Buffer)
    , nextBuffer_(new Buffer)
    , buffers_()
    , latch_(1)
{
    assert(logFileName_.size() > 1);
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}

//(前端)所有LOG_*最终都会调用append函数。
void AsyncLogging::append(const char* logline, int len)
{
    MutexLockGuard lock(mutex_);
    // 如果当前buffer还有空间，就添加到当前日志
    if (currentBuffer_->avail() > len)
    {
   
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
        output.flush();
    }
    output.flush();
}