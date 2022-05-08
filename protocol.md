#### 命令格式(命令行格式/json格式)
- 向队列中添加一条数据，必选参数(队列名,消息字符串)
```json
// 命令行格式
// add {queue} {message} 
// queue 队列名
// message 消息字符串
{
    "call":"add",
    "params":
        {
            "queue":"Queue", // 队列名
            "data":"Message" // 消息字符串
        }
}
// 返回结果
{
    "code":0,
    "data": 123, // 返回这条消息对应的id
    "error":"",
}
```
- 向队列中获取消息，必选参数(队列名,消息id)，可选参数(数量,阻塞时间)
```json
// 命令行格式
// read {queue} {message_id} [count=1] [block=-1]
// queue 队列名
// message_id 信息id
// count 需要获取的信息数量
// block 需要阻塞等待的时间
{
    "call":"read",
    "params":
        {
            "queue":"Queue", // 队列名
            "message_id":0, // 大于message_id的消息
            "count":1, // 获取的消息数量，默认1
            "block":0, // -1表示无限阻塞，0表示立刻返回，任意正整数，表示阻塞时长，默认-1
        }
}
// 返回结果
{
    "code":0,
    "data": [ { "message": "Message1", "id": 1 } ], // 返回多条消息组成的列表
    "error":"",
}
```
- 向消费者组中获取消息，必选参数(队列名,消费者组名,消费者名,消息id)，可选参数(数量，阻塞时间)
```json
// 命令行格式
// readgroup {queue} {group} {consumer} {message_id} [count=1] [block=-1]
// queue 队列名
// group 消费者组名
// consumer 消费者名
// message_id 消息id
// count 需要获取的信息数量
// block 需要阻塞等待的时间
{
    "call":"read",
    "params":
        {
            "queue":"Queue", // 队列名
            "group":"Group", // 消费者组名
            "consumer":"Consumer", // 消费者名
            "message_id":0, // 大于message_id的消息，如果使用-1则表示为获取当前消费者组的最新未被获取过的消息
            "count":1, // 获取的消息数量，默认1
            "block":0, // -1表示无限阻塞，0表示立刻返回，任意正整数，表示阻塞时长，默认-1
        }
}
// 返回结果
{
    "code":0,
    "data": [ { "message": "Message1", "id": 1 } ], // 返回多条消息组成的列表
    "error":"",
}
```
- 向消费者组确认一条消息
```json
// 命令行格式
// ack {queue} {group} {message_id} 
// queue 队列名
// group 消费者组名
// message_id 消息id
{
    "call":"ack",
    "params":
        {
            "queue":"Queue", // 队列名
            "group":"Group", // 消费者组名
            "message_id":0, // 等于message_id的消息
        }
}
// 返回结果
{
    "code":0,
    "data": "ack message success:1", // 返回操作是否成功
    "error":"",
}
```
- 创建一个消息队列
```json
// 命令行格式
// createqueue {queue} 
// queue 队列名
{
    "call":"createqueue",
    "params":
        {
            "queue":"Queue", // 队列名
        }
}
// 返回结果
{
    "code":0,
    "data": "create queue success", // 返回操作是否成功
    "error":"",
}
```
- 创建一个消费者组
```json
// 命令行格式
// ack {queue} {gorup} [mkqueue=false]
// queue 队列名
// group 消费者组名
// message_id 消息id
{
    "call":"ack",
    "params":
        {
            "queue":"Queue", // 队列名
            "group":"Group", // 消费者组名
            "mkqueue": true // 如果没有该消息队列，是否创建一个新的消息队列，默认是false
        }
}
// 返回结果
{
    "code":0,
    "data": true, // 返回操作是否成功
    "error":"",
}
```
- 服务器返回命令统一格式
```json
{
    "code":0, // 返回0表示正常返回结果，若非0则意味存在错误
    "data":{}, // 根据实际的命令返回相应的格式
    "error":"",// 若code码不为0，则返回相应的错误
}
```