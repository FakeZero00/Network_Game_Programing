#include "Common.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>
using namespace std;

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024	//1KB

//임계영역 객체 생성
CRITICAL_SECTION cs;

//콘솔의 가장 아래쪽 커서 위치를 저장하는 변수
CONSOLE_SCREEN_BUFFER_INFO csbi;

void PrintProgress(SHORT y_pos, char* clientAddr, float percent) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	EnterCriticalSection(&cs);
	GetConsoleScreenBufferInfo(hConsole, &csbi);
	COORD currentPos = csbi.dwCursorPosition;
	//커서를 가장 아래쪽으로 이동
	COORD printPos = { 0, y_pos };
	SetConsoleCursorPosition(hConsole, printPos);
	cout << fixed;
	cout.precision(2);
	cout << "\r" << "[" << clientAddr << "] 클라이언트 수신 진행률: " << percent << "%" << flush;
	//커서를 원래 위치로 되돌림
	SetConsoleCursorPosition(hConsole, currentPos);
	LeaveCriticalSection(&cs);
}

DWORD WINAPI ProcessClient(LPVOID arg) {
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	int addrlen;
	int retval;		//에러 체크용 변수
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO logCursor;
	SHORT row;

	addrlen = sizeof(clientaddr);
	char addr[INET_ADDRSTRLEN];
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
	
	//데이터 송수신에 사용할 변수
	vector<char> buffer;
	long long fileLen;
	char filename[256];

	//파일 이름 수신
	EnterCriticalSection(&cs);


	retval = recv(client_sock, (char*)filename, sizeof(char) * 256, MSG_WAITALL);
	if (retval == SOCKET_ERROR) {
		err_display("recv()");
		closesocket(client_sock);
		return 1;
	}
	cout << "파일 이름 수신 성공: " << filename << endl;

	//파일 사이즈 수신
	retval = recv(client_sock, (char*)&fileLen, sizeof(long long), MSG_WAITALL);
	if (retval == SOCKET_ERROR) {
		err_display("recv()");
		closesocket(client_sock);
		return 1;
	}
	cout << "파일 사이즈 수신 성공: " << fileLen << " bytes" << endl;
	LeaveCriticalSection(&cs);

	buffer.resize(fileLen); //버퍼 크기 예약

	//수신 진행률 출력할 커서 위치 저장
	EnterCriticalSection(&cs);
	GetConsoleScreenBufferInfo(hConsole, &logCursor);
	row = logCursor.dwCursorPosition.Y;
	COORD logPosDown = { 0, row + 1 };
	SetConsoleCursorPosition(hConsole, logPosDown);
	LeaveCriticalSection(&cs);

	//실제 데이터 수신
	int totalReceived = 0;
	while (totalReceived < fileLen) {
		//수신할 데이터 크기 수신
		int len;
		retval = recv(client_sock, (char*)&len, sizeof(int), MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			return 1;
		}
		else if (retval == 0) return 1;

		//수신한 데이터 크기만큼 버퍼에 추가
		retval = recv(client_sock, buffer.data() + totalReceived, len, MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			return 1;
		}
		else if (retval == 0) return 1;

		totalReceived += len;
		
		PrintProgress(row, addr, (float)totalReceived / fileLen * 100);
	}

	//수신한 데이터 출력
	ofstream outFile(filename, ios::binary);
	if (!outFile.is_open()) {
		err_display("파일을 작성할 수 없습니다.");
		return 1;
	}
	EnterCriticalSection(&cs);
	outFile.write(buffer.data(), buffer.size());
	outFile.close();
	cout << "파일 저장 완료." << endl;
	LeaveCriticalSection(&cs);
	
	return 0;
}

int main(int argc, char* argv[])
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	int retval;
	bool isLoop = true;

	//윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	//서버 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	//bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVER_PORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	//Listen 상태로 전환
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	//데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	HANDLE hTread;

	InitializeCriticalSection(&cs);

	while (isLoop) {
		EnterCriticalSection(&cs);
		cout << "클라이언트 접속 대기 중..." << endl;
		LeaveCriticalSection(&cs);
		//accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		//클라이언트 정보 출력
		addrlen = sizeof(clientaddr);
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		EnterCriticalSection(&cs);
		cout << "클라이언트 접속: " << addr << ":" << ntohs(clientaddr.sin_port) << endl;
		LeaveCriticalSection(&cs);

		//클라이언트 처리 스레드 생성
		hTread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, NULL);
		if (hTread == NULL) {
			err_display("CreateThread()");
			closesocket(client_sock);
		}
		else CloseHandle(hTread);
	}

	DeleteCriticalSection(&cs);
	closesocket(listen_sock);
	WSACleanup();
	system("pause");
}