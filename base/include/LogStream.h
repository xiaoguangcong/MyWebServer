#pragma once

#include "NonCopyable.h"
#include <cstddef>
#include<cstring>

class AsyncLogging;
const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template<int SIZE>
class FixBuffer : private NonCopyable
{
public:
    FixBuffer() : cur_(data_) {}

    ~FixBuffer() {}

    void append(const char* buf, size_t len)
    {
        if(avail() > static_cast<int>(len))
        {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    const char* data() const { return data_; }

    int length() const { return static_cast<int>(cur_ - data_); }

    char* current() { return cur_; }

    int avail() const { return static_cast<int>(end() - data_); }

    void add(size_t len) { cur_ += len; } 

    void reset() { cur_ = data_; }

    void bzero() { memset(data_, 0, sizeof data_); }

private:
    const char* end() const { return data_ + sizeof data_; }
    char data_[SIZE];
    char* cur_;
};
