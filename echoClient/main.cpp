
#include <iostream>
#include <string>
#include <thread>
#include <ctime>

#include "../winsock_help.h"

const short PORT_NUM = 7707;
const short MAX_MSG_LEN = 256;

int main() {
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	SOCKET sock;
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == SOCKET_ERROR) {
		return -1;
	}
	std::string serverIP;
	std::cin >> serverIP;

	SOCKADDR_IN serverADDR = { 0, };
	serverADDR.sin_family = AF_INET;
	serverADDR.sin_addr.S_un.S_addr = inet_addr(serverIP.c_str());
	serverADDR.sin_port = htons(PORT_NUM);
	int re = 0;
	re = connect(sock, (struct sockaddr*)&serverADDR, sizeof(serverADDR));
	if (re == SOCKET_ERROR) {
		return -1;
	}
	char msg[MAX_MSG_LEN] = "hello";
	//gets_s(msg);
	std::srand(std::time(0));
	rand();
	while (1) {
		int r = (double)rand() / RAND_MAX * (5 * 2) + 2;
		std::this_thread::sleep_for(std::chrono::seconds(r));
		
		//gets_s(msg, MAX_MSG_LEN);
		send(sock, msg, sizeof(msg), 0);
		if (strcmp(msg, "exit") == 0){
			break;
		}
		recv(sock, msg, sizeof(msg), 0);
		printf("recv: %s\n", msg);
	}
	closesocket(sock);
	WSACleanup();
	return 0;
}