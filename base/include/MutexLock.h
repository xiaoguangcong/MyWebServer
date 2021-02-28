#pragma once

#include <pthread.h>
#include <cstdio>
#include "NonCopyable.h"

class MutexLock : private NonCopyable
{
public:
    MutexLock() 
    {
        pthread_mutex_init(&mutex, nullptr);
    }

    ~MutexLock()
    {
        pthread_mutex_lock(&mutex);
        pthread_mutex_destroy(&mutex);
    }

    void lock()
    {
        pthread_mutex_lock(&mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&mutex);
    }

    pthread_mutex_t* get()
    {
        return &mutex;
    }

private:
    pthread_mutex_t mutex;

private:
    friend class Condition;

};

class MutexLockGuard : private NonCopyable
{
public:
    explicit MutexLockGuard(MutexLock& mutex_) : mutex(mutex_)
    {
        mutex.lock();
    }

    ~MutexLockGuard()
    {
        mutex.unlock();
    }

private:
    MutexLock &mutex;
};