#include "Common.h"
#include <vector>
#include <fstream>
#include <iostream>
using namespace std;

char* SERVERIP = (char*)"210.93.56.16";
#define SERVERPORT 9000

int main(int argc, char* argv[]) {
	int retval;

	//윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	//소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	//서버 접속
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	//원하는 파일명 전송
	char filename[256];
	strcpy(filename, argv[1]);
	retval = send(sock, filename, 256, 0);
	if (retval == SOCKET_ERROR) err_quit("send()");

	//데이터 송수신에 사용할 변수
	vector<char> buffer;
	long long fileLen;

	//파일 사이즈 수신
	retval = recv(sock, (char*)&fileLen, sizeof(long long), MSG_WAITALL);
	if (retval == SOCKET_ERROR) {
		err_display("recv()");
		return 1;
	}
	cout << "파일 사이즈 수신 성공: " << fileLen << " bytes" << endl;

	buffer.resize(fileLen); //버퍼 크기 예약
	//실제 데이터 수신
	int totalReceived = 0;
	while (totalReceived < fileLen) {
		//수신할 데이터 크기 수신
		int len;
		retval = recv(sock, (char*)&len, sizeof(int), MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			return 1;
		}
		else if (retval == 0) return 1;

		//수신한 데이터 크기만큼 버퍼에 추가
		retval = recv(sock, buffer.data() + totalReceived, len, MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			return 1;
		}
		else if (retval == 0) return 1;

		totalReceived += len;
		cout << fixed;
		cout.precision(2);
		cout << "\r수신 진행률: " << (double)totalReceived / fileLen * 100 << "%" << flush;
	}
	cout << endl;

	//수신한 데이터 출력
	ofstream outFile(filename, ios::binary);
	if (!outFile.is_open()) {
		err_display("파일을 작성할 수 없습니다.");
		return 1;
	}

	outFile.write(buffer.data(), buffer.size());
	outFile.close();
	cout << "파일 저장 완료." << endl;

	system("pause");
}