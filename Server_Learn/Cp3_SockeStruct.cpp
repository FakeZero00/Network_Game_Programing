#include <iostream>
#include "Common.h"
#include "Execution.h"
using namespace std;

#define endianNull -1
#define endianBig 1
#define endianLittle 0

static int CheckEndian() {
	short s = 0x0102;
	char* p = (char*)&s;

	printf("테스트용 short형 변수: %#x\n", s);
	cout << "먼저 저장된 정보: " << (int)p[0] << endl;
	cout << "나중에 저장된 정보: " << (int)p[1] << endl;
	//큰 자리부터 저장했으므로 big-endian
	if (p[0] == 1 && p[1] == 2) {
		cout << "호스트 컴퓨터는 big-endian 방식을 사용하고 있습니다." << endl;
		return endianBig;
	}
	//작은 자리부터 저장했으므로 little-endian
	else if (p[0] == 2 && p[1] == 1) {
		cout << "호스트 컴퓨터는 little-endian 방식을 사용하고 있습니다." << endl;
		return endianLittle;
	}
	else {
		cout << "호스트 컴퓨터는 big-endian 방식도 little-endian 방식도 아닙니다." << endl;
		return endianNull;
	}
}

int Cp3_SocketStruct() {
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	u_short x1 = 0x1234;
	u_long  y1 = 0x12345678;
	u_short x2;
	u_long  y2;

	// 호스트 바이트 -> 네트워크 바이트
	printf("[호스트 바이트 -> 네트워크 바이트]\n");
	printf("%#x -> %#x\n", x1, x2 = htons(x1));
	printf("%#x -> %#x\n", y1, y2 = htonl(y1));

	// 네트워크 바이트 -> 호스트 바이트
	printf("\n[네트워크 바이트 -> 호스트 바이트]\n");
	printf("%#x -> %#x\n", x2, ntohs(x2));
	printf("%#x -> %#x\n", y2, ntohl(y2));

	// 잘못된 사용 예
	printf("\n[잘못된 사용 예]\n");
	printf("%#x -> %#x\n", x1, htonl(x1));

	//엔디안 확인
	cout << endl;
	int checkEndian = CheckEndian();
	string endianType = checkEndian == endianNull ? "NULL" : (checkEndian == endianBig ? "Big Endian" : "Little Endian");
	cout << "엔디안 확인: " << endianType << endl;
	system("pause");

	// 윈속 종료
	WSACleanup();
	return 0;
}