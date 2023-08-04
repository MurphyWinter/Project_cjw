#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")

using namespace std;

const int BUF_SIZE = 2048;			//创建缓冲区
char sendbuf[BUF_SIZE];		//发送缓冲区
char recvbuf[BUF_SIZE];		//接受缓冲区

void initialization()
{
	WSADATA wsadata;

	WORD wsaversion = MAKEWORD(2, 2);

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
	SOCKET sockCli;	//创建服务器套接字

	//SOCKADDR_IN是系统封装的一个结构体，具体包含了成员变量：sin_family、sin_addr、sin_zero
	SOCKADDR_IN addrSer;

	//初始化
	initialization();

	//创建套接字
	sockCli = socket(AF_INET, SOCK_STREAM, 0);

	//初始化地址包
	//addrSer.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	inet_pton(AF_INET, "127.0.0.1", &addrSer.sin_addr.s_addr);
	addrSer.sin_family = AF_INET;
	addrSer.sin_port = htons(8000);

	//在建立套接字文件描述符成功后，需要对套接字进行地址和端口的绑定，才能进行数据的接收和发送操作。
	if (connect(sockCli, (SOCKADDR*)&addrSer, sizeof(addrSer)) == SOCKET_ERROR)
	{
		cout << "服务器连接失败" << endl;
		WSACleanup();
	}
	else
	{
		cout << "服务器连接成功" << endl;
	}

	//接受数据:
	while (true)
	{
		cout << "请输入密钥信息：" << endl;
		cin >> sendbuf;
		int sendlen = send(sockCli, sendbuf, sizeof(sendbuf), 0);
		if (sendlen < 0)
		{
			cout << "发送失败！" << endl;
			break;
		}

		int recvlen = recv(sockCli, recvbuf, sizeof(sendbuf), 0);

		if (recvlen < 0)
		{
			cout << "接受失败！" << endl;
			break;
		}
		cout << "收到Server回答:" << recvbuf << endl;
		/* 验证响应的合法性 */
		if (1)   //加判断
			cout << "Server身份确认成功！准备发送加密数据" << endl;
		else
		{
			cout << "Server身份确认失败！" << endl;
			break;
		}
		cout << "发送加密数据：" << endl;

	}
	closesocket(sockCli);
	WSACleanup();
	return 0;
}
