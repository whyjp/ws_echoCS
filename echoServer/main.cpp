#include "../winsock_help.h"
#include <iostream>
#include <thread>
#include <functional>
#include <atomic>
#include <mutex>

const short PORT_NUM = 7707;
const short BLOG_SIZE = 5;
const short MAX_MSG_LEN = 256;

std::vector<std::thread> workers;
std::mutex mutexG;

HANDLE hIOCP;
struct skeyInfo {
	SOCKET socket;
	SOCKADDR_IN clientAddr;
	std::string ipADDR;
	int port;
};
struct sInfo {
	WSAOVERLAPPED overlapped;
	WSABUF dataBuffer;
	SOCKET socket;
	char msg[MAX_MSG_LEN] = "";
	int bytes = 0;
	int myNum = 0;
};
void WorkerThread(HANDLE hIOCP)
{
	SOCKET dosock;
	DWORD receiveBytes;
	DWORD bytesSend;
	sInfo* info;
	DWORD flags = 0;
	skeyInfo* keyInfo;

	while (1) {
		if (GetQueuedCompletionStatus(hIOCP, &receiveBytes, (PULONG_PTR)&keyInfo, (LPOVERLAPPED*)&info, INFINITE)) {
			dosock = keyInfo->socket;
			{
				std::lock_guard<std::mutex> lock_guard(mutexG);
				printf("%s: %d recv from queued [%d] \n",
					keyInfo->ipADDR.c_str(),
					keyInfo->port,
					(int)keyInfo->socket);
				if (receiveBytes == SOCKET_ERROR) {
					errPrint("err");
					closesocket(dosock);
				}
				else if (receiveBytes != 0) {
					printf("recv: %s  [ num : %d ]\n", info->msg, info->myNum);

					auto len = WSASend(dosock, (LPWSABUF)&info->dataBuffer, 1, &bytesSend, 0, NULL, NULL);

					if (len == SOCKET_ERROR) {
						if (WSAGetLastError() != WSA_IO_PENDING) {
							errPrint("WSASend_ERROR");
							closesocket(dosock);
							break;
						}
					}
					WSARecv(dosock, &info->dataBuffer, 1, &receiveBytes, &flags, &info->overlapped, NULL);

					if (len == SOCKET_ERROR) {
						if (WSAGetLastError() != WSA_IO_PENDING) {
							errPrint("WSASend_ERROR");
							closesocket(dosock);
							break;
						}
					}
				}
			}
		}
		else {
			errPrint("err");
			closesocket(dosock);
			break;
		}
	}
}
void initThreadPOOL()
{
	hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hIOCP == NULL) {
		errPrint("IOCP init Failed");
		return;
	}
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	for (DWORD i = 0; i < sysInfo.dwNumberOfProcessors*2; ++i) {
		workers.emplace_back(std::thread(&WorkerThread, hIOCP));
	}
}
SOCKET SetTCPServer(short portnum, int blog)
{
	SOCKET sock;
	sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,WSA_FLAG_OVERLAPPED);
	if (sock == INVALID_SOCKET) {
		errPrint("WSASocket ERR");
		return -1;
	}
	SOCKADDR_IN serverAddr = { 0, };
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr = GetMyIP();
	serverAddr.sin_port = htons(portnum);

	std::cout << inet_ntoa(serverAddr.sin_addr);

	int re = 0;
	re = bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (re == SOCKET_ERROR) {
		errPrint("bind ERR");
		return -1;
	}
	re = listen(sock, blog);
	if (re == SOCKET_ERROR) {
		errPrint("listen ERR");
		return - 1;
	}
	return sock;
}

//iocp 에서는 accept 된 socket 을 iocp 와 연결해주며 async recv 를 호출해두어 대기상태로 돌린다
void AcceptLoop(SOCKET sock) 
{
	sInfo* info;
	DWORD flags=0;
	DWORD receiveBytes=0;
	skeyInfo* keyInfo;

	while (1) {
		SOCKADDR_IN clientAddr = { 0, };
		int len = sizeof(clientAddr);
		SOCKET dosock = accept(sock, (SOCKADDR*)&clientAddr, &len);
		if (dosock == INVALID_SOCKET) {
			break;
		} 
		{
			std::lock_guard<std::mutex> lock_guard(mutexG);
				static int nALL = 1;
				printf("%s: %d accept connect [%d in %d] \n",
					inet_ntoa(clientAddr.sin_addr),
					ntohs(clientAddr.sin_port),
					(int)dosock,
					nALL++);

			info = new sInfo();
			info->socket = dosock;
			info->dataBuffer.buf = info->msg;
			info->dataBuffer.len = MAX_MSG_LEN;
			info->myNum = nALL - 1;

			keyInfo = new skeyInfo();
			keyInfo->socket = dosock;
			keyInfo->ipADDR = inet_ntoa(clientAddr.sin_addr);
			keyInfo->port = clientAddr.sin_port;
			keyInfo->clientAddr = clientAddr;

			CreateIoCompletionPort((HANDLE)dosock, hIOCP, (DWORD)keyInfo, 0);
			auto ret = WSARecv(info->socket, &info->dataBuffer, 1, &receiveBytes, &flags, &info->overlapped, NULL);
			if (ret == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING) {
					errPrint("firstWSARecv_ERROR");
					break;
				}
			}
		}
	}
	closesocket(sock);
	return;
}

int main()
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	initThreadPOOL();
	SOCKET sock = SetTCPServer(PORT_NUM, BLOG_SIZE);
	if (sock != SOCKET_ERROR) {
		AcceptLoop(sock);		
	}
	WSACleanup();
	return 0;
}