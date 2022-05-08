#include "ojSocket.h"
#include "defer.h"
using namespace std;
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <ctime>
#include <queue>
#include <unordered_set>
#include <unordered_map>

ull getTimeStamp()
{
#ifdef __linux__
    struct timeval time;
 
    /* 获取时间，理论到us */
    gettimeofday(&time, NULL);
    return (ull)(time.tv_usec / 1000) + (ull)time.tv_sec * 1000;
#else
    return 0;
#endif
}
// 此处应当传入参数列表，和Scheduler类
const unordered_map<string, function<void(const JsonDict &obj, Scheduler &scheduler)>> callFunName =
    {
        {"add", [](const JsonDict &obj, Scheduler &scheduler) -> void
         {
            string &queue = obj.at("queue")->asString();
            string &data = obj.at("data")->asString();
            scheduler.addMessage(queue, data);
         }},
        {"read", [](const JsonDict &obj, Scheduler &scheduler) -> void
         {
            string &queue = obj.at("queue")->asString();
            int message_id = obj.at("message_id")->asInt();
            int count = obj.at("count")->asInt();
            int block = obj.at("block")->asInt();
            scheduler.readMessage(queue, message_id, count, block);
            
         }},
        {"readgroup", [](const JsonDict &obj, Scheduler &scheduler) -> void {
            string &queue = obj.at("queue")->asString();
            string &group = obj.at("group")->asString();
            string &consumer = obj.at("consumer")->asString();
            int message_id = obj.at("message_id")->asInt();
            int count = obj.at("count")->asInt();
            int block = obj.at("block")->asInt();
            scheduler.readMessageGroup(queue, group, consumer, message_id, count, block);
         }},
        {"createqueue", [](const JsonDict &obj, Scheduler &scheduler) -> void {
            string &queue = obj.at("queue")->asString();
            scheduler.createQueue(queue);
         }},
        {"creategroup", [](const JsonDict &obj, Scheduler &scheduler) -> void {
            string &queue = obj.at("queue")->asString();
            string &group = obj.at("group")->asString();
            bool mkqueue = obj.at("mkqueue")->asBool();
            scheduler.createGroup(queue, group, mkqueue);
         }},
        {"ack", [](const JsonDict &obj, Scheduler &scheduler) -> void {
            string &queue = obj.at("queue")->asString();
            string &group = obj.at("group")->asString();
            int message_id = obj.at("message_id")->asInt();
            scheduler.ackMessage(queue, group, message_id);
         }},
        {"delqueue", [](const JsonDict &obj, Scheduler &scheduler) -> void {

         }},
        {"delgroup", [](const JsonDict &obj, Scheduler &scheduler) -> void {

         }},
};

//class LinkList
LinkList::LinkList()
{
    nex[_head] = _end;
}

int LinkList::getPre(int x)
{
    return pre[x];
}

int LinkList::getNext(int x)
{
    return nex[x];
}

void LinkList::pop(int x)
{
    _size--;
    nex[pre[x]] = nex[x];
    pre[nex[x]] = pre[x];
}

void LinkList::pushBack(int x)
{
    _size++;
    nex[x] = _end;
    pre[x] = _tail;
    nex[_tail] = x;
    _tail = x;
}

void LinkList::moveBack(int x)
{
    pop(x);
    pushBack(x);
}

int LinkList::front()
{
    return nex[_head];
}

int LinkList::size()
{
    return _size;
}

int LinkList::end()
{
    return _end;
}

int LinkList::tail()
{
    return _tail;
}

//class ClientBuf
ClientBuf::ClientBuf()
{
}

ClientBuf::ClientBuf(int clientFd)
{
    socketFd = clientFd;
}

int ClientBuf::recvReadBuf()
{
    int nread = recvMsg(socketFd, buf);
    addReadBuf(buf, nread);
    return nread;
}

void ClientBuf::addReadBuf(char buffer[], int n)
{
    if (n <= 0)
        return;
    readBuf.insert(readBuf.end(), buffer, buffer + n);
}

