#ifndef DEFER_H
#define DEFER_H
#include <functional>

typedef std::function<void()> funDefer;
#define CONTACT(a, b) a##b
#define CONTACT_(a, b) CONTACT(a, b) //用来解析转移传入变量的
#define defer(code) Defer CONTACT_(__Defer__, __LINE__)([&]() { code; }) //按照代码行数造一个变量名

class Defer
{
private:
    funDefer fun;

public:
    Defer(funDefer &&fun) : fun(fun) {}
    ~Defer() { fun(); }
};
#endif