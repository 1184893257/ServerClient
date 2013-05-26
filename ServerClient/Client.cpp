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
		closesocket(mSocket); // ����ʱ������ mSocket ������
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
	// �Զ���ͷ��
	const int HEADSIZE = 3;
	int len = n + HEADSIZE;
	int *buf = new int[len];
	buf[0] = n;
	buf[1] = ~n;
	buf[2] = type;

	// ����
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

// У��ͷ��, ��pos���õ���ȷ��ͷ���Ŀ�ʼ��, У���������������쳣���� false
static bool verifyHead(BoundArray& buf, int& pos)
{
	try
	{
		for(;;)
		{
			int pos2 = pos; // pos2��pos����ʱ����
			int n = buf.pollint(pos2);
			int invn = buf.pollint(pos2);
			if ((n ^ invn) == -1)
				break;
			pos++;	// ÿ��У��ʧ��ǰ��1���ֽ�
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
	for(;;) // һ��ѭ��ȡ��һ����, �� break ��
	{
		if (!verifyHead(mBuffer, pos))
			break;				// У�� Head ���������ݲ�����
		int pos2 = pos;			// pos2��pos����ʱ����
		int n = mBuffer.pollint(pos2);
		if (mBuffer.getSize() - pos2 < (n + 2)*sizeof(int))
			break;				// ��������

		// ����һ��, ���������쳣��, ��Ϊ�õ���һ����ȷ�İ�
		pos2 += sizeof(int);	// ����У��
		int type = mBuffer.pollint(pos2);
		onRecv(type, (const int*)mBuffer.at(pos2));
		pos = pos2 + n * sizeof(int);
	}

	if (pos)	// pos ���Ѿ�ɨ������ֽ���,���������������İ�
		mBuffer.removeHead(pos);
}