void ClientBuf::addWriteBuf(std::string msg)
{
    int msgSize = msg.length();
    char header[HEADER_SIZE] = {0, 0, 0, 0};
    for (int i = 0; i < HEADER_SIZE; i++)
        header[HEADER_SIZE - i - 1] = msgSize >> i * 8 & 0xff;
    writeBuf.insert(writeBuf.end(), header, header + HEADER_SIZE);
    writeBuf.insert(writeBuf.end(), msg.begin(), msg.end());
}

int ClientBuf::sendWriteBuf()
{
    int t = min(BUFSIZE, (int)writeBuf.size());
    for (int i = 0; i < t; i++)
        buf[i] = writeBuf[i];
    int nwrite = sendMsg(socketFd, buf, t);
    for (int i = 0; i < nwrite; i++)
        writeBuf.pop_front();
    return nwrite;
}

std::string ClientBuf::getMessage()
{
    // 使用大端序
    if (readBuf.size() <= HEADER_SIZE)
        return "";
    int datasize = 0;
    for (int i = 0; i < HEADER_SIZE; i++)
        datasize = (datasize << 8) | readBuf[i];
    if (readBuf.size() < HEADER_SIZE + datasize)
        return "";
    for (int i = 0; i < HEADER_SIZE; i++)
        readBuf.pop_front();
    std::string res(readBuf.begin(), readBuf.begin() + datasize);
    // readBuf.erase(readBuf.begin(), readBuf.begin() + datasize);
    for (int i = 0; i < datasize; i++) //感觉pop_front()快于erase
        readBuf.pop_front();
    return res;
}

int ClientBuf::push()
{
    int n, sum = 0;
    while (true)
    {
        if (writeBuf.size() == 0)
            return sum;
        n = sendWriteBuf();
        if (n == -1)
        {
            if (errno == EAGAIN || errno == EINTR)
            {
                printf("缓冲区数据已经写完\n");
                break;
            }
            else
            {
                perror("write error");
                printf("Error: %s (errno: %d)\n", strerror(errno), errno);
                return -1;
            }
        }
        else if (n == 0)
        {
            perror("连接已断开\n");
            return -1;
        }
        sum += n;
    }
    return sum;
}

int ClientBuf::pull()
{
    int n, sum = 0;
    while (true)
    {
        n = recvReadBuf();
        if (n == -1)
        {
            if (errno == EAGAIN || errno == EINTR)
            {
                printf("缓冲区数据已经读完\n");
                break;
            }
            else
            {
                perror("recv error\n");
                printf("Error: %s (errno: %d)\n", strerror(errno), errno);
                return -1;
            }
        }
        else if (n == 0)
        {
            perror("unlink error\n");
            return -1;
        }
        sum += n;
    }
    return sum;
}

void Scheduler::setClient(int clientFd)
{
    nowClient = clientFd;
    if(clients.find(nowClient) == clients.end())
        clients[nowClient] = ClientBuf();
}
void Scheduler::response(string result, int clientFd)
{
#ifdef __linux__
    modEvent(epollFd, clientFd == -1 ? nowClient : clientFd , ALL_OP);
#else
#endif
    // 测试用
    // cout << result << endl;
    if(clientFd == -1)
    {
        clients[nowClient].addWriteBuf(result);
    }
    else
    {
        clients[clientFd].addWriteBuf(result);
    }
}

string Scheduler::packageMessage(int code, const string &data)
{
    if(code == 0)
    {
        JsonObject obj(
            JsonDict{
                {"code",JSONOBJECT(code)},
                {"data",JSONOBJECT(data)}
            }
        );
        return obj.json();
    }
    else
    {
        JsonObject obj(
            JsonDict{
                {"code",JSONOBJECT(code)},
                {"error",JSONOBJECT(data)}
            }
        );
        return obj.json();
    }
    return "";
}

string Scheduler::packageMessage(int code, const JsonObject& data)
{
    JsonObject obj(
        JsonDict{
            {"code",JSONOBJECT(code)},
            {"data",JSONOBJECT(data)}
        }
    );
    return obj.json();
}

