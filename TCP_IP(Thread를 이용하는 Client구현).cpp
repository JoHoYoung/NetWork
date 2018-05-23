#include<stdio.h>
#include<WinSock2.h>

CRITICAL_SECTION cs;

DWORD WINAPI hThread(void* socketListen)
{
	char recvBuffer[127];
	int size = sizeof(recvBuffer);
	int recvByte;
	printf("%d", size);
	while (1)
	{
		recvByte=recv((SOCKET)socketListen, recvBuffer, size, 0);
		if (recvByte <1 )
		{
			 
			shutdown((SOCKET)socketListen, SD_BOTH);
			closesocket((SOCKET)socketListen);
			break;

		}
		printf("%s", recvBuffer);
	}
	return NULL;
}

int main()
{
	WSADATA wsa;

	WSAStartup(0x20,&wsa);
	
	SOCKET socketClient;
	socketClient = INVALID_SOCKET;
	socketClient = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	

	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("210.89.160.88");
	servaddr.sin_port = htons(80);

	connect(socketClient, (sockaddr*)&servaddr, sizeof(servaddr));
	HANDLE handler;
	handler = CreateThread(NULL, 0, hThread, (void*)socketClient, THREAD_PRIORITY_NORMAL, 0);

	char sendBuffer[127] = "Test.\n\n";

	if (send(socketClient, sendBuffer, sizeof(sendBuffer),0) == -1)
		printf("Send Fail");

	WaitForSingleObject(handler,INFINITE);

	int a;
	scanf_s("%d", &a);
}