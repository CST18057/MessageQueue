#ifndef OJSOCKET_H
#define OJSOCKET_H
#include <string>
#include <vector>
#include <ctime>
#include <set>
#include <deque>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include "jsonObject.h"

#define BUFSIZE 4096
#define MAXSIZE 1024
#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 18057
#define MAX_LISTEN 10

#define CMD_OK 0
#define NONE_QUEUE 1
#define QUEUE_BUSY 2
#define NONE_GROUP 11
#define GROUP_BUSY 12
#define CONSUMER_BUSY 21
#define NONE_MESSAGE 31

class LinkList
{
private:
    std::unordered_map<int, int> pre,nex;
    //head表示头结点，tail表示尾结点，end表示结束标识符，size为列表内的结点数（不包含头结点）
    int _head = -1, _end = -2, _tail = -1, _size = 0;
public:
    int getPre(int x);
    int getNext(int x);
    void pop(int x);
    void pushBack(int x);
    void moveBack(int x);
    int front();
    int size();
    int end();
    int tail();
};

class ClientBuf
{
public:
    std::deque<char> readBuf, writeBuf;
    int socketFd;
    char buf[BUFSIZE];
    ClientBuf();
    ClientBuf(int clientFd);
    std::string getMessage();
    void addReadBuf(char buffer[], int n);
    void addWriteBuf(std::string msg);
    int sendWriteBuf();
    int recvReadBuf();
    int push();
    int pull();
};


// 加入一个数据后会返回一个唯一的id，可以用该id检索到该数据
template<class T>
class DataPool
{
private:
    unordered_map<int, T> pool;
    set<int> ids;
    int lastId = 1;
public:
    void setStartId(int id)
    {
        lastId = id;
    }
    T get(int id, T defalutValue = T())
    {
        auto x = pool.find(id);
        return x == pool.end() ? defalutValue : *x;
    }
    bool pop(int id)
    {
        return pool.erase(x) && ids.erase(x);
    }
    int put(T value = T())
    {
        pool[lastId] = T;
        ids.insert(lastId);
        return lastId++;
    }
    int LastMessageId()
    {
        return ids.empty() ? -1 : *(--ids.end());
    }
    T LastMessage(T defaultValue = T())
    {
        return ids.empty() ? defaultValue : pool[*(--ids.end())];
    }
    int getUpperBoundMessageId(int id)
    {
        auto x = ids.upper_bound(id);
        return x == ids.end() ? -1 : *x;
    }
    T getUpperBoundMessage(int id, T defaultValue = T())
    {
        int x = getUpperBoundMessageId(id);
        return x == -1 ? defaultValue : pool[x];
    }
    unsigned int size()
    {
        return pool.size();
    }
};


// 用于返回对应命令的字符串，将其发送则意味着执行对应的命令
class CommandStr
{
public:
    string addMessageQueue(string queue, uint maxSize);
    string addGroup(string queue, string group, uint autoClaimTime, string consumer);
    string addMessage(string data,string queue);
    string readMessage(string consumer, string queue, int count = 1, int block = 0);
    string readGroupMessage(string queue, string consumer, string group, int count = 1, int block = 0);
    string ackMessage(string group, int MessageId);
    string delMessage(int MessageId);
    // 自动转移超时持有信息，按ms计算
    string autoClaimMessage(string group, uint autoClaimTime, string consumer);
};

class Consumer
{
public:
    string name;
    uint exceptMessageSize = 0;
    ull block = 0;
    ull joinTime = 0;
    LinkList messages;
    int clientFd = 0;
};

class ConsumerGroup
{
public:
    string name;
    // -1表示指向消息linklist的头节点
    int lastId = -1;
    unordered_map<string, Consumer> consumers;
    unordered_set<string> waitConsumers;
    unordered_map<int, HoldTime<int>> messageInfos;
    set<int> pendingMessages;
};

using ull = unsigned long long;
using uint = unsigned int;
template<class T>
class HoldTime
{
private:
    T id;
    uint hlodCount = 0;
    ull start = 0, end = 0;
    string owner;

public:
    HoldTime(T id,const string& owner)
    {
        this->owner = owner;
        this->id = id;
        start = getTimeStamp();
        hlodCount = 1;
        end = 0;
    }
    uint hold()
    {
        if(!end)
            return getTimeStamp() - start;
        return end - start;
    }
    uint count()
    {
        return hlodCount;
    }
    void stop()
    {
        end = getTimeStamp();
    }
    void claim(const string& owner)
    {
        this->owner = owner;
        hlodCount++;
        start = getTimeStamp();
        end = 0;
    }

};

