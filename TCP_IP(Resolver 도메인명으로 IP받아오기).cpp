#include <stdio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>

#include "wtpipv6.h"
#include "wspiapi.h"


#define MAX_CLIENT 3
#define MAX_MESSAGE 3

//**********************************************************************
void usage(char *progname)
{
	fprintf(stderr, "usage: %s [-n name] [-p port] \n", progname);
	fprintf(stderr, "   -n name    Host name to resolve, [127.0.0.1] \n");
	fprintf(stderr, "   -p port    Port number to resolve, [8600] \n");

	ExitProcess(-1);
}

DWORD WINAPI NetReceive(LPVOID socketConnect)
{
	char recvBuffer[127];
	int  RecvBytes;


	while (1) {
		RecvBytes = ::recv((SOCKET)socketConnect, recvBuffer, 127, 0);
		if (RecvBytes<1)
			break;
		printf("%d> %d : %s\n", socketConnect, RecvBytes, recvBuffer);

		if (recvBuffer[RecvBytes - 2] == '.')
			break;
	}

	return NULL;
}

//************************************************************************
// insert you code here to get numerical address
int resolveAddr(int argc, char **argv, char *serverName, char *serverPort)
{
	WSADATA          wsd;
	struct addrinfo hints, *res = NULL;

	char *HostName = NULL;

	WSAStartup(MAKEWORD(2, 2), &wsd);

	HostName = argv[1];
	serverPort = argv[2];

	memset(&hints, 0, sizeof(hints));

	hints.ai_flags = NULL;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	::getaddrinfo(HostName, ServPort, &hints, &res);


	::getnameinfo(res->ai_addr, res->ai_addrlen, serverName, 120, NULL, 0, NULL);
	printf("Domain name resolve to addresses %s \n\n", numerichost);

	freeaddrinfo(res);
	WSACleanup();
	system("pause");

	return 1;
}
//************************************************************************


void main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET socketConnect[MAX_CLIENT];
	struct sockaddr_in serverAddr;
	int  k;
	HANDLE handleThread[MAX_CLIENT];


	::WSAStartup(0x202, &wsaData);


	for (k = 0; k<MAX_CLIENT; k++) {
		socketConnect[k] = INVALID_SOCKET;
		socketConnect[k] = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (socketConnect[k] == INVALID_SOCKET)
		{
			printf("Cannot create socket %d !!\n", k);
			continue;
		}
	}



	//******************************* Address and port resolve
	char serverName[120], serverPort[120];                        //여기에 저장
	resolveAddr(arc, arv, serverName, serverPort);
	if (resolveAddr(argc, argv, serverName, serverPort)<1) {
		printf("*** Unable to resolve server name !\n");
		ExitProcess(-1);
	}


	//  접속할 서버의 정보를 설정한다.
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::inet_addr(serverName);
	serverAddr.sin_port = ::htons(atoi(serverPort));

	//********************************************************

	for (k = 0; k<MAX_CLIENT; k++) {
		if (socketConnect[k] != INVALID_SOCKET) {
			if (::connect(socketConnect[k], (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
			{
				printf("Cannot connect to server %d !!\n", k);
				socketConnect[k] = INVALID_SOCKET;
				continue;
			}
			else {
				// create thread for receive
				handleThread[k] = CreateThread(NULL, 0, NetReceive, (void *)socketConnect[k], THREAD_PRIORITY_NORMAL, NULL);
			}
		}
	}



	//  서버와 통신을 한다.
	int count = 0;
	while (count++ < MAX_MESSAGE) {
		char sendBuffer[127];
		int sentBytes;

		for (k = 0; k<MAX_CLIENT; k++) {

			if (socketConnect[k] != INVALID_SOCKET) {
				sprintf(sendBuffer, "%d> Test Message to server", k);
				if (count == MAX_MESSAGE) {
					strcat(sendBuffer, ".");
					if (strcmp(serverPort, "80") == 0)
						strcat(sendBuffer, "\n\n");
				}

				sentBytes = ::send(socketConnect[k], sendBuffer, ::strlen(sendBuffer) + 1, 0);
				if (sentBytes<0) {
					::shutdown(socketConnect[k], SD_BOTH);
					::closesocket(socketConnect[k]);
					socketConnect[k] = INVALID_SOCKET;
				}
			}
		}

	}


	::WaitForMultipleObjects(MAX_CLIENT, handleThread, TRUE, INFINITE);


	for (k = 0; k<MAX_CLIENT; k++) {
		::shutdown(socketConnect[k], SD_BOTH);
		::closesocket(socketConnect[k]);
	}
	::WSACleanup();

	printf("Server Connection Closed !\n");
	char temp[120];
	fgets(temp, 118, stdin);
}
