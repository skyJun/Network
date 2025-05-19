#include <stdio.h>
#include <sys/types.h> // 데이터 타입 (socklen_t)을 사용하기 위한 헤더 파일
#include <signal.h>    // 시그널 처리를 위한 헤더
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // 인터넷 주소 변환 함수 및 구조체 사용을 위한 헤더

/*
TCP 기반 Server 소켓 동작 순서
소켓은 인터페이스(=프레임워크) 과정을 잘 이해해야함
1. 소켓 생성
2. bind : 소켓에 주소 할당
3. listen : 클라이언트 연결 요청 대기
4. accept : 클라이언트 연결 승인
5. read/write : 통신
6. 소켓 닫기
*/

// 서버 포트 번호 상수
const char *PORT = "12345";

int server_sock; // 서버 소켓 파일 디스크립터
int client_sock; // 클라이언트 소켓 파일 디스크립터

void interrupt(int arg)
{
    printf("Bye");

    close(client_sock);
    close(server_sock);
    exit(1); // 프로그램 비정상 종료
}

// 문자열에서 개행 문자 '\n'을 제거하는 함수
void removeEnterChar(char *buf)
{
    int len = strlen(buf);
    for (int i = len - 1; i >= 0; i--)
    {
        if (buf[i] == '\n')
        {
            buf[i] = '\0';
            break;
        }
    }
}

int main()
{
    // SIGINT 시그널을 수신하면 interrupt 함수 실행하도록 설정
    signal(SIGINT, interrupt);

    // 서버 소켓 생성
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1)
    {
        printf("ERROR :: 1_Socket Create Error\n");
        exit(1);
    }

    printf("Server On..\n");

    // 소켓 옵션 설정: 이미 사용 중인 주소라도 재사용 가능하도록 설정
    int optval = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (void *)&optval, sizeof(optval));

    // 서버 주소 정보 설정
    struct sockaddr_in server_addr = {0};            // 서버 주소 구조체 초기화
    server_addr.sin_family = AF_INET;                // IPv4 프로토콜 사용
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 모든 IP로부터의 연결을 허용
    server_addr.sin_port = htons(atoi(PORT));        // 서버 포트 번호 설정 (문자열을 정수로 변환 후 네트워크 바이트 순서로 변환)
    socklen_t server_addr_len = sizeof(server_addr); // 서버 주소 구조체의 크기

    // 소켓과 주소를 바인딩
    if (bind(server_sock, (struct sockaddr *)&server_addr, server_addr_len) == -1)
    {
        printf("ERROR :: 2_bind Error\n");
        exit(1);
    }

    printf("Bind Success\n");

    // 클라이언트의 연결 요청을 대기 (백 로그 큐 크기: 5)
    if (listen(server_sock, 5) == -1)
    {
        printf("ERROR :: 3_listen Error");
            exit(1);
    }

    printf("Wait Client...\n");

    client_sock = 0;                                 // 클라이언트 소켓 초기화
    struct sockaddr_in client_addr = {0};            // 클라이언트 주소 구조체 초기화
    socklen_t client_addr_len = sizeof(client_addr); // 클라이언트 주소 구조체의 크기

    // 서버 메인 루프: 클라이언트 연결 요청을 계속해서 수락
    while (1)
    {
        memset(&client_addr, 0, client_addr_len); // 클라이언트 주소 구조체 초기화

        // 클라이언트의 연결 요청을 수락
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock == -1)
        {
            printf("ERROR :: 4_accept Error\n");
            break;
        }

        printf("Client Connect Success!\n");

        char buf[100]; // 송수신할 데이터 버퍼
        while (1)
        {
            memset(buf, 0, 100); // 버퍼 초기화

            int len = read(client_sock, buf, 99); // 클라이언트로부터 데이터 수신
            removeEnterChar(buf);

            // 클라이언트가 연결을 끊은 경우
            if (len == 0)
            {
                printf("INFO :: Disconnect with client... BYE\n");
                break;
            }

            if (!strcmp("exit", buf))
            {
                printf("INFO :: Client wants to close... BYE\n");
                break;
            }
            write(client_sock, buf, strlen(buf)); // 클라이언트로부터 받은 메시지를 그대로 전송
        }

        close(client_sock);
        printf("Client BYE!\n");
    }

    close(server_sock);
    printf("Server off..\n");
    return 0;
}