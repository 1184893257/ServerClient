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

// 越界会抛异常的字节数组, 客户端使用它来缓存、分析收到的数据
class EXPORT_CLASS BoundArray
{
public:
	BoundArray();
	~BoundArray();
	void append(int len, const char* buf);
	void removeHead(int size);	// 删除头size个字节, 后边的数据前移
	int getSize() {return mSize;}
	void* at(int pos) {return mBuffer + pos;}

	// 以下函数中会调用 inBound
	char pollchar(int& pos);
	short pollshort(int& pos);
	int pollint(int& pos);

private:
	void inBound(int pos); // mBuffer[pos]会不会溢出? 溢出会抛出int类型的异常

	char *mBuffer;
	int mSize;
	int mBufferSize;
};

// 客户端的声明
class EXPORT_CLASS Client
{
public:
	Client();	// 这里会初始化mSocket
	~Client();	// 这里会close

	bool init(const char *ip, unsigned short port);	// 这里调用下面的init方法
	bool init(unsigned long addr, unsigned short port);	// addr是网络字节序, 这里会connect, 并调用init(mSocket)
	bool init(SOCKET socket);						// 这里会调用 startRecvThread

	void close();									// 关闭 SOCKET 后 waitForClose
	void waitForClose();							// 这里会等待接收线程结束

	bool sendInts(int type, int n, ...);			// 最后是n个int, 调用sendIntArray来实现发送
	bool sendIntArray(int type, int n, const int* array);
protected:
	virtual void onRecv(int type, const int *buf)=0;// 用户来实现
	virtual void onRecv(int len, const char *buf);	// 这里会调用 onRecv(int, const int*)
	virtual void afterClose(){};
private:
	bool startRecvThread();							// 这里会启动一个接收线程
	friend DWORD WINAPI RecvThread(LPVOID client);	// 这里会调用 onRecv(int, const char*), mSocket关闭后线程退出前调用afterClose

	HANDLE mThread;
	BoundArray mBuffer;
protected:
	SOCKET mSocket;
};

// 服务器的声明
class EXPORT_CLASS Server
{
public:
	Server();	// 创建SOCKET
	~Server();	// close

	bool listen(unsigned short port);							// startAcceptThread
	void close();												// 关闭 SOCKET 后 waitForClose
	void waitForClose();										// 这里会等待Accept线程结束
protected:
	virtual void onAccept(SOCKET socket, sockaddr_in *remote)=0;// 用户来实现
	virtual void afterClose(){};
private:
	bool startAcceptThread();
	friend DWORD WINAPI AcceptThread(LPVOID server);			// 这里会调用 onAccept

	HANDLE mThread;
	SOCKET mListen;
};

#endif