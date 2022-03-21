#ifndef CONFIGURE_H
#define CONFIGURE_H
#include <string>
#include <map>
class Configure
{
    std::string filename;

public:
    std::map<std::string, std::map<std::string, std::string>> groups;
    Configure();
    Configure(std::string filename);
    bool parse(std::string filename = "",std::string separator = "=");
    // 将当前配置写入文件中，实现的是全部配置都写入覆盖，可能性能并不理想
    bool write(std::string filename);
};
#endif