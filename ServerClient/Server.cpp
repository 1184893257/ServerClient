#define _CSREALISE_
#include "ServerClient.h"

#include <iostream>
using namespace std;

Server::Server()
{
	mListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

Server::~Server()
{
	close();
}

bool Server::listen(unsigned short port)
{
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	
	if (bind(mListen, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
		return false;
	if (::listen(mListen, 5) == SOCKET_ERROR)
		return false;
	return startAcceptThread();
}

void Server::close()
{
	if (mListen != INVALID_SOCKET)
		closesocket(mListen);
	waitForClose();
}

void Server::waitForClose()
{
	HANDLE thread = mThread;
	if (thread != NULL)
		WaitForSingleObject(thread, INFINITE);
}

DWORD WINAPI AcceptThread(LPVOID lpParam)
{
	Server *server = (Server*)lpParam;
	int nAddrLen = sizeof(sockaddr_in);

	for(;;)
	{
		SOCKET client;
		sockaddr_in remoteAddr;
		client = accept(server->mListen, (SOCKADDR*)&remoteAddr, &nAddrLen);
		if (client == INVALID_SOCKET)
			break;
		server->onAccept(client, &remoteAddr);
	}

	server->mListen = INVALID_SOCKET;
	server->mThread = NULL;
	server->afterClose();
	return 0;
}

bool Server::startAcceptThread()
{
	mThread = CreateThread(NULL, 0, AcceptThread, (LPVOID)this, 0, NULL);
	if (mThread == NULL)
		return false;
	return true;
}