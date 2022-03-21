#include <vector>
#include <string>
#include <cmath>
#include <string.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <assert.h>
#include <sys/stat.h>
#include "fileio.h"
#include "log.h"

using namespace std;
#ifdef __linux__
#include <unistd.h>
#include <dirent.h>
#include <string.h>
std::vector<std::string> searchFiles(std::string path, std::string pattern, bool (*check)(const char *, const char *))
{
    vector<string> files;
    struct dirent *ptr;
    DIR *dir;
    dir = opendir((path).c_str());
    // printf("文件列表:\n");
    while ((ptr = readdir(dir)) != NULL)
    {

        //跳过'.'和'..'两个目录
        if (ptr->d_name[0] == '.')
            continue;
        if (check(ptr->d_name, pattern.c_str()))
            files.push_back(ptr->d_name);
    }
    closedir(dir);
    return files;
}

#endif

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#include <windows.h>
std::vector<std::string> searchFiles(std::string path, std::string pattern, bool (*check)(const char *, const char *))
{
    long handle;
    path = path + "/*";
    struct _finddata_t fileinfo;
    vector<string> files;
    //第一次查找
    handle = _findfirst(path.c_str(), &fileinfo);
    if (handle == -1)
        return files;
    do
    {
        //找到的文件的文件名
        if (fileinfo.name[0] == '.')
            continue;
        if (check(fileinfo.name, pattern.c_str()))
            files.push_back(fileinfo.name);

    } while (!_findnext(handle, &fileinfo));
    _findclose(handle);
    return files;
}

#endif

std::string replace(std::string s, std::string old_s, std::string new_s, int n)
{
    int pre = 0, now;
    string res = "";
    while (pre < s.length() && (now = s.find(old_s, pre)) != -1 && n)
    {
        res += s.substr(pre, now - pre);
        res += new_s;
        pre = now + old_s.length();
        n--;
    }
    res += s.substr(pre);
    return res;
}

std::vector<std::string> split(std::string s, std::string flag)
{
    std::vector<std::string> res;
    if (flag == "")
    {
        bool f = true;
        for (int i = 0; i < s.length();i++)
        {
            if(isspace(s[i]))
                f = true;
            else
            {
                if(f)
                {
                    res.push_back("");
                    f = false;
                }
                res.back().append(1,s[i]);
            }
        }
        return res;
    }
    int pre = 0, x;
    while (true)
    {
        x = s.find(flag, pre);
        if (x == -1)
        {
            res.push_back(s.substr(pre));
            break;
        }
        else
        {
            res.push_back(s.substr(pre, x - pre));
            pre = x + flag.length();
        }
    }
    return res;
}

bool startWith(const char *a, const char *b)
{
    int n = strlen(a), m = strlen(b);
    if (n < m)
        return false;
    for (int i = 0; i < m; i++)
    {
        if (a[i] != b[i])
            return false;
    }
    return true;
}

bool endWith(const char *a, const char *b)
{
    int n = strlen(a), m = strlen(b);
    if (n < m)
        return false;
    for (int i = 0; i < m; i++)
    {
        if (a[n - 1 - i] != b[m - 1 - i])
            return false;
    }
    return true;
}

void fClose()
{
#if defined(WIN32) || defined(WIN64)
    freopen("CON", "w", stderr);
    freopen("CON", "w", stdout);
    freopen("CON", "r", stdin);
#else
    freopen("/dev/tty", "w", stderr);
    freopen("/dev/tty", "w", stdout);
    freopen("/dev/tty", "r", stdin);
#endif
    // cout << "redirect console ok" << endl;
}

void fRedirectAppend(std::string inFile, std::string outFile, std::string errorFile)
{
    // cout << inFile << " " << outFile << " " << errorFile << endl;
    if (errorFile != "")
        if (freopen(errorFile.c_str(), "a+", stderr) == NULL)
        {
            Log::instance().getError() << errorFile << " open a_mode error " << endl;
            exit(SYS_ERROR);
        }
    if (outFile != "")
        if (freopen(outFile.c_str(), "a+", stdout) == NULL)
        {
            Log::instance().getError() << outFile << " open a_mode error " << endl;
            exit(SYS_ERROR);
        }
    if (inFile != "")
        if (freopen(inFile.c_str(), "r+", stdin) == NULL)
        {
            Log::instance().getError() << inFile << " open r_mode error " << endl;
            exit(SYS_ERROR);
        }
}

void fRedirectWrite(std::string inFile, std::string outFile, std::string errorFile)
{
    // cout << inFile << " " << outFile << " " << errorFile << endl;
    if (errorFile != "")
        if (freopen(errorFile.c_str(), "w+", stderr) == NULL)
        {
            Log::instance().getError() << errorFile << " open w_mode error " << endl;
            exit(SYS_ERROR);
        }
    if (outFile != "")
        if (freopen(outFile.c_str(), "w+", stdout) == NULL)
        {
            Log::instance().getError() << outFile << " open w_mode error " << endl;
            exit(SYS_ERROR);
        }
    if (inFile != "")
        if (freopen(inFile.c_str(), "r+", stdin) == NULL)
        {
            Log::instance().getError() << inFile << " open r_mode error " << endl;
            exit(SYS_ERROR);
        }
}

std::string commandAppendIOE(std::string command, std::string inFile, std::string outFile, std::string errorFile)
{
    if (outFile != "")
        command += " 1>" + outFile;
    if (errorFile != "")
    {
        if(outFile == errorFile)
            command += " 2>&1";
        else
            command += " 2>" + errorFile;        
    }
    if (inFile != "")
        command += " <" + errorFile;
    return command;
}

std::string osJoin(std::string path, std::string file)
{
    if (path.back() == '/')
        path.pop_back();
    return path + "/" + file;
}

int getLastUpdateTime(string filename)
{
    struct stat buf;
    stat(filename.c_str(), &buf);
    return buf.st_mtime;
}

bool existsFile(std::string filename)
{
    return access(filename.c_str(), 0) != -1;
}

int getFileSize(string filename)
{
    struct stat buf;
    stat(filename.c_str(), &buf);
    return buf.st_size;
}

std::string fileToStr(std::string filename, unsigned int size)
{
    ifstream infile;
    infile.open(filename);
    if (!infile.is_open())
    {
        Log::instance().getError() << "open ifstream " << filename << " error" << endl;
        exit(SYS_ERROR);
    }
    stringstream in;
    in << infile.rdbuf();
    infile.close();
    string res = in.str();
    if (size > 0 && res.length() > size)
        return res.substr(0,size) + "...省略" + to_string(res.length() - size) + "个字符";
    return res;
}

int strToFile(std::string filename, std::string data, const char* flag)
{
    ofstream out;
    if(flag == "w")
        out.open(filename, ios::out);
    else if(flag == "a")
        out.open(filename, ios::app);
    if(!out.is_open())
    {
        Log::instance().getError() << "open ifstream " << filename << " error" << endl;
        return false;
    }
    out << data;
    return true;
}

bool isBlank(char c)
{
    return isspace(c) || iscntrl(c);
}