void Scheduler::readMessage(const string &queue,int MessageId, int count, int block)
{
    auto it = messageQueues.find(queue);
    if (it == messageQueues.end())
    {
        response(packageMessage(NONE_QUEUE,"no name Queue:" + queue));
        return;
    }
    MessageQueue &q = it->second;
    int messageId = q.messagePool.getLowerBoundMessageId(MessageId);
    // 如果没找到
    if (messageId == -1)
    {
        if(block == 0)
        {
            response(packageMessage(0, JsonObject(JsonArray{})));
            return;
        }
        cout << "join blockLink" << endl;
        // 加入阻塞消费者队列
        ull nowTime = getTimeStamp();
        blockLink.insert({nowTime + block ? block : MAX_DELAY_TIME, nowClient});
        // 在当前消费者组加入此等待标识符
        q.waitConsumers.insert(nowClient);
        clientConsumers[nowClient] = {queue, "", ""};
        blockLinkTime.insert({nowClient, nowTime + block ? block : MAX_DELAY_TIME});
    }
    else
    {
        vector<int> mids = q.messagePool.rangeId(MessageId, INT_MAX, count);
        JsonObject vj(
            JsonArray{}
        );
        for (int mid : mids)
            vj.asArray().push_back(
                JSONOBJECT(
                    JsonObject(
                        JsonDict{
                            {"id",JSONOBJECT(mid)},
                            {"message",JSONOBJECT(q.messagePool.get(mid).data)}
                        }
                    )
            ));
        response(packageMessage(CMD_OK,vj));
    }
}
void Scheduler::readMessageGroup(const string &queue, const string &group, const string &consumer,int MessageId, int count, int block)
{
    auto qit = messageQueues.find(queue);
    if (qit == messageQueues.end())
    {
        response(packageMessage(NONE_QUEUE,"no name Queue:" + queue));
        return;
    }
    MessageQueue &q = qit->second;
    auto git = q.groups.find(group);
    if (git == q.groups.end())
    {
        response(packageMessage(NONE_GROUP,"no name Group:" + group));
        return;
    }
    ConsumerGroup &g = git->second;
    if(g.waitConsumers.find(consumer) != g.waitConsumers.end())
    {
        response(packageMessage(CONSUMER_BUSY,"consumer is waiting:" + group));
        return;
    }
    if (MessageId == -1)
        MessageId = g.lastId;
    cout << "message id:" << MessageId << endl;
    auto stop = q.Messages.end();
    // 如果当前没有空闲消息
    if(q.Messages.upper_bound(MessageId) == q.Messages.end())
    {
        // 不等待直接返回
        if(block == 0)
        {
            response(packageMessage(0, JsonObject(JsonArray{})));
            return;
        }

        // 设置好consumer参数
        clientConsumers[nowClient] = {queue, group, consumer};
        q.waitGroups.insert(group);
        g.waitConsumers.insert(consumer);
        Consumer c;
        c.clientFd = nowClient;
        c.name = consumer;
        c.exceptMessageSize = count;
        // 如果block小于0则意味着无限等待
        c.block = block < 0 ? MAX_DELAY_TIME : block;
        c.joinTime = getTimeStamp();
        g.consumers[consumer] = c;
        // 放入block阻塞消费者中
        blockLink.insert({c.joinTime + c.block, nowClient});
        blockLinkTime.insert({nowClient, c.joinTime + c.block});
        return;
    }
    JsonObject vj(
        JsonArray{}
    );
    auto mit = q.Messages.upper_bound(MessageId);
    while(count--)
    {
        if(mit == q.Messages.end())
            break;
        g.lastId = max(g.lastId, *mit);
        g.pendingMessages.insert(*mit);
        vj.asArray().push_back(
                JSONOBJECT(
                    JsonObject(
                        JsonDict{
                            {"id",JSONOBJECT(*mit)},
                            {"message",JSONOBJECT(q.messagePool.get(*mit).data)}
                        }
                    )
            ));
        mit++;
    }
    // needWriteClients.insert(nowClient);
    response(packageMessage(0, vj));
}
void Scheduler::addMessage(const string &queue, const string& data)
{
    auto qit = messageQueues.find(queue);
    if (qit == messageQueues.end())
    {
        response(packageMessage(NONE_QUEUE,"no name Queue:" + queue));
        return;
    }
    MessageQueue &q = qit->second;
    Message msg;
    msg.data = data;
    int messageId = q.messagePool.put(msg);
    q.Messages.insert(messageId);
    response(packageMessage(CMD_OK,"add message success"));
    vector<string> noWaitGroups;
    // cout << "add message ok:" << messageId << endl;
    // cout << "message size:" << q.messagePool.size() << endl;
    for(string group : q.waitGroups)
    {
        ConsumerGroup &g = q.groups[group];
        // 如果当前消费组有消费者等待消息且有消息可读
        auto mit = q.Messages.upper_bound(g.lastId);
        while(mit != q.Messages.end() && !g.waitConsumers.empty())
        {
            // 随机获取集合中的一个元素
            int size = g.waitConsumers.size();
            int t = rand() % size;
            auto it = g.waitConsumers.begin();
            advance(it, t);

            string consumer = *it;
            Consumer &c = g.consumers[consumer];
            JsonObject vj(
                JsonArray{}
            );
            int count = c.exceptMessageSize;
            vector<int> messageIds;
            while(count--)
            {
                if(mit == q.Messages.end())
                    break;
                g.lastId = *mit;
                mit++;
                messageIds.push_back(g.lastId);
                vj.asArray().push_back(JSONOBJECT(q.messagePool.get(g.lastId).data));
                // 将此条消息加入到所在消费者组pendingMessages，并更新messageInfos
                g.pendingMessages.insert(g.lastId);
                g.messageInfos.insert({g.lastId, HoldTime<int>(g.lastId,c.name)});

            }
            for(int i : messageIds)
                c.messages.insert(i);
            // needWriteClients.insert(c.clientFd);
            response(packageMessage(0, vj), c.clientFd);
            // 从阻塞连接中删除当前的连接
            delClientFdFromBlockLink(c.clientFd);
            // 从等待消费者中移除
            g.waitConsumers.erase(it);
        }
        if(g.waitConsumers.empty())
            noWaitGroups.push_back(group);
    }
    for(string group : noWaitGroups)
        q.waitGroups.erase(group);
}
void Scheduler::addClient(int clientFd)
{
    clients.insert({clientFd, ClientBuf(clientFd)});
}
void Scheduler::delClient(int clientFd)
{
    clients.erase(clientFd);
}
void Scheduler::delConsumer(const string &queue, const string &group, const string &consumer)
{
    auto qit = messageQueues.find(queue);
    if(qit != messageQueues.end())
    {
        MessageQueue &q = qit->second;

    }
}
void Scheduler::createQueue(const string &queue)
{
    auto qit = messageQueues.find(queue);
    if (qit != messageQueues.end())
    {
        response(packageMessage(2,"Queue already exist:" + queue));
        return;
    }
    messageQueues.insert({queue, MessageQueue()});
    response(packageMessage(CMD_OK, "create queue success:" + queue));
}
void Scheduler::delQueue(const string &queue)
{
    auto qit = messageQueues.find(queue);
    if (qit == messageQueues.end())
    {
        response(packageMessage(NONE_QUEUE,"no name Queue:" + queue));
        return;
    }
    messageQueues.erase(qit);
    response(packageMessage(CMD_OK, "delete queue success:" + queue));
}
void Scheduler::createGroup(const string &queue, const string &group, bool mkQueue)
{
    auto qit = messageQueues.find(queue);
    if (qit == messageQueues.end())
    {
        if(mkQueue)
        {
            messageQueues.insert({queue, MessageQueue()});
            qit = messageQueues.find(queue);
        }
        else
        {
            response(packageMessage(NONE_QUEUE,"no name Queue:" + queue));
            return;
        }
    }
    MessageQueue &q = qit->second;
    auto git = q.groups.find(group);
    if (git != q.groups.end())
    {
        response(packageMessage(GROUP_BUSY,"Group already exist:" + group));
        return;
    }
    q.groups.insert({group, ConsumerGroup()});
    response(packageMessage(CMD_OK, "create group success"));
}
void Scheduler::delGroup(const string &queue, const string &group)
{
    auto qit = messageQueues.find(queue);
    if (qit == messageQueues.end())
    {
        response(packageMessage(NONE_QUEUE,"no name Queue:" + queue));
        return;
    }
    MessageQueue &q = qit->second;
    auto git = q.groups.find(group);
    if (git == q.groups.end())
    {
        response(packageMessage(NONE_GROUP,"no name Group:" + group));
        return;
    }
    q.groups.erase(git);
    response(packageMessage(CMD_OK, "remove group success:" + group));
}
void Scheduler::ackMessage(const string &queue, const string &group, int MessageId)
{
    auto qit = messageQueues.find(queue);
    if (qit == messageQueues.end())
    {
        response(packageMessage(NONE_QUEUE,"no name Queue:" + queue));
        return;
    }
    MessageQueue &q = qit->second;
    auto git = q.groups.find(group);
    if (git == q.groups.end())
    {
        response(packageMessage(NONE_GROUP,"no name Group:" + group));
        return;
    }
    ConsumerGroup &g = git->second;
    auto mid = g.pendingMessages.find(MessageId);
    if (mid == g.pendingMessages.end())
    {
        response(packageMessage(NONE_MESSAGE,"no Pending Message:" + to_string(MessageId)));
        return;
    }
    g.pendingMessages.erase(mid);
    g.messageInfos[g.lastId].stop();
    response(packageMessage(CMD_OK,"ack message success:" + to_string(*mid)));
}
void Scheduler::parse(int clientFd, const string &s)
{
    // 设置当前正在操作的连接标识符
    nowClient = clientFd;
    JsonObjectPtr obj = JsonObject::decoder(s);
    string &call = obj->asDict().at("call")->asString();
    JsonDict &params = obj->asDict().at("params")->asDict();
    try
    {
        callFunName.at(call)(params, *this);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        response(packageMessage(TYPE_ERROR, e.what()));
    }
}

