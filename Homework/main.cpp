#include "Common.h"
#include <vector>
#include <fstream>
#include <iostream>
using namespace std;

char* SERVERIP; //= (char*)"116.36.156.99";
#define SERVERPORT 25565
#define BUFFER_SIZE 1024	//1KB

int main(int argc, char* argv[]) {
	SERVERIP = argv[1];

	//원하는 파일 읽기
	char filename[256];
	strcpy(filename, argv[2]);
	//////////////////파일 읽기//////////////////
	ifstream file{ filename, ios::binary | ios::ate };
	if (!file.is_open()) {
		err_display("파일을 열 수 없습니다.");
		return 1;
	}

	//파일 사이즈 확인
	streamsize size = file.tellg();

	//파일 포인터를 처음으로 이동
	file.seekg(0);

	//파일 내용을 읽어오기
	vector<char> buffer(size);
	if (!file.read(buffer.data(), size)) {
		cerr << "파일 읽기에 실패했습니다." << endl;
		err_display("file.read()");
		return 1;
	}

	file.close();
	/////////////////////////////////////
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

	//통신 시작
	retval = send(sock, (char*)&filename, sizeof(char) * 256, 0);
	if (retval == SOCKET_ERROR) {
		cout << "파일 이름 전송 실패" << endl;
		err_display("send()");
		closesocket(sock);
		return 1;
	}
	cout << "파일 이름 전송 성공: " << filename << endl;

	long long fileLen = size;	//파일 사이즈

	//서버와 데이터 통신
	//전체 파일 크기 송신
	retval = send(sock, (char*)&fileLen, sizeof(long long), 0);
	if (retval == SOCKET_ERROR) {
		cout << "파일 사이즈 전송 실패" << endl;
		err_display("send()");
		closesocket(sock);
		return 1;
	}
	cout << "파일 사이즈 전송 성공: " << fileLen << " bytes" << endl;

	const int CHUNK_SIZE = BUFFER_SIZE;
	int totalSent = 0;

	while (totalSent < fileLen) {
		//송신할 데이터 길이(고정 길이 송신)
		int len = min(CHUNK_SIZE, fileLen - totalSent);

		//고정 길이 송신
		retval = send(sock, (char*)&len, sizeof(int), 0);
		if (retval == SOCKET_ERROR) {
			cout << "파일 내용 전송 실패" << endl;
			err_display("send()");
			closesocket(sock);
			break;
		}

		//가변 길이 송신
		retval = send(sock, (char*)buffer.data() + totalSent, len, 0);
		if (retval == SOCKET_ERROR) {
			cout << "파일 내용 전송 실패" << endl;
			err_display("send()");
			closesocket(sock);
			break;
		}

		totalSent += len;
		cout << fixed;
		cout.precision(2);
		cout << "\r전송 진행률: " << (double)totalSent / fileLen * 100 << "%" << flush;
		if (totalSent == fileLen) {
			cout << endl;
			cout << "파일 내용 전송 완료" << endl;
		}
	}

	closesocket(sock);
	WSACleanup();
	return 0;
}