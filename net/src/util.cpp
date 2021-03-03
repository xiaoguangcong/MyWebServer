#include "util.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/socket.h>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>



/*
    头文件：#include <unistd.h>

    定义函数：ssize_t read(int fd, void * buf, size_t count);

    函数说明：read()会把参数fd所指的文件传送count个字节到buf指针所指的内存中.
              若参数count为0, 则read()不会有作用并返回0. 
              返回值为实际读取到的字节数, 如果返回0, 表示已到达文件尾或是无可读取的数据,此外文件读写位置会随读取到的字节移动.

    附加说明：
    如果顺利read()会返回实际读到的字节数, 最好能将返回值与参数count 作比较,
    若返回的字节数比要求读取的字节数少, 则有可能读到了文件尾.

    当有错误发生时则返回-1, 错误代码存入errno 中, 而文件读写位置则无法预期.

    错误代码：
    EINTR 此调用被信号所中断.
    EAGAIN 当使用不可阻断I/O 时(O_NONBLOCK), 若无数据可读取则返回此值.
    EBADF 参数fd 非有效的文件描述词, 或该文件已关闭.

 */


const static int MAX_BUFF = 4096;

ssize_t Readn(int fd, void *buff, size_t n)
{
    ssize_t nleft = n;
    ssize_t nread = 0;
    ssize_t readSum = 0;
    char *ptr = (char*) buff;
    while(nleft > 0)
    {
        if ((nread = read(fd, ptr, nleft)) < 0)
        {
            if (errno == EINTR)
            {
                nread = 0;
            }
            else if (errno == EAGAIN)
            {
                return readSum;
            }
            else
            {
                return -1;
            }
        }
        else if (nread == 0)
        {
            break;
        }
        readSum += nread;
        nleft -= nread;
        ptr += nread;
    }
    return readSum;
}

ssize_t Readn(int fd, std::string &inBuffer, bool &zero)
{
    ssize_t nread = 0;
    ssize_t readSum = 0;
    while(true)
    {
        char buffer[MAX_BUFF];
        if ((nread = read(fd, buffer, MAX_BUFF)) < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else if (errno == EAGAIN)
            {
                return readSum;
            }
            else
            {
                perror("read error");
                return -1;
            }
        }
        else if (nread == 0)
        {
            zero = true;
            break;
        }
        readSum += nread;
        inBuffer += std::string(buffer, buffer+nread);
    }
    return readSum;
}

ssize_t Readn(int fd, std::string &inBuffer)
{
    ssize_t nread = 0;
    ssize_t readSum = 0;
    while(true)
    {
        char buffer[MAX_BUFF];
        if ((nread = read(fd, buffer, MAX_BUFF)) < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else if (errno == EAGAIN)
            {
                return readSum;
            }
            else
            {
                perror("read error");
                return -1;
            }
        }
        else if (nread == 0)
        {
            break;
        }
        readSum += nread;
        inBuffer += std::string(buffer, buffer+nread);
    }
    return readSum;
}

ssize_t Writen(int fd, void *buff, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    char *ptr = (char *)buff;
    while (nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0)
            {
                if (errno == EINTR)
                {
                    nwritten = 0;
                    continue;
                }
                else if (errno == EAGAIN)
                {
                    return writeSum;
                }
                else 
                {
                    return -1;
                }
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    return writeSum;
}

ssize_t Writen(int fd, std::string &sbuff)
{
    ssize_t nleft = sbuff.size();
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    const char *ptr = sbuff.c_str();
    while(nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0)
            {
                if (errno == EINTR)
                {
                    nwritten = 0;
                    continue;
                }
                else if (errno == EAGAIN)
                {
                    return writeSum;
                }
                else 
                {
                    return -1;
                }
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    if (writeSum == static_cast<int>(sbuff.size()))
    {
        sbuff.clear();
    }
    else 
    {
        sbuff = sbuff.substr(writeSum);
    }
    return writeSum;
}

/*
    void *memset(void *s, int ch, size_t n);
    函数解释：将s中当前位置后面的n个字节 （typedef unsigned int size_t ）用 ch 替换并返回 s 。
    memset：作用是在一段内存块中填充某个给定的值，它是对较大的结构体或数组进行清零操作的一种最快方法 [1]  。
    memset()函数原型是extern void *memset(void *buffer, int c, int count) buffer：为指针或是数组,c：是赋给buffer的值,count：是buffer的长度.

 */ 

void HandleForSigpipe()
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof sa);
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL)) return;
}