ull Scheduler::awakeTime()
{
    if(blockLink.empty())
        return -1;
    ull nowTime = getTimeStamp();
    ull nextTime = blockLink.begin()->first;
    return nowTime <= nextTime ? 0 : nextTime - nowTime;
}

void Scheduler::delClientFdFromBlockLink(int clientFd)
{
    ull blockTime = blockLinkTime[clientFd];
    blockLink.erase(blockLink.find({blockTime, clientFd}));
    blockLinkTime.erase(blockLinkTime.find(clientFd));
    auto qgc = clientConsumers.find(clientFd);
    if (qgc != clientConsumers.end())
    {
        string queue = get<0>(qgc->second);
        string group = get<1>(qgc->second);
        string consumer = get<2>(qgc->second);
        MessageQueue &q = messageQueues[get<0>(qgc->second)];
        // 如果不是消费组
        if(group == "")
        {
            q.waitConsumers.erase(clientFd);
        }
        else
        {
            ConsumerGroup &g = q.groups[group];
            g.waitConsumers.erase(consumer);
            if(g.waitConsumers.empty())
                q.waitGroups.erase(group);
        }
    }
}

void Scheduler::info(const string &queue, const string &group, const string &consumer)
{
    if (queue != "")
    {
        return;
    }
    if (group != "")
    {
        return;
    }
    if (consumer != "")
    {
        return;
    }
}

