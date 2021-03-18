#pragma once

#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <malloc.h>

#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

IN_ADDR GetMyIP()
{
	char localhostname[MAX_PATH] = { 0, };

	IN_ADDR addr = { 0, };
	if (gethostname(localhostname, MAX_PATH) != SOCKET_ERROR) {
		HOSTENT* ptr = gethostbyname(localhostname);
		while (ptr && ptr->h_name) {
			if (ptr->h_addrtype == PF_INET) {
				memcpy(&addr, ptr->h_addr_list[0], ptr->h_length);
				break;
			}
			ptr++;
		}
	}
	return addr;
}