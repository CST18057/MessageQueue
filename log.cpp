#include "log.h"
#include <iostream>
using namespace std;
Log::Log(const std::string& dir)
{
    out.open(dir + "./out.txt", ios::app);
    err.open(dir + "./error.txt", ios::app);
}

Log& Log::instance(const std::string& dir)
{
    static Log log(dir);
    return log;
}

void Log::error(const char* data)
{
    err << data;
}
void Log::output(const char* data)
{
    cout << data;
    out << data;
}
bool Log::changeOutputFile(std::string logFile)
{
    out.close();
    out.open(logFile, ios::app | ios::out);
    return out.is_open();
}
bool Log::changeErrorFile(std::string errorFile)
{
    err.close();
    err.open(errorFile, ios::app);
    return err.is_open();
}
std::ofstream& Log::getOutput()
{
    return out;
}
std::ofstream& Log::getError()
{
    return err;
}

Log::~Log()
{
    out.flush();
    err.flush();
    out.close();
    err.close();
}