void Scheduler::pending(const string &queue, const string &group, int start, int end, int count, const string &consumer)
{
    auto qit = messageQueues.find(queue);
    if (qit == messageQueues.end())
    {
        response(packageMessage(NONE_QUEUE,"no name Queue:" + queue));
        return;
    }
    MessageQueue &q = qit->second;
    auto git = q.groups.find(group);
    if (git == q.groups.end())
    {
        response(packageMessage(NONE_GROUP,"no name Group:" + group));
        return;
    }
    ConsumerGroup &g = git->second;
    auto it = g.pendingMessages.lower_bound(start);
    auto stop = g.pendingMessages.upper_bound(end);
    // 如果给定了消费者
    if (consumer != "")
    {
        Consumer &c = g.consumers[consumer];
        it = c.messages.lower_bound(start);
        stop = c.messages.upper_bound(end);
    }
    JsonObject data(JsonArray{});
    for (; it != stop; it++)
    {
        if(count == 0)
            break;
        data.asArray().push_back(JSONOBJECT(q.messagePool.get(*it).data));
        count--;
    }
    response(packageMessage(CMD_OK, data), nowClient);
}

void Scheduler::rangeMessage(const string &queue, int start, int end, int count)
{
    auto qit = messageQueues.find(queue);
    if (qit == messageQueues.end())
    {
        response(packageMessage(NONE_QUEUE,"no name Queue:" + queue));
        return;
    }
    MessageQueue &q = qit->second;
    vector<int> v = q.messagePool.rangeId(start, end, count);
    JsonObject data(JsonArray{});
    for (int i : v)
    {
        data.asArray().push_back(JSONOBJECT(q.messagePool.get(i).data));
    }
    response(packageMessage(CMD_OK, data), nowClient);
}

