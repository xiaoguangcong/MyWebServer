#pragma once

#include <cstddef>
#include <cstdio>
#include <string>
#include "NonCopyable.h"

class AppendFile : private NonCopyable
{
public:
    explicit AppendFile(std::string filename);
    ~AppendFile();

    // append 会向文件写
    void append(const char* logline, const size_t len);
    void flush();

private:
    size_t write(const char* logline, size_t len);
    FILE* fp_;   //指向文件的指针
    char buffer_[64 * 1024];  //缓存
};