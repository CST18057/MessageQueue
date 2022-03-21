#ifndef LOG_H
#define LOG_H
#include <unordered_map>
#include <map>
#include <vector>
#include <memory>
#include <fstream>
#include <string>
#include <functional>
class Log
{
public:
    static Log& instance(const std::string& dir = "./");
    void error(const char* data);
    void output(const char* data);
    bool changeOutputFile(std::string logFile);
    bool changeErrorFile(std::string errorFile);
    template<class... T>
    void printf(const char* s,T... t)
    {
        char data[256] = {0};
        sprintf(data, s, t...);
        out << data;
    }
    template<class... T>
    void wprintf(const wchar_t* s,T... t)
    {
        wchar_t data[256] = {0};
        sprintf(data, s, t...);
        out << data;
    }
    std::ofstream &getOutput();
    std::ofstream &getError();    
    ~Log(); 

private:
    std::ofstream out,err;
    explicit Log(const std::string& dir);
};
#endif