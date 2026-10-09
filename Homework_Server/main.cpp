#include "Common.h"
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

#define SERVER_PORT 9000

int main(int argc, char* argv[])
{
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

	while (isLoop)
	{
		//accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		//클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		cout << "클라이언트 접속: " << addr << ":" << ntohs(clientaddr.sin_port) << endl;

		//파일 이름 받기
		char filename[256];
		retval = recv(client_sock, filename, 256, MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0) {
			cout << "클라이언트가 연결을 종료했습니다." << endl;
			break;
		}
		//filename[retval] = '\0';
		cout << "수시한 파일 이름: " << filename << endl;

		//////////////////파일 읽기//////////////////
		ifstream file{ filename, ios::binary | ios::ate };
		if (!file.is_open()) err_quit("파일을 열 수 없습니다.");

		//파일 사이즈 확인
		streamsize size = file.tellg();

		//파일 포인터를 처음으로 이동
		file.seekg(0);

		//파일 내용을 읽어오기
		vector<char> buffer(size);
		if (!file.read(buffer.data(), size)) {
			cerr << "파일 읽기에 실패했습니다." << endl;
			return 1;
		}

		file.close();
		/////////////////////////////////////
		long long fileLen = size;	//파일 사이즈

		//클라이언트와 데이터 통신
		//전체 파일 크기 송신
		retval = send(client_sock, (char*)&fileLen, sizeof(long long), 0);
		if (retval == SOCKET_ERROR) {
			cout << "파일 사이즈 전송 실패" << endl;
			err_display("send()");
			break;
		}
		cout << "파일 사이즈 전송 성공: " << fileLen << " bytes" << endl;

		const int CHUNK_SIZE = 4096; //4KB씩 전송
		int totalSent = 0;

		while (totalSent < fileLen){
			//송신할 데이터 길이(고정 길이 송신)
			int len = min(CHUNK_SIZE, fileLen - totalSent);

			//고정 길이 송신
			retval = send(client_sock, (char*)&len, sizeof(int), 0);
			if (retval == SOCKET_ERROR) {
				cout << "파일 내용 전송 실패" << endl;
				err_display("send()");
				break;
			}

			//가변 길이 송신
			retval = send(client_sock, (char*)buffer.data() + totalSent, len, 0);
			if (retval == SOCKET_ERROR) {
				cout << "파일 내용 전송 실패" << endl;
				err_display("send()");
				break;
			}

			totalSent += len;
			cout << fixed;
			cout.precision(2);
			cout << "\r전송 진행률: " << (double)totalSent / fileLen * 100 << "%" << flush;
			if (totalSent == fileLen) {
				cout << endl;
				cout << "파일 내용 전송 완료" << endl;
				isLoop = false;
			}
		}
		closesocket(client_sock);
	}

	closesocket(listen_sock);
	system("pause");
}