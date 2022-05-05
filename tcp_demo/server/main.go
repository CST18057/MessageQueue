package main

import (
	"bufio"
	"bytes"
	"encoding/binary"
	"fmt"
	"net"
)

func Encode(message string) ([]byte, error) {
	// 读取消息的长度转换成int32类型（4字节）
	var length = int32(len(message))
	var pkg = new(bytes.Buffer)
	// 写入消息头
	err := binary.Write(pkg, binary.LittleEndian, length)
	if err != nil {
		return nil, err
	}
	// 写入包体
	err = binary.Write(pkg, binary.LittleEndian, []byte(message))
	if err != nil {
		return nil, err
	}
	return pkg.Bytes(), nil
}

func Decode(reader *bufio.Reader) (string, error) {
	// 读消息长度
	lengthByte, _ := reader.Peek(4)
	lengthBuff := bytes.NewBuffer(lengthByte)
	var length int32
	err := binary.Read(lengthBuff, binary.LittleEndian, &length)
	if err != nil {
		return "", err
	}
	// buffer返回缓冲中现有的可读的字节数
	for int32(reader.Buffered()) < length+4 {
		return "", err
	}
	// 读取真正的数据
	pack := make([]byte, int(4+length))
	_, err = reader.Read(pack)
	if err != nil {
		return "", err
	}
	return string(pack[4:]), nil
}

//tcp server demo
func process(conn net.Conn) {
	defer conn.Close()
	//处理完之后要关闭这个连接
	//针对当前的连接做数据的发送和接收操作
	reader := bufio.NewReader(conn)
	msg, err := Decode(reader)
	if err != nil {
		fmt.Println("出现错误:" + msg)
		result, _ := Encode(msg)
		conn.Write(result)
		return
	}
	fmt.Println("接受到的数据：", msg)
	result, _ := Encode("ok")
	conn.Write(result)
}

func main() {
	//1.开始服务
	listen, err := net.Listen("tcp", "127.0.0.1:20000")
	if err != nil {
		fmt.Printf("listen failed,err:%v\n", err)
	}
	for {
		//等待客户端建立连接
		conn, err := listen.Accept()
		if err != nil {
			fmt.Printf("accept failed,err:%v\n", err)
			continue
		}
		//3.启动一个单独的gorountine去处理连接
		go process(conn)
	}
}
