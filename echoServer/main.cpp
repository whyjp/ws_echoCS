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
	return sock;
}
void DoWork(SOCKET dosock)
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

void AcceptLoop(SOCKET sock) 
{
	SOCKET dosock;
	SOCKADDR_IN clientAddr = { 0, };
	int len = sizeof(clientAddr);
	while (1) {
		dosock = accept(sock, (SOCKADDR*)&clientAddr, &len);
		if (dosock == SOCKET_ERROR) {
			break;
		}
		if (bCOUT == false) {
			bCOUT.store(true);
			printf("%s: %d accept connect \n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
			bCOUT.store(false);
		}
		workers.emplace_back(std::thread(std::bind(&DoWork, dosock)));
	}
	closesocket(sock);
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