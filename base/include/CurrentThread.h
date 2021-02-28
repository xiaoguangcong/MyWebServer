#pragma once

#include <stdint.h>

namespace CurrentThread
{

/*
__thread

1. GCC内置的线程局部存储设施,__thread变量每一个线程有一份独立实体，各个线程的值互不干扰。可以用来修饰那些带有全局性且值可能变，但是各线程独立不干扰的变量；
2. 只能修饰POD类型(类似整型指针的标量)，不能修饰class类型，因为无法自动调用构造函数和析构函数;
3. 可以用于修饰全局变量，函数内的静态变量，不能修饰函数的局部变量或者class的普通成员变量;
4. 且__thread变量值只能初始化为编译器常量。

 */

extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char* t_threadName;

void cacheTid();

inline int tid()
{   
    /*
    这个指令是gcc引入的，作用是允许程序员将最有可能执行的分支告诉编译器。
    这个指令的写法为：__builtin_expect(EXP, N)。
    意思是：EXP==N的概率很大。
     */
    if(__builtin_expect(t_cachedTid == 0, 0))
    {
        cacheTid();
    }
    
    return t_cachedTid;
}

inline const char* tidString()  // for logging
{
    return t_tidString;
}

inline int tidStringLength()    // for logging
{
    return t_tidStringLength;
}

inline const char* name()
{
    return t_threadName;
}

};