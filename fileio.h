#ifndef FILEIO_H
#define FILEIO_H
#include <vector>
#include <string>

#define SYS_ERROR 100

//根据flag字符串切分字符
std::vector<std::string> split(std::string s, std::string flag = "");
std::string replace(std::string s, std::string old_s, std::string new_s, int n = 1e9 + 7);
bool endWith(const char *a, const char *b);
bool startWith(const char *a, const char *b);

//根据文本中的空白符将所有的非空白符分组
std::vector<std::string> fileSplitToVectorByBlank(std::string filename);
std::vector<std::string> fileSplitToVectorByNonBlank(std::string filename);
std::string fileToStr(std::string filename, unsigned int size = 0);
int strToFile(std::string filename, std::string data, const char* flag = "w");
std::string fileBlankToStr(std::string filename);
std::string fileNonBlankToStr(std::string filename);

//合并路径
std::string osJoin(std::string path, std::string file);

//获取文件最近更新时间
int getLastUpdateTime(std::string filename);
int getFileSize(std::string filename);
bool existsFile(std::string filename);

void fClose();
//设置文件定向
std::vector<std::string> searchFiles(std::string path, std::string suffix = "", bool (*check) (const char*,const char*) = endWith);
void fRedirectAppend(std::string inFile, std::string outFile, std::string errorFile);
void fRedirectWrite(std::string inFile, std::string outFile, std::string errorFile);
std::string commandAppendIOE(std::string command, std::string inFile, std::string outFile, std::string errorFile);

bool isBlank(char c);

//根据不同的精度或忽略换行等场景进行文件比对
#define STRICT_CNSISTENT 0      //严格一致,该选项应该是独立的，不能与其他选项共存
#define ALLOW_PRECISION_ERROR 2 //允许存在精度误差
#define IGNORE_END_BLANK 4      //忽略每行换行符前的空白符，并且会忽略最后一个可见字符之后的一切空白字符
#define IGNORE_ALL_BLANK 8      //忽略一切空白符，只比较非空白符
#define IGNORE_CASE 16          //忽略大小写

//根据flag标识进行两个文件的对比，文件一致返回AC，不一致返回WA，格式错误返回PE，出现意外错误退出进程，退出码为SYS_ERROR
int compareFile(std::string f1, std::string f2, int flag = STRICT_CNSISTENT, double eps = 1e-6);
//比较两个浮点字符串，如果其中一个不能转为浮点数，或两个数字转为浮点数后的误差超过了eps则返回0，否则返回1
int compareFloatStr(std::string a, std::string b, double eps);

#endif