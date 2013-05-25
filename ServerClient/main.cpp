#define _CSREALISE_
#include "ServerClient.h"

// WinSock ��ʼ��/����
static bool WinSockInit()
{
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	if (WSAStartup(sockVersion, &wsaData) != 0)
		return false;
	return true;
}

static bool WinSockCleanup()
{
	return WSACleanup() == 0;
}

// ���̰�DLLʱ��ʼ��WINSOCK
// ���̽��DLLʱ����WINSOCK
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reasion_for_call, LPVOID lpReserved)
{
	switch(ul_reasion_for_call)
	{
	case DLL_PROCESS_ATTACH:
		WinSockInit();
		break;
	case DLL_PROCESS_DETACH:
		WinSockCleanup();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}
