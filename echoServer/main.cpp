#include "../winsock_help.h"
#include <iostream>
#include <thread>
#include <functional>
#include <atomic>

const short PORT_NUM = 7707;
const short BLOG_SIZE = 5;
const short MAX_MSG_LEN = 256;

std::vector<std::thread> workers;
std::atomic<bool> bCOUT = false;

fd_set read;
fd_set temp;
TIMEVAL timeV;

SOCKET SetTCPServer(short portnum, int blog)
{
	SOCKET sock;
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1) {
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
		return -1;
	}
	re = listen(sock, blog);
	if (re == SOCKET_ERROR) {
		return - 1;
	}

	FD_ZERO(&read);
	FD_SET(sock, &read);
	return sock;
}
void DoWorkLoop(SOCKET dosock)
{
	char msg[MAX_MSG_LEN] = "";
	while (recv(dosock, msg, sizeof(msg), 0) > 0) {
		if (bCOUT == false) {
			bCOUT.store(true);
			printf("recv: %s \n", msg);
			bCOUT.store(false);
		}
		send(dosock, msg, sizeof(msg), 0);
	}
	closesocket(dosock);
}
void DoWork(SOCKET dosock)
{
	char msg[MAX_MSG_LEN] = "";
	auto len = recv(dosock, msg, sizeof(msg), 0);
	if (len == SOCKET_ERROR) {
		errPrint("err");
		FD_CLR(dosock, &read);
		closesocket(dosock);
	} else if(len != 0){
		if (bCOUT == false) {
			bCOUT.store(true);
			printf("recv: %s \n", msg);
			bCOUT.store(false);
		}
		len = send(dosock, msg, sizeof(msg), 0);

		if (len == SOCKET_ERROR) {
			errPrint("err");
			FD_CLR(dosock, &read);
			closesocket(dosock);
		}
	} else {
		FD_CLR(dosock, &read);
		closesocket(dosock);
	}
}

void AcceptLoop(SOCKET sock) 
{
	int re;
	while (1) {
		temp = read;
		timeV.tv_sec = 1;
		timeV.tv_usec = 0;
		re = select(NULL, &temp, NULL, NULL, &timeV);
		if (re == SOCKET_ERROR) {
			break;
		}
		if (re == 0) {
			continue;
		}
		for (int i = 0; i < read.fd_count; ++i) {
			if (FD_ISSET(read.fd_array[i], &temp)) {
				if (sock == read.fd_array[i]) {
					SOCKADDR_IN clientAddr = { 0, };
					int len = sizeof(clientAddr);
					SOCKET dosock = accept(sock, (SOCKADDR*)&clientAddr, &len);
					if (dosock == SOCKET_ERROR) {
						break;
					}
					FD_SET(dosock, &read);
					if (bCOUT == false) {
						bCOUT.store(true);
						printf("%s: %d accept connect [%d in %d] \n", 
							inet_ntoa(clientAddr.sin_addr), 
							ntohs(clientAddr.sin_port),
							(int)dosock,
							read.fd_count);
						bCOUT.store(false);
					}
				}
				else {
					DoWork(read.fd_array[i]);
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
	SOCKET sock = SetTCPServer(PORT_NUM, BLOG_SIZE);
	if (sock != SOCKET_ERROR) {
		AcceptLoop(sock);		
	}
	WSACleanup();
	return 0;
}