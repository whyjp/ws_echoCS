
#include <iostream>
#include <string>
#include <thread>
#include <ctime>

#include "../winsock_help.h"

const short PORT_NUM = 7707;
const short MAX_MSG_LEN = 256;

void errPrint(std::string msg)
{
	auto err = WSAGetLastError();
	LPVOID lpMsgBuf;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
		err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf, 0, NULL);
	printf("[%s:%d] %s", msg, err, (LPCSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
void connectAndWork(std::string serverIP)
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	SOCKET sock;
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == SOCKET_ERROR) {
		return;// -1;
	}

	SOCKADDR_IN serverADDR = { 0, };
	serverADDR.sin_family = AF_INET;
	serverADDR.sin_addr.S_un.S_addr = inet_addr(serverIP.c_str());
	serverADDR.sin_port = htons(PORT_NUM);
	int re = 0;
	re = connect(sock, (struct sockaddr*)&serverADDR, sizeof(serverADDR));
	if (re == SOCKET_ERROR) {
		return;// -1;
	}
	static int cur = 1;
	char msg[MAX_MSG_LEN] = "";
	sprintf(msg, "hello- iam %d", cur++);

	char msg_R[MAX_MSG_LEN] = "";
	//gets_s(msg);
	std::srand(std::time(0));
	rand();
	while (1) {
		int r = (double)rand() / RAND_MAX * (15 * 1) + 1;
		std::this_thread::sleep_for(std::chrono::seconds(r));

		//gets_s(msg, MAX_MSG_LEN);
		if (send(sock, msg, sizeof(msg), 0) == SOCKET_ERROR) {
			errPrint(msg);
			break;
		}
		printf("send: %s\n", msg);
		if (strcmp(msg, "exit") == 0) {
			break;
		}
		memset(msg_R, 0, MAX_MSG_LEN);
		if (recv(sock, msg_R, sizeof(msg_R), 0) == SOCKET_ERROR) {
			errPrint(msg);
			break;
		}
		if (strcmp(msg_R, "") == 0) {
			auto err = WSAGetLastError();
			LPVOID lpMsgBuf;
			FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
				err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPSTR)&lpMsgBuf, 0, NULL);
			printf("[%s:%d] %s", msg, err, (LPCSTR)lpMsgBuf);
			LocalFree(lpMsgBuf);
		}
		else {
			printf("recv: %s\n", msg_R);
		}
	}
	closesocket(sock);
	WSACleanup();
}

int main() {
	
	std::string serverIP;
	std::cin >> serverIP;

	std::vector<std::thread> workers;
	for (int i = 0; i < 1000; ++i) {
		workers.emplace_back(std::thread(connectAndWork, serverIP));
	}
	while (1) {
		//getchar();
	}
	return 0;
}