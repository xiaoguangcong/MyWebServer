#pragma once

#include <cstddef>
#include<cstring>
#include <string>

#include "noncopyable.h"

/*
    LogStream.h定义了Buffer类和LogStream类。
    1. Buffer类是缓存，日志先缓存到Buffer中，之后再输出。
    2. LogStream类包含buffer，主要实现各种类型数据到字符串的转换，并把数据添加到buffer中。
    添加时使用append()函数，如果日志太长，将无法添加，但是每条日志输出都是构建了一个匿名类，不存在日志叠加到一个缓存的问题。
 */

class AsyncLogging;
const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;


/*
    FixedBuffer的实现为一个非类型参数的模板类（对于非类型参数而言，目前C++的标准仅支持整型、枚举、指针类型和引用类型）
    传入一个非类型参数SIZE表示缓冲区的大小。通过成员 data_首地址、cur_指针、end()完成对缓冲区的各项操作。
    通过append()接口把日志内容添加到缓冲区来。

 */ 

template<int SIZE>
class FixedBuffer : private NonCopyable
{
public:
    FixedBuffer() : cur_(data_) {}

    ~FixedBuffer() {}

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


class LogStream : private NonCopyable
{
public:
    typedef FixedBuffer<kSmallBuffer> Buffer;

    LogStream& operator<<(bool v)
    {
        buffer_.append(v ? "1" : "0", 1);
        return *this;
    }

    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long);
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);

    LogStream& operator<<(const void*);

    LogStream& operator<<(float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }
    
    LogStream& operator<<(double);
    LogStream& operator<<(long double);

    LogStream& operator<<(char v)
    {
        buffer_.append(&v, 1);
    }
    
    LogStream& operator<<(const char* c)
    {
        if (c)
        {
            buffer_.append(c, strlen(c));
        }
        else {
            buffer_.append("(null)", 6);
        }
        return *this;
    }

    LogStream& operator<<(const unsigned char* str)
    {
        return operator<<(reinterpret_cast<const char*>(str));
    }

    LogStream& operator<<(const std::string& v)
    {
        buffer_.append(v.c_str(), v.size());
        return *this;
    }

    void append(const char* data, int len)
    {
        buffer_.append(data, len);
    }

    const Buffer& buffer() const
    {
        return buffer_;
    }

    void resetBuffer()
    {
        buffer_.reset();
    }

private:
    void staticCheck();

    template<typename T>
    void formatInteger(T);

    Buffer buffer_;

    static const int kMaxNumbericSize = 32;
};