#include <iostream>
#include "Common.h"
#include "Execution.h"
using namespace std;

static int f(int x) {
	if (x >= 0) {
		WSASetLastError(0);
		return 0;
	}
	else {
		WSASetLastError(WSAEINVAL);
		return SOCKET_ERROR;
	}
}

int CP2_Socketinit() {
	//윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(1, 1), &wsa) != 0) return 1;
	printf("[알림] 윈속 초기화 성공\n");
	cout << "wVersion: 주 버전: " << (int)LOBYTE(wsa.wVersion) << ", 부 버전: " << (int)HIBYTE(wsa.wVersion) << endl;
	cout << "wHighVersion: 주 버전: " << (int)LOBYTE(wsa.wHighVersion) << ", 부 버전: " << (int)HIBYTE(wsa.wHighVersion) << endl;
	cout << "szDescription: " << wsa.szDescription << endl;
	cout << "szSystemStatus: " << wsa.szSystemStatus << endl;

	//소켓 생성
	SOCKET sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET) err_quit("socket()");
	
	SOCKET wsaSockTCP = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (wsaSockTCP == INVALID_SOCKET) err_quit("socket()");

	SOCKET wsaSockUDP = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (wsaSockUDP == INVALID_SOCKET) err_quit("socket()");

	printf("[알림] 소켓 생성 성공\n");

	//소켓 닫기
	if (closesocket(sock) != 0) err_display("closesocket()");
	printf("[알림] 소켓 닫기 성공\n");

	/*int retval = f(-100);
	if (retval == SOCKET_ERROR) err_quit("f()");*/

	//윈속 종료
	WSACleanup();
	system("pause");
	return 0;
}