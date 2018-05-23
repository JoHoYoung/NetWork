#include <stdio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>

#include "wtpipv6.h"
#include "wspiapi.h"

#include "Packet.h"

#define MAX_MESSAGE 1000
int userinfo = -1;
void ResetString(char* arr)
{
	int len = strlen(arr);
	userinfo = arr[0]-48;

		for (int i = 1; i < len; i++)
			arr[i - 1] = arr[i];

		arr[len - 1] = '\n';
}


DWORD WINAPI NetReceive(LPVOID socketConnect)
{
	char recvBuffer[PACKETBUFFERSIZE];
	int  RecvBytes;

	Packet	recvPacket;
	char	receiveBuffer[PACKETBUFFERSIZE];
	int		receivedPacketSize = 0;


	while (1) {

		int bufSize = PACKETBUFFERSIZE - receivedPacketSize;
		RecvBytes = ::recv((SOCKET)socketConnect, &receiveBuffer[receivedPacketSize], bufSize, 0);
		if (RecvBytes<1)
			break;
		
		receivedPacketSize += RecvBytes;
		while (receivedPacketSize > 0)
		{
			recvPacket.copyToBuffer(receiveBuffer, receivedPacketSize);
			int packetlength = (int)recvPacket.getPacketSize();

			if (receivedPacketSize >= packetlength)
			{
				recvPacket.readData(recvBuffer, recvPacket.getDataFieldSize());

				ResetString(recvBuffer);

				printf("%d Bytes Received Form User %d=> %s",recvPacket.getDataFieldSize(), userinfo,recvBuffer);
				receivedPacketSize -= packetlength;
				if (receivedPacketSize > 0)
				{
					CopyMemory(recvBuffer, (receiveBuffer + recvPacket.getPacketSize()), receivedPacketSize);
					CopyMemory(receiveBuffer, recvBuffer, receivedPacketSize);
				}

			}
			else
				break;
		}
		if (recvBuffer[RecvBytes - 2] == '.')
			break;
	}
	return NULL;
}



void main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET socketConnect;
	struct sockaddr_in serverAddr;
	HANDLE handleThread;

	::WSAStartup(0x202, &wsaData);


		socketConnect = INVALID_SOCKET;
		socketConnect = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (socketConnect == INVALID_SOCKET)
		{
			printf("Cannot create socket !!\n");
		}
	
//  접속할 서버의 정보를 설정한다.
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::inet_addr("127.0.0.1");
	serverAddr.sin_port = ::htons(8600);

	//********************************************************
		
	if (::connect(socketConnect, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		printf("Cannot connect to server !!\n");
		socketConnect = INVALID_SOCKET;
	}
			else {
				// create thread for receive
		handleThread=	CreateThread(NULL, 0, NetReceive, (void *)socketConnect, THREAD_PRIORITY_NORMAL, NULL);
			}
		
	

	Packet sendPacket;

	//  서버와 통신을 한다.
	int count = 0;
	while (count++ < MAX_MESSAGE) {
		char sendBuffer[127];
		int sentBytes;

		if (socketConnect != INVALID_SOCKET) {
			fgets(sendBuffer, 127, stdin);

			if (count == MAX_MESSAGE)
				strcat(sendBuffer, ".");

			///////////////////////////////////////////////////////////

			sendPacket.clear();
			sendPacket.id(1000 + count);
			sendPacket.writeData(sendBuffer, strlen(sendBuffer) + 1);
			sentBytes = ::send(socketConnect, sendPacket.getPacketBuffer(), sendPacket.getPacketSize(), 0);
			///////////////////////////////////////////////////////////

			if (sentBytes < 0) {
				::shutdown(socketConnect, SD_BOTH);
				::closesocket(socketConnect);
				socketConnect = INVALID_SOCKET;
			}
		}

	}
	
	if(socketConnect!=INVALID_SOCKET)
	::WaitForSingleObject(handleThread, INFINITE);
	::shutdown(socketConnect, SD_BOTH);
	::closesocket(socketConnect);
	::WSACleanup();

	printf("Server Connection Closed !\n");
	char temp[120];
	fgets(temp, 119, stdin);
}