int SetSocketNonBlocking(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);   // 取得文件描述符filedes的文件状态标志
    if (flag == -1)
    {
        return -1;
    }

    flag |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flag) == -1)  // 设置文件描述符filedes的文件状态标志
    {
        return -1;
    }
    return 0;
}

/*
    int setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen);

    参数：  
    sock：将要被设置或者获取选项的套接字。
    level：选项所在的协议层。
    optname：需要访问的选项名。
    optval：对于getsockopt()，指向返回选项值的缓冲。对于setsockopt()，指向包含新选项值的缓冲。
    optlen：对于getsockopt()，作为入口参数时，选项值的最大长度。作为出口参数时，选项值的实际长度。对于setsockopt()，现选项的长度。

 */

void SetSocketNodelay(int fd)
{
    int enable = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable, sizeof enable);
}

void SetSocketNoLinger(int fd)
{
    struct linger linger_;
    linger_.l_onoff = 1;     // 允许套接口延迟关闭
    linger_.l_linger = 30;   // 指明延迟关闭的时间
    setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char*)&linger_, sizeof linger_);
}

/* 
    TCP半关闭
    在TCP服务端和客户端建立连接之后服务端和客户端会分别有两个独立的输入流和输出流，而且相互对应。
    服务端的输出流对应于客户端的输入流，服务端的输入流对应于客户端的输出流。这是在建立连接之后的状态。

　　当我们调用close()函数时，系统会同时把双方的输入流和输出流全部关闭，
    但是有时候我们仍需要在一方断开连接之后只进行接受数据或者传输数据其中一项操作。
    这时就需要我们只断开输入或者输出，保留另一个流的正常运转，也就引入了TCP的半关闭状态。

    基本操作：

    之前我们传输完数据之后便直接调用了close()函数，我们可以使用系统提供的shutdown()函数方便的完成TCP的半关闭。

    shutdown(int socket , int type)：半关闭套接字中的输入或者输出流

    socket（套接字描述符）：需要断开的套接字描述符
    type（断开类型）：套接字的断开方式
　　
    SHUT_RD——断开输入流，并清空输入缓冲中的数据
　　SHUT_WR——断开输出流，并将输出缓冲中的数据输出
　　SHUT_RDWR——同时断开输入输出流，分两次调用shutdown()函数

　　成功时返回0，失败时返回-1

 */ 

void ShutDownWR(int fd)
{
    shutdown(fd, SHUT_WR);
}

int socket_bind_listen(int port)
{
    // 检查port值，取正确区间范围
    if (port < 0 || port > 65535)
    {
        return -1;
    }

    // 创建socket(IPv4 + TCP)，返回监听描述符
    int listen_fd = 0;
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) // //创建IPv4 TCP套接字
    {
        return -1;
    }

    // 消除bind时"Address already in use"错误
    int optval = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) == -1 )  // 
    {
        close(listen_fd);
        return -1;
    }

    // 设置服务器IP和Port，和监听描述副绑定
    struct sockaddr_in server_addr;
    bzero((char *)&server_addr, sizeof server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)port);
    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof server_addr) == -1)
    {
        close(listen_fd);
        return -1;
    }

    // 开始监听，最大等待队列长为LISTENQ
    if (listen(listen_fd, 2048) == -1)
    {
        close(listen_fd);
        return -1;
    }

    // 无效监听描述符
    if (listen_fd == -1)
    {
        close(listen_fd);
        return -1;
    }

    return listen_fd;
}