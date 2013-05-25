#ifndef _SERVERCLIENT_
#define _SERVERCLIENT_

#pragma comment(lib, "WS2_32.lib")
#include <winsock2.h>
#include <windows.h>

#ifdef _CSREALISE_
#define EXPORT_CLASS __declspec(dllexport)
#else
#define EXPORT_CLASS __declspec(dllimport)
#endif

// Խ������쳣���ֽ�����, �ͻ���ʹ���������桢�����յ�������
class EXPORT_CLASS BoundArray
{
public:
	BoundArray();
	~BoundArray();
	void append(int len, const char* buf);
	void removeHead(int size);	// ɾ��ͷsize���ֽ�, ��ߵ�����ǰ��
	int getSize() {return mSize;}
	void* at(int pos) {return mBuffer + pos;}

	// ���º����л���� inBound
	char pollchar(int& pos);
	short pollshort(int& pos);
	int pollint(int& pos);

private:
	void inBound(int pos); // mBuffer[pos]�᲻�����? ������׳�int���͵��쳣

	char *mBuffer;
	int mSize;
	int mBufferSize;
};

// �ͻ��˵�����
class EXPORT_CLASS Client
{
public:
	Client();	// ������ʼ��mSocket
	~Client();	// �����close

	bool init(const char *ip, unsigned short port);	// ������������init����
	bool init(unsigned long addr, unsigned short port);	// addr�������ֽ���, �����connect, ������init(mSocket)
	bool init(SOCKET socket);						// �������� startRecvThread

	void close();									// �ر� SOCKET �� waitForClose
	void waitForClose();							// �����ȴ������߳̽���

	bool sendInts(int type, int n, ...);			// �����n��int, ����sendIntArray��ʵ�ַ���
	bool sendIntArray(int type, int n, const int* array);
protected:
	virtual void onRecv(int type, const int *buf)=0;// �û���ʵ��
	virtual void onRecv(int len, const char *buf);	// �������� onRecv(int, const int*)
	virtual void afterClose(){};
private:
	bool startRecvThread();							// ���������һ�������߳�
	friend DWORD WINAPI RecvThread(LPVOID client);	// �������� onRecv(int, const char*), mSocket�رպ��߳��˳�ǰ����afterClose

	HANDLE mThread;
	BoundArray mBuffer;
protected:
	SOCKET mSocket;
};

// ������������
class EXPORT_CLASS Server
{
public:
	Server();	// ����SOCKET
	~Server();	// close

	bool listen(unsigned short port);							// startAcceptThread
	void close();												// �ر� SOCKET �� waitForClose
	void waitForClose();										// �����ȴ�Accept�߳̽���
protected:
	virtual void onAccept(SOCKET socket, sockaddr_in *remote)=0;// �û���ʵ��
	virtual void afterClose(){};
private:
	bool startAcceptThread();
	friend DWORD WINAPI AcceptThread(LPVOID server);			// �������� onAccept

	HANDLE mThread;
	SOCKET mListen;
};

#endif