void Scheduler::responseBlockConsumer()
{
    ull nowTime = getTimeStamp();
    while(blockLink.size() && blockLink.begin()->first <= nowTime)
    {
        response(packageMessage(CMD_OK,"READ TIMEOUT"), blockLink.begin()->second);
        delClientFdFromBlockLink(blockLink.begin()->second);
    }
}
//linux下
#ifdef __linux__
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <ctype.h>


int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void addEvent(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) < 0)
    {
        printf("epoll_ctl Error: %s (errno: %d)\n", strerror(errno), errno);
        exit(-1);
    }
}

void delEvent(int epollfd, int fd)
{
    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL) < 0)
    {
        printf("epoll_ctl Error: %s (errno: %d)\n", strerror(errno), errno);
        exit(-1);
    }
}
void modEvent(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev) < 0)
    {
        printf("epoll_ctl Error: %s (errno: %d)\n", strerror(errno), errno);
        exit(-1);
    }
}

int createServer()
{
    int serverFd;
    struct sockaddr_in st_sersock;
    char msg[MAXSIZE];
    int nrecvSize = 0;

    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0)
    {
        cerr << "create server failed" << endl;
        exit(-1);
    }
    memset(&st_sersock, 0, sizeof(st_sersock));
    st_sersock.sin_family = AF_INET; //IPv4协议
    st_sersock.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    st_sersock.sin_port = htons(SERVER_PORT);
    if (bind(serverFd, (struct sockaddr *)&st_sersock, sizeof(st_sersock)) < 0) //将套接字绑定IP和端口用于监听
    {
        printf("bind Error: %s (errno: %d)\n", strerror(errno), errno);
        exit(-1);
    }

    if (listen(serverFd, MAX_LISTEN) < 0) //设定可同时排队的客户端最大连接个数
    {
        printf("listen Error: %s (errno: %d)\n", strerror(errno), errno);
        exit(-1);
    }

    return serverFd;
}

int createClient()
{
    int clientFd;
    struct sockaddr_in st_sersock;
    char msg[MAXSIZE];

    clientFd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientFd < 0)
    {
        cerr << "create client failed" << endl;
        exit(-1);
    }
    printf("client %d:create success\n", clientFd);
    memset(&st_sersock, 0, sizeof(st_sersock));
    st_sersock.sin_family = AF_INET; //IPv4协议
    st_sersock.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    st_sersock.sin_port = htons(SERVER_PORT);
    if (connect(clientFd, (struct sockaddr *)&st_sersock, sizeof(st_sersock)) < 0)
    {
        perror("connect falied\n");
        exit(1);
    }
    printf("connect success\n");
    setnonblocking(clientFd);
    return clientFd;
}

int recvMsg(int targetFd, char buf[])
{
    int n = recv(targetFd, buf, BUFSIZE, MSG_DONTWAIT);
    return n;
}

int sendMsg(int targetFd, char buf[], int size)
{
    int n = send(targetFd, buf, size, MSG_DONTWAIT);
    return n;
}

