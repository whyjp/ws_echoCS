#pragma once

#define FD_SETSIZE 5096
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <process.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <malloc.h>
#include <atomic>
#include <mutex>

#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

//IN_ADDR GetMyIP()
//{
//	char localhostname[MAX_PATH] = { 0, };
//
//	IN_ADDR addr = { 0, };
//	if (gethostname(localhostname, MAX_PATH) != SOCKET_ERROR) {
//		HOSTENT* ptr = gethostbyname(localhostname);
//		while (ptr && ptr->h_name) {
//			if (ptr->h_addrtype == PF_INET) {
//				memcpy(&addr, ptr->h_addr_list[0], ptr->h_length);
//				break;
//			}
//			ptr++;
//		}
//	}
//	return addr;
//}

std::mutex errPrint_Mutex;
void errPrint(std::string msg)
{
	std::lock_guard<std::mutex> ggM(errPrint_Mutex);
	auto err = WSAGetLastError();
	LPVOID lpMsgBuf;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
		err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf, 0, NULL);
	printf("--------- [%s:%d] %s \n", msg.c_str(), err, (LPCSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}