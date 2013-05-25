#include "../ServerClient/ServerClient.h"
#include <iostream>
using namespace std;

Client *demoClient;
Server *demoServer;
Client *demoServerClient;

class DemoClient : public Client
{
protected:
	void onRecv(int type, const int *buf)
	{
		switch(type)
		{
		case 0:
			cout<<"client received : "<<buf[0]<<endl;
			if (buf[0] > 10)
			{
				closesocket(mSocket);
				return;
			}
			Sleep(1000);
			sendInts(0, 1, buf[0] + 1);
			break;
		}
	}
};

class DemoServer : public Server
{
protected:
	void onAccept(SOCKET socket, sockaddr_in *remote)
	{
		demoServerClient->init(socket);
	}
};

class DemoServerClient : public Client
{
protected:
	void onRecv(int type, const int *buf)
	{
		switch(type)
		{
		case 0:
			cout<<"serverclient received : "<<buf[0]<<endl;
			Sleep(1000);
			sendInts(0, 1, buf[0] + 1);
			break;
		}
	}
};

int main()
{
	const int port = 3000;
	demoServer = new DemoServer();
	demoClient = new DemoClient();
	demoServerClient = new DemoServerClient();

	demoServer->listen(port);
	demoClient->init("127.0.0.1", port);
	demoClient->sendInts(0, 1, 0);

	demoClient->waitForClose();
	delete demoServer;
	delete demoClient;
	delete demoServerClient;
	return 0;
}