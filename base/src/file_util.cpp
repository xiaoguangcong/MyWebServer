#include <cstddef>
#include <cstdio>

#include "../include/file_util.h"

AppendFile::AppendFile(std::string filename) : fp_(fopen(filename.c_str(), "ae"))
{
    // 用户提供缓冲区
    setbuffer(fp_, buffer_, sizeof buffer_);
}

AppendFile::~AppendFile()
{
    fclose(fp_);
}

void AppendFile::append(const char *logline, const size_t len)
{
    size_t n = this->write(logline, len);
    size_t remain = len - n;
    while(remain > 0)
    {
        size_t x = this->write(logline + n, remain);
        if (x == 0)
        {
            int err = ferror(fp_);
            if (err) fprintf(stderr, "AppendFile::append() failed !\n ");
            break;
        }
        // n代表已经写入的数
        n += x;
        // remain代表还需写入的数
        remain = len - n;
    }
}

/*
    定义函数：int fflush(FILE* stream);

    函数说明：fflush()会强迫将缓冲区内的数据写回参数stream 指定的文件中. 如果参数stream 为NULL,fflush()会将所有打开的文件数据更新.
 */

void AppendFile::flush() { fflush(fp_); }

size_t AppendFile::write(const char* logline, size_t len)
{
    //写文件的不加锁的版本，线程不安全
    return fwrite_unlocked(logline, 1, len, fp_);
}
