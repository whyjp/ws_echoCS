
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <ctime>

#include "../winsock_help.h"

const short PORT_NUM = 7707;
const short MAX_MSG_LEN = 256;

void connectAndWork(std::string serverIP)
{
	SOCKET sock;
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == SOCKET_ERROR) {
		return;// -1;
	}

	SOCKADDR_IN serverADDR = { 0, };
	serverADDR.sin_family = AF_INET;
	auto ret = InetPtonA(AF_INET, serverIP.c_str(), &serverADDR.sin_addr.S_un.S_addr);
	serverADDR.sin_port = htons(PORT_NUM);

	int re = 0;
	re = connect(sock, (struct sockaddr*)&serverADDR, sizeof(serverADDR));
	if (re == SOCKET_ERROR) {
		return;// -1;
	}
	static int cur = 1;
	char msg[MAX_MSG_LEN] = "";
	printf("connected : %d \n", cur);
	std::stringstream ss;
	ss << std::this_thread::get_id();
	sprintf(msg, "hello- iam %s:%d",
		ss.str().c_str(), cur++);

	char msg_R[MAX_MSG_LEN] = "";
	//gets_s(msg);
	std::srand(static_cast<unsigned int>(std::time(0)));
	rand();

	while (1) {
		int r = static_cast<int>((double)rand() / RAND_MAX * (10 * 1) + 1);
		printf("[%s] sleep for %d \n", ss.str().c_str(), r);
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
			errPrint(msg);
			break;
		}
		else {
			printf("recv: %s\n", msg_R);
		}
	}
	closesocket(sock);
}

int main() {

	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	std::string serverIP = "192.168.164.1";
	//std::cin >> serverIP;

	std::vector<std::thread> workers;
	for (int i = 0; i < 1000; ++i) {
		workers.emplace_back(std::thread(connectAndWork, serverIP));
	}
	while (1) {
		//getchar();
	}
	WSACleanup();
	return 0;
}