void closeSocket(int socketFd)
{
    cout << "socket id:" << socketFd << " is closed" << endl;
    close(socketFd);
}


void polling(int serverFd, Scheduler& scheduler)
{
    defer(
        close(serverFd););
    setnonblocking(serverFd);
    struct epoll_event ev, events[MAXSIZE];
    int epfd, nCounts; //epfd:epoll实例句柄, nCounts:epoll_wait返回值
    epfd = epoll_create(MAXSIZE);
    char buf[MAXSIZE];
    if (epfd < 0)
    {
        printf("epoll_create Error: %s (errno: %d)\n", strerror(errno), errno);
        exit(-1);
    }
    scheduler.epollFd = epfd;
    addEvent(epfd, serverFd, NO_OUT_OP);
    printf("======waiting for client's request======\n");
    int epollDelayTime = 0;
    while (true)
    {
        // 即使没有标识符事件，仍然会唤醒并且将超时consumer返回
        // epollDelayTime = 当前block队列中的最早时间
        epollDelayTime = -1;
        int eventCount = epoll_wait(epfd, events, MAXSIZE, epollDelayTime);
        printf("events num:%d\n", eventCount);
        if (eventCount < 0)
        {
            printf("epoll_ctl Error: %s (errno: %d)\n", strerror(errno), errno);
            exit(-1);
        }
        else if (eventCount == 0)
        {
            printf("no data!\n");
        }
        else
        {
            for (int i = 0; i < eventCount; i++)
            {
                int clientFd = events[i].data.fd;
                cout << "eventFd:" << clientFd << endl;
                //有请求链接
                if (clientFd == serverFd)
                {
                    cout << "建立连接" << endl;
                    int clientFd;
                    if ((clientFd = accept(serverFd, (struct sockaddr *)NULL, NULL)) > 0)
                    {
                        printf("accept Client[%d]\n", clientFd);
                        setnonblocking(clientFd);
                        addEvent(epfd, clientFd, NO_OUT_OP);
                        scheduler.addClient(clientFd);
                    }
                    continue;
                }
                // 传来连接挂断或错误消息时
                if ((events[i].events & EPOLLHUP) || (events[i].events & EPOLLERR))
                {
                    printf("%d client err\n", clientFd);
                    scheduler.delClient(clientFd);
                    delEvent(epfd, clientFd);
                    continue;
                }
                // 当有输入缓冲时
                if (events[i].events & EPOLLIN)
                {
                    cout << "正在拉取缓冲区" << endl;
                    int n = scheduler.clients[clientFd].pull();
                    printf("recv %d byte\n", n);
                    if (n < 0)
                    {
                        printf("%d close link\n", clientFd);
                        scheduler.delClient(clientFd);
                        delEvent(epfd, clientFd);
                        continue;
                    }
                    string s;
                    while (true)
                    {
                        s = scheduler.clients[clientFd].getMessage();
                        cout << "parse message:" << s << endl;
                        cout << "readbuf size:" << scheduler.clients[clientFd].readBuf.size() << endl;
                        cout << "readbuf:";
                        for (int i = 0; i < scheduler.clients[clientFd].readBuf.size(); i++)
                            cout << scheduler.clients[clientFd].readBuf[i];
                        cout << endl;
                        if (s == "")
                            break;
                        scheduler.parse(clientFd, s);
                    }

                }
                // 当有输出缓冲时
                if (events[i].events & EPOLLOUT)
                {
                    // cout << "正在写入缓冲区" << endl;
                    scheduler.clients[clientFd].push();
                    if (scheduler.clients[clientFd].writeBuf.empty())
                        modEvent(epfd, clientFd, NO_OUT_OP);
                }
            }
        }
    }
}

//windows
#elif defined(_WIN32) || defined(_WIN64)

int createClient()
{
    return 1;
}

int recvMsg(int targetFd, char buf[])
{
}

int setnonblocking(int socketFd)
{
}

int sendMsg(int targetFd, char buf[], int n)
{
}

void closeSocket(int socketFd)
{
}

void polling(int serverFd, Scheduler& scheduler)
{
}

#endif
