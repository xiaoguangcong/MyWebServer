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
        currentBuffer_->append(logline, len);
    }
    else // 当前buffer已满
    {
        // 把当前buffer添加到buffer列表中 
        buffers_.push_back(currentBuffer_);
        currentBuffer_.reset();

        // 重新设置当前buffer
        if (nextBuffer_)
        {
            currentBuffer_ = std::move(nextBuffer_);
        }
        else
        {
            //如果前端写入速度太快了，一下子把两块缓冲都用完了，那么只好分配一块新的buffer,作当前缓冲，这是极少发生的情况
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(logline, len);

        // 通知日志线程，有数据可写  
        // 只有当缓冲区满了之后才会将数据写入日志文件中
        condition_.notify();
    }
}

// (后端)线程调用的函数，主要用于周期性的flush数据到日志文件中
void AsyncLogging::threadFunc()
{
    assert(running_ == true);
    latch_.countDown();
    LogFile output(basename_);
    BufferPtr newBuffer1(new Buffer);   // 这两个是后台线程的buffer
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;        // 用来和前台线程的buffers_进行swap.
    buffersToWrite.reserve(16);
    while(running_)
    {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        {
            MutexLockGuard lock(mutex_);
            // 如果buffer为空，那么表示没有数据需要写入文件，那么就等待指定的时间（注意这里没有用倒数计数器）
            if (buffers_.empty())
            {
                condition_.waitForSeconds(flushInterval_);
            }

            // 无论condition是因何而醒来，都要将currentBuffer_放到buffers_中。  
            // 如果是因为时间到而醒，那么currentBuffer_还没满，此时也要将之写入LogFile中。  
            // 如果已经有一个前台buffer满了，那么在前台线程中就已经把一个前台buffer放到buffers_中了。
            // 此时，还是需要把currentBuffer_放到buffers_中（注意，前后放置是不同的buffer，  
            // 因为在前台线程中，currentBuffer_已经被换成nextBuffer_指向的buffer了） 
            buffers_.push_back(currentBuffer_);
            currentBuffer_.reset();

            /*---归还一个buffer---*/ 
            // 将新的buffer转成当前缓冲区  
            currentBuffer_ = std::move(newBuffer1);
            // buffers_和buffersToWrite交换数据，此时buffers_所有的数据存放在buffersToWrite，而buffers_变为空
            buffersToWrite.swap(buffers_);
            if (!nextBuffer_)
            {
                /*-----假如需要，归还第二个----*/ 
                nextBuffer_ = std::move(newBuffer2);
            }
        }

        assert(!buffersToWrite.empty());

        // 如果将要写入文件的buffer列表中buffer的个数大于25，那么将多余数据删除  
        // 消息堆积
        //前端陷入死循环，拼命发送日志消息，超过后端的处理能力
        //这是典型的生产速度超过消费速度，会造成数据在内存中的堆积
        //严重时引发性能问题(可用内存不足),
        //或程序崩溃(分配内存失败)
        if (buffersToWrite.size() > 25)
        {
            buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
        }

        // 将buffersToWrite的数据写入到日志中
        for (size_t i = 0; i < buffersToWrite.size(); ++i)
        {
            output.append(buffersToWrite[i]->data(), buffersToWrite[i]->length());
        }

        // 重新调整buffersToWrite的大小
        if (buffersToWrite.size() > 2)
        {
            // drop non-bzero-ed buffers, avoid trashing
            // 仅保留两个buffer,用于newBuffer1和newBuffer2
            buffersToWrite.resize(2);
        }

        if (!newBuffer1)
        {
            assert(!buffersToWrite.empty());
            // 从buffersToWrite中弹出一个作为newBuffer1
            newBuffer1 = buffersToWrite.back();
            buffersToWrite.pop_back();
            // 清理newBuffer1
            newBuffer1->reset();
        }

         //前台buffer是由newBuffer1 2 归还的。现在把buffersToWrite的buffer归还给后台buffer
        if (!newBuffer2)
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = buffersToWrite.back();
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }
        buffersToWrite.clear();
        output.flush();
    }
    output.flush();
}