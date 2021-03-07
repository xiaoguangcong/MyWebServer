#pragma once

// 如果不想让外面的用户直接构造一个类的对象，而希望用户只能构造这个类的子类，
// 那你就可以将类的构造函数/析构函数声明为protected，而将类的子类的构造函数/析构函数声明为public

class NonCopyable
{
protected:
    NonCopyable() {}
    ~NonCopyable() {}
private:
    NonCopyable(const NonCopyable&);
    const NonCopyable& operator=(const NonCopyable&);
};
