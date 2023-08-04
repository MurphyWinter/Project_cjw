#include <iostream>
#include <thread>
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")

using namespace std;

const int BUF_SIZE = 2048;			//缓冲区大小
char sendbuf[BUF_SIZE];		//发送缓冲区
char recvbuf[BUF_SIZE];		//接受缓冲区

/* 初始化 */
void initialization()
{
	//WSADATA是一种数据结构，用来存储被WSAStartup函数调用后返回的Windows sockets数据，包含Winsock.dll执行的数据。需要头文件
	WSADATA wsadata;

	//MAKEWORD声明调用不同的Winsock版本。例如MAKEWORD(2,2)就是调用2.2版
	WORD wsaversion = MAKEWORD(2, 2);

	//WSAStartup函数必须是应用程序或DLL调用的第一个Windows套接字函数
	//可以进行初始化操作，检测winsock版本与调用dll是否一致，成功返回0
	if (WSAStartup(wsaversion, &wsadata) != 0)
	{
		cout << "载入socket库失败" << endl;
	}
	else
	{
		cout << "初始化成功" << endl;
	}
}

int main()
{
	SOCKET sockSer;	//创建服务器套接字
	SOCKET sockCli;

	//SOCKADDR_IN是系统封装的一个结构体，具体包含了成员变量：sin_family、sin_addr、sin_zero
	SOCKADDR_IN addrSer;
	SOCKADDR_IN addrCli;

	//初始化
	initialization();

	/* 现有的服务器代码 */
	//socket(int domain, int type, int protocol)
	//domain表示要使用的协议AF_INET指ipv4,type指套接字类型 stream指tcp
	//协议“protocol”一般设置为“0”，就是说在已经确定套接字使用的协议簇和类型时，这个参数的值就为0。
	sockSer = socket(AF_INET, SOCK_STREAM, 0);

	//初始化地址包
	//sin_addr主要用来保存IP地址信息
	//htonl 函数可用于将主机字节顺序中的 IPv4 地址转换为网络字节顺序中的 IPv4 地址。
	addrSer.sin_addr.s_addr = htonl(INADDR_ANY);
	//sin_family主要用来定义是哪种地址族
	addrSer.sin_family = AF_INET;
	//sin_port主要用来保存端口号
	addrSer.sin_port = htons(8000);

	// 在建立套接字文件描述符成功后，需要对套接字进行地址和端口的绑定，才能进行数据的接收和发送操作。
	if (bind(sockSer, (SOCKADDR*)&addrSer, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		cout << "套接字绑定失败";
		//调用 WSACleanup 时，将取消此过程中任何线程发出的挂起阻止或异步 Windows 套接字调用，
		//而无需发布任何通知消息，也不发出任何事件对象的信号。
		WSACleanup();
	}
	else
	{
		cout << "绑定套接字成功" << endl;
	}

	//设置监听
	cout << "开始连接" << endl;
	//等待连接最大 0x7fffffff 即32位最大值，成功返回0，失败失败返回SOCKET_ERROR
	if (listen(sockSer, SOMAXCONN) != 0)
	{
		cout << "监听失败！";
		WSACleanup();
	}
	cout << "监听成功" << endl;

	//接受连接:
	//accept将客户端的信息绑定到一个socket上，
	//也就是给客户端创建一个socket，通过返回值返回给我们客户端的socket
	int addrlen = sizeof(SOCKADDR);
	sockCli = accept(sockSer, (SOCKADDR*)&addrCli, &addrlen);
	if (sockCli == SOCKET_ERROR)
	{
		cout << "接受连接失败！" << endl;
		WSACleanup();
		return 0;
	}

	cout << "连接建立，准备接受数据" << endl;
	if (recv(sockCli, recvbuf, sizeof(recvbuf), 0) == -1)
	{
		cout << "接受失败！" << endl;
	}
	else
	{
		cout << "收到Client请求：" << recvbuf << endl;
	}

	while (true)
	{
		/* 回应Client请求 */
		cout << "请输入对Client的回复信息：" << endl;
		cin >> sendbuf;
		if (send(sockCli, sendbuf, sizeof(sendbuf), 0) == SOCKET_ERROR)
		{
			cout << "发送失败！" << endl;
		}

		if (recv(sockCli, recvbuf, sizeof(recvbuf), 0) == -1)
		{
			cout << "接受失败！" << endl;
		}
		cout << "收到Client发送的密钥信息：" << recvbuf << endl;

	}
	closesocket(sockSer);
	closesocket(sockCli);
	WSACleanup();
	return 0;
}
