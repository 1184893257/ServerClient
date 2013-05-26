#define _CSREALISE_
#include "ServerClient.h"

#include <iostream>
#include <stdarg.h>
using namespace std;

Client::Client()
{
	mSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

Client::~Client()
{
	close();
}

bool Client::init(const char *ip, unsigned short port)
{
	return init(inet_addr(ip), port);
}

bool Client::init(unsigned long addr, unsigned short port)
{
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.S_un.S_addr = addr;

	if (connect(mSocket, (sockaddr*)&sin, sizeof(sockaddr)) == -1)
		return false;
	return init(mSocket);
}

bool Client::init(SOCKET socket)
{
	if (mSocket != socket)
	{
		closesocket(mSocket); // 构造时创建的 mSocket 不用了
		mSocket = socket;
	}
	return startRecvThread();
}

void Client::close()
{
	if (mSocket != INVALID_SOCKET)
		closesocket(mSocket);
	waitForClose();
}

void Client::waitForClose()
{
	HANDLE thread = mThread;
	if (thread != NULL)
		WaitForSingleObject(thread, INFINITE);
}

bool Client::sendInts(int type, int n, ...)
{
	va_list ap;
	va_start(ap, n);

	bool success = sendIntArray(type, n, (const int*)ap);

	va_end(ap);
	return success;
}

bool Client::sendIntArray(int type, int n, const int* array)
{
	// 自定义头部
	const int HEADSIZE = 3;
	int len = n + HEADSIZE;
	int *buf = new int[len];
	buf[0] = n;
	buf[1] = ~n;
	buf[2] = type;

	// 数据
	copy(array, array + n, buf + HEADSIZE);

	int sended = send(mSocket, (const char *)buf, len * sizeof(int), 0);

	delete[] buf;
	return sended == (len * sizeof(int));
}

#define BUFSIZE 256

DWORD WINAPI RecvThread(LPVOID lpParam)
{
	Client *client = (Client*)lpParam;

	for(;;)
	{
		char buf[BUFSIZE];
		int ret = recv(client->mSocket, buf, BUFSIZE, 0);
		if (ret == SOCKET_ERROR)
			break;
		client->onRecv(ret, buf);
	}

	client->mSocket = INVALID_SOCKET;
	client->mThread = NULL;
	client->afterClose();
	return 0;
}

bool Client::startRecvThread()
{
	mThread = CreateThread(NULL, 0, RecvThread, (LPVOID)this, 0, NULL);
	if (mThread == NULL)
		return false;
	return true;
}

// 校验头部, 将pos放置到正确的头部的开始处, 校验过程中如果发生异常返回 false
static bool verifyHead(BoundArray& buf, int& pos)
{
	try
	{
		for(;;)
		{
			int pos2 = pos; // pos2是pos的临时拷贝
			int n = buf.pollint(pos2);
			int invn = buf.pollint(pos2);
			if ((n ^ invn) == -1)
				break;
			pos++;	// 每次校验失败前移1个字节
		}
		return true;
	}
	catch (int){}
	return false;
}

void Client::onRecv(int len, const char *buf)
{
	if (len == 0)
		return;

	mBuffer.append(len, buf);
	int pos = 0;
	for(;;) // 一次循环取出一个包, 会 break 的
	{
		if (!verifyHead(mBuffer, pos))
			break;				// 校验 Head 过程中数据不够多
		int pos2 = pos;			// pos2是pos的临时拷贝
		int n = mBuffer.pollint(pos2);
		if (mBuffer.getSize() - pos2 < (n + 2)*sizeof(int))
			break;				// 包不完整

		// 到这一步, 不会再抛异常了, 认为得到了一个正确的包
		pos2 += sizeof(int);	// 跳过校验
		int type = mBuffer.pollint(pos2);
		onRecv(type, (const int*)mBuffer.at(pos2));
		pos = pos2 + n * sizeof(int);
	}

	if (pos)	// pos 是已经扫描过的字节数,但不包括不完整的包
		mBuffer.removeHead(pos);
}