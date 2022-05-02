package main

import (
	"bufio"
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"net"
	"net/http"
)

// 解码
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
	if int32(reader.Buffered()) < length+4 {
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

func Handler(w http.ResponseWriter, r *http.Request) {
	io.WriteString(w, "Hello World")
}

//tcp server demo
func process(conn net.Conn) {
	defer conn.Close()
	//处理完之后要关闭这个连接
	//针对当前的连接做数据的发送和接收操作
	reader := bufio.NewReader(conn)
	for {
		msg, err := Decode(reader)
		if err != nil {
			return
		}
		fmt.Println("接受到的数据：", msg)
		conn.Write([]byte("ok"))
	}
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
