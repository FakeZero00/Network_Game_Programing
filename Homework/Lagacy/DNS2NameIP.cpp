#include "Common.h"

hostent* GetDNS2NameIP(const char* domain)
{
	struct hostent* ptr = gethostbyname(domain);
	if (ptr == NULL){
		err_display("gethostbyname()");
		return NULL;
	}
	return ptr;
}

int main(int argc, char* argv[]) {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	if(argc != 2) {
		printf("도메인을 정확한 위치에 입력해주세요.\n");
		return 1;
	}

	char* domain = argv[1];
	printf("입력한 도메인: %s\n", domain);
	struct hostent* hostPtr = GetDNS2NameIP(domain);

	if (!hostPtr) {
		printf("유효한 도메인이 아닙니다.\n");
		return 1;
	}

	printf("[호스트 별명]:\n");
	int count = 0;
	for (int i = 0; hostPtr->h_aliases[i] != NULL; i++) {
		if (hostPtr->h_aliases[i]) {
			printf("%s\n", hostPtr->h_aliases[i]);
		}
		count++;
	}
	if (count == 0) {
		printf("없음\n");
	}

	printf("[IP 주소]:\n");
	count = 0;
	for (int i = 0; hostPtr->h_addr_list[i] != NULL; i++) {
		if (hostPtr->h_addr_list[i]) {
			if (hostPtr->h_addrtype == AF_INET) {
				char str[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, hostPtr->h_addr_list[i], str, sizeof(str));
				printf("%s\n", str);
			}
			else if (hostPtr->h_addrtype == AF_INET6) {
				char str[INET6_ADDRSTRLEN];
				inet_ntop(AF_INET6, hostPtr->h_addr_list[i], str, sizeof(str));
				printf("%s\n", str);
			}
		}
		count++;
	}
	if (count == 0) {
		printf("없음\n");
	}

	return 0;
}