class Message
{
public:
    string data = "";
};

class MessageQueue
{
public:
    string name;
    unsigned int MAX_SIZE = 1000;
    DataPool<Message> messagePool;
    unordered_map<string, ConsumerGroup> groups;
    LinkList Messages;
    unordered_set<string> waitGroups;
    // 用来存储block等待的连接
    unordered_set<int> waitConsumers;
};

class Scheduler
{ 
private:
    unordered_map<string, MessageQueue> messageQueues;
    unordered_map<int, ClientBuf> clients;
    // unordered_set<int> needWriteClients;

    // 阻塞消费者队列 ,存着pair first是阻塞最晚时间，second是该连接标识符
    set<pair<ull, int>> blockLink;

    // 持有信息消费者队列，first是阻塞最晚时间，second是(队列名，消费者组名，消费者名)
    set<pair<ull, tuple<string, string, string>>> holdConsumers;

    // 此连接标识符当前对应的 队列名，消费者组名，消费者名
    // 当断开连接的时候去掉相应的消费者
    unordered_map<int, tuple<string, string, string>> clientConsumers;
    ull MAX_DELAY_TIME = 1000000000000000000ull;
    int nowClient = -1;


public:
    // 设置epoll标识符
    int epollFd;
    // 改用整体调度，分层调度参数传入和阻塞操作太麻烦
    void setClient(int clientFd);
    void response(string result, int clientFd = -1);
    void readMessage(const string &queue,int MessageId, int count = 1,int block = 0);
    void readMessageGroup(const string &queue, const string &group, const string &consumer,int MessageId, int count = 1,int block = 0);
    void addMessage(const string &queue, const string& data);
    void addClient(int clientFd);
    void delClient(int clientFd);
    void createQueue(const string &queue);
    void delQueue(const string &queue);
    void createGroup(const string &queue, const string &group, bool mkQueue = false);
    void delGroup(const string &queue, const string &group);
    void ackMessage(const string &queue, const string &group, int MessageId);
    void delConsumer(const string &queue, const string &group, const string &consumer);
    string packageMessage(int code, const string &data);
    string packageMessage(int code, const JsonObject &data);
    void parse(int clientFd, const string &s);
    // 返回过多长时间会有一个阻塞连接到时间
    ull awakeTime();
    // 删除阻塞连接队列中的该连接
    void delClientFdFromBlockLink(int clientFd);
    // 反馈阻塞的消费者连接
    void responseBlockConsumer();
};


template<typename Function, typename Tuple, std::size_t... Index>
decltype(auto) invoke_impl(Function&& func, Tuple&& t, std::index_sequence<Index...>)
{
    return func(std::get<Index>(std::forward<Tuple>(t))...);
}

template<typename Function, typename Tuple>
decltype(auto) invoke(Function&& func, Tuple&& t)
{
    constexpr auto size = std::tuple_size<typename std::decay<Tuple>::type>::value;
    return invoke_impl(std::forward<Function>(func), std::forward<Tuple>(t), std::make_index_sequence<size>{});
}

extern const unordered_map<string, function<void(const JsonDict &obj, Scheduler &scheduler)>> callFunName;
#ifdef __linux__
const int ALL_OP = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLOUT;
const int NO_OUT_OP = EPOLLIN | EPOLLERR | EPOLLHUP;
#else

#endif

void addEvent(int epollfd, int fd, int state);
void delEvent(int epollfd, int fd);
void modEvent(int epollfd, int fd, int state);

//header -> 四位int型，表示之后多少个字节是一整个数据包
#define HEADER_SIZE 4

// 获取当前时间戳
ull getTimeStamp();

unsigned long long getTimeStamp();

//创建服务器并返回服务器标识符
int createServer();

//创建客户端并返回客户端标识符
int createClient();

//发送一个HEADER_SIZE字节大小的int整型
bool sendHeader(int msgSize);

//接收一个HEADER_SIZE字节大小的int整型
int recvHeader(int targetFd);

//发送一个字符串数据
bool sendMsg(int targetFd, std::string message);
int sendMsg(int targetFd, char buf[],int size);

//接收固定size的数据
bool recvMsg(int msgSize);
int recvMsg(int targetFd, char buf[]);

int setnonblocking(int socketFd);


void polling(int serverFdm, Scheduler scheduler);

void closeSocket(int socketFd);

std::vector<std::string> split(std::string s, std::string flag);

#endif