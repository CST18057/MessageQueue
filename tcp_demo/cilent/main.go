package main

import (
	"bufio"
	"bytes"
	"encoding/binary"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"net"
	"net/http"
)

//tcp cilent demo
type Result struct {
	Code  int         `json:"code"`
	Error string      `json:"error"`
	Data  interface{} `json:"data"`
}
type Package struct {
	Call   string                 `json:"call"`
	Params map[string]interface{} `json:"params"`
}

func Encode(message string) ([]byte, error) {
	// 读取消息的长度转换成int32类型（4字节）
	var length = int32(len(message))
	var pkg = new(bytes.Buffer)
	// 写入消息头
	err := binary.Write(pkg, binary.BigEndian, length)
	if err != nil {
		return nil, err
	}
	// 写入包体
	err = binary.Write(pkg, binary.BigEndian, []byte(message))
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
	err := binary.Read(lengthBuff, binary.BigEndian, &length)
	if err != nil {
		fmt.Printf("%v\n", err)
		return `{"code":17,"error":"bytes to int error"}`, err
	}
	// buffer返回缓冲中现有的可读的字节数
	for int32(reader.Buffered()) < length+4 {
		return `{"code":27,"error":"buffered size less than package size"}`, err
	}
	// 读取真正的数据
	pack := make([]byte, int(4+length))
	_, err = reader.Read(pack)
	if err != nil {
		return `{"code":37,"error":"read package error"}`, err
	}
	return string(pack[4:]), nil
}
func ServeHTTP(w http.ResponseWriter, r *http.Request) {
	if r.Method == "GET" {
		return
	}
	w.Header().Set("content-type", "application/json")
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		fmt.Printf("ioutil read error:%v\n", err)
		return
	}
	p := &Package{}
	if err = json.Unmarshal(body, &p); err != nil {
		fmt.Printf("json unmarshal error:%v\n", err)
		return
	}
	jsonP, _ := json.Marshal(p)
	fmt.Println(string(jsonP))
	message, err := Encode(string(jsonP))
	if err != nil {
		fmt.Println("Encode失败，err:", err)
	}
	conn, err := net.Dial("tcp", "127.0.0.1:18057")
	if err != nil {
		fmt.Printf("dial failed err:%v\n", err)
		return
	}
	_, err = conn.Write(message)
	if err != nil {
		fmt.Printf("send failed,err:%v\n", err)
		return
	}
	//从服务端接受回复的消息
	reader := bufio.NewReader(conn)
	msg, err := Decode(reader)
	if err != nil {
		fmt.Println("出现错误:" + msg)
		w.Write([]byte(msg))
		return
	}
	fmt.Println("接受到的数据：", msg)
	w.Write([]byte(msg))
}
func ListenAndServe() {
	files := http.FileServer(http.Dir("./"))
	http.Handle("/", files)
	http.HandleFunc("/server", ServeHTTP)
	err := http.ListenAndServe(":8085", nil)
	if err != nil {
		log.Fatal("ListenAndServe", err)
	}
}
func main() {
	ListenAndServe()
	//中转站:80
	//前端--->中转站---->封装--->发送消息队列/服务端--->Result--->前端
	//1.与服务端建立连接
	// conn, err := net.Dial("tcp", "127.0.0.1:20000")
	// if err != nil {
	// 	fmt.Printf("dial failed err:%v\n", err)
	// 	return
	// }
	//2.利用该连接进行数据的发送和接收
	// p := &Package{
	// 	Call: "read",
	// 	Params: map[string]interface{}{
	// 		"queue":      "Queue", // 队列名
	// 		"message_id": 0,       // 大于message_id的消息
	// 		"count":      1,       // 获取的消息数量，默认1
	// 		"block":      0,
	// 	},
	// }
	// jsonP, _ := json.Marshal(p)
	// fmt.Println(string(jsonP))
	// for {
	// 	//给服务端发消息
	// 	_, err := conn.Write([]byte(s))
	// 	if err != nil {
	// 		fmt.Printf("send failed,err:%v\n", err)
	// 		return
	// 	}
	// 	//从服务端接受回复的消息
	// 	var buf [1024]byte
	// 	n, err := conn.Read(buf[:])
	// 	if err != nil {
	// 		fmt.Printf("read failed,err%v\n", err)
	// 	}
	// 	fmt.Println("收到服务端回复：", string(buf[:n]))
	// }
}
