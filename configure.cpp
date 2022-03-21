#include "configure.h"
#include <fstream>
#include <iostream>
using namespace std;
Configure::Configure() {}

Configure::Configure(std::string filename) : filename(filename) {}

bool Configure::parse(std::string filename,std::string separator)
{
    if(filename != "")
        this->filename = filename;
    string s;
    string nowGroup = "";
    ifstream in(this->filename);
    if(!in.is_open())
    {
        // cout << "文件打开失败" << endl;
        return false;
    }
    while(getline(in,s))
    {
        cout << s << endl;
        int l = 0, r = s.length() - 1;
        while(l <= r && isspace(s[l]))
            l++;
        while (l <= r && isspace(s[r]))
            r--;
        s = s.substr(l, r - l + 1);
        if(s.size() < 2 || s[0] == '#')
            continue;
        if(s[0] == '[' && s[s.size() - 1] == ']')
        {
            nowGroup = s.substr(1, s.size() - 2);
            continue;
        }
        int k = s.find("=");
        if(k != -1)
        {
            string key = s.substr(0, k);
            string value = s.substr(k + 1, s.size());
            groups[nowGroup][key] = value;
            continue;
        }
        return false;
    }
    return true;
}

bool Configure::write(std::string filename)
{
    ofstream out(filename);
    if(!out.is_open())
        return false;
    for(pair<string,map<string,string>> group : groups)
    {
        out << "[" << group.first << "]" << endl;
        for (pair<string,string> kv : group.second)
        {
            out << kv.first << "=" << kv.second << endl;
        }
    }
    return true;
}