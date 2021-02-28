#include "../include/Thread.h"
#include "../include/CurrentThread.h"
#include <cstdio>
#include <pthread.h>
#include <sched.h>
#include <sys/prctl.h>
#include <assert.h>

namespace CurrentThread
{
    __thread int t_cachedTid = 0;
    __thread char t_tidString[32];
    __thread int t_tidStringLength = 6;
    __thread const char* t_threadName = "default";
}

pid_t gettid()
{
    // syscall - 间接系统调用
    // SYS_gettid - 获得该线程的真实pid，称为tid
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

void CurrentThread::cacheTid()
{
    if (t_cachedTid == 0)
    {
        t_cachedTid = gettid();
        // C 库函数 int snprintf(char *str, size_t size, const char *format, ...) 
        // 设将可变参数(...)按照 format 格式化成字符串，并将字符串复制到 str 中，size 为要写入的字符的最大数目，超过 size 会被截断。
        t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d", t_cachedTid);
    }
}

// 为了在线程中保留name tid等数据
struct ThreadData
{
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;
    std::string name_;
    pid_t* tid_;
    CountDownLatch* latch_;

    ThreadData(const ThreadFunc& func, const std::string& name, pid_t* tid,
                CountDownLatch* latch)
        : func_(func), name_(name), tid_(tid), latch_(latch)
    {}

    // 运行线程
    void runInThread()
    {
        *tid_ = CurrentThread::tid();
        tid_ = nullptr;
        latch_->countDown();
        latch_ = nullptr;

        CurrentThread::t_threadName = name_.empty() ? "Thread" : name_.c_str();
        /*
            函数用法：int 用法:prctl(PR_SET_NAME , (char*)name);
            PR_SET_NAME表示给线程命名，第二个参数是进程名字符串，长度至多16字节
         */
        prctl(PR_SET_NAME, CurrentThread::t_threadName);

        func_();  // 具体的运行函数
        CurrentThread::t_threadName = "finished";
    }

};

void* startThread(void* obj)
{
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return nullptr;
}

Thread::Thread(const ThreadFunc& func, const std::string& name)
    : started_(false)
    , joined_(false)
    , pthread_id_(0)
    , tid_(0)
    , func_(func)
    , name_(name)
    ,latch_(1)
{
    setDefaultName();
}

Thread::~Thread()
{
    if(started_ && !joined_)
    {
        pthread_detach(pthread_id_);
    }
}

void Thread::setDefaultName()
{
    if (name_.empty())
    {
        char buf[32];
        snprintf(buf, sizeof buf, "Thread");
        name_ = buf;
    }
}

void Thread::start()  //线程启动函数,调用pthread_create创建线程
{
    assert(!started_);
    started_ = true;
    ThreadData* data = new ThreadData(func_, name_, &tid_, &latch_); //data存放了线程真正要执行的函数,记为func,线程id,线程name等信息 
    if (pthread_create(&pthread_id_, NULL, &startThread, data))
    {
        started_ = false;
        delete data;
        //LOG_SYSFATAL << "Failed in pthread_create";
    }
    else 
    {
        latch_.wait();
        assert(tid_ > 0);
    }
}

int Thread::join()
{
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthread_id_, NULL);
}

/*
    为什么不能直接在创建线程的时候执行某个类的成员函数？

    答： 因为pthread_create的线程函数定义为void *func(void*)，无法将non-staic成员函数传递给pthread_create。
 */


