#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>     // 시그널 처리를 위한 헤더 파일
#include <sys/types.h>  // 데이터 타입 (socklen_t 등)을 사용하기 위한 헤더 파일
#include <sys/socket.h> // 소켓 프로그래밍을 위한 헤더파일
#include <netinet/in.h> // 인터넷 프로토콜 관련 구조체 (sockaddr_in 등)를 사용하기 위한 헤더 파일
#include <arpa/inet.h>  // 인터넷 주소 변환 함수를 사용하기 위한 헤더 파일
#include <string.h>

/*
TCP 기반 Client 소켓 동작 순서
소켓은 인터페이스(=프레임워크)이므로 해당 과정 반드시 숙지
1. 소켓 생성
2. connect : 연결 요청
3. read/write : 데이터 송수신
4. close : 연결 종료
*/

// 서버 IP 주소와 통신이 열린 포트 번호 상수 선언
const char *SERVER_IP = "127.0.0.1";
const char *SERVER_PORT = "12345";

int client_sock; // 클라이언트 소켓 파일 디스크립터

// SIGINT 시그널이 발생했을 때 호출하는 함수
void interrupt(int arg)
{
    printf("Bye");

    close(client_sock);
    exit(1); // 프로그램 비정상 종료
}

int main()
{
    signal(SIGINT, interrupt);

    // 클라이언트 소켓 생성
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == -1)
    {
        printf("ERROR :: 1_Socket Create Error\n");
        exit(1);
    }

    // 서버 주소 정보 설정
    struct sockaddr_in server_addr = {0};               // 서버 주소 구조체 초기화
    server_addr.sin_family = AF_INET;                   // IPv4 프로토콜 사용
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP); // 서버 IP 주소 설정
    server_addr.sin_port = htons(atoi(SERVER_PORT));    // 서버 포트 번호 설정 (문자열을 정수로 변환 후 네트워크 바이트 순서로 변환)
    socklen_t server_addr_len = sizeof(server_addr);    // 서버 주소 구조체의 크기

    // 서버에 연결 요청
    if (connect(client_sock, (struct sockaddr *)&server_addr, server_addr_len) == -1)
    {
        printf("ERROR :: 2_Connect Error\n");
        exit(1);
    }

    char buf[100];
    while (1)
    {
        memset(buf, 0, 100); // 버퍼 초기화
        scanf("%s", buf);    // 사용자 입력을 받아 버퍼에 저장
        if (!strcmp(buf, "exit"))
        {
            write(client_sock, buf, strlen(buf));
            break;
        }

        write(client_sock, buf, strlen(buf)); // 서버에 입력한 메시지 전송

        memset(buf, 0, 100);
        int len = read(client_sock, buf, 99); // 서버로부터 응답 수신
        if (len == 0)
        {
            printf("INFO :: Server Disconnected\n");
            break;
        }
        printf("%s\n", buf);
    }
    close(client_sock);
    return 0;
}