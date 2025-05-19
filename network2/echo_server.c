/*
network1 서버는 하나의 클라이언트를 위한 서버
1:N 다수의 클라이언트가 접속 가능한 서버로 구축
1:N 젒5ㅗㄱ을 위한 변수와 Thread 선언

TCP 기반 Server 소켓 동작
1. 소켓 생성
2. bind : 소켓에 주소 할당
3. listen : 클라이언트 연결 요청 대기
4. accept : 클라이언트 연결 승인
5. read/write :  통신
6. close : 소켓 닫기
*/
#include <stdio.h>
#include <signal.h> // 신호 처리 함수
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>  // 시스템 호출에 사용되는 데이터 타입
#include <sys/socket.h> // 소켓 함수와 데이터 구조체
#include <arpa/inet.h>  // 인터넷 작업 관련 함수 (inet_addr 등)
#include <pthread.h>    // POSIX 스레드 라이브러리

#define MAX_CLIENT_CNT 500 // 최대 클라이언트 수

char PORT[6];
int server_sock;                                      // 서버 소켓 디스크립터
int client_sock[MAX_CLIENT_CNT] = {0};                // 클라이언트 소켓 디스크립터 배열
struct sockaddr_in client_addr[MAX_CLIENT_CNT] = {0}; // 클라이언트 주소 구조체 배열

pthread_t tid[MAX_CLIENT_CNT]; // 클라이언트별 스레드 ID 배열
int exitFlag[MAX_CLIENT_CNT];  // 클라이언트별 종료 플래그 배열

pthread_mutex_t mlock; // 임계 구역 보호를 위한 뮤텍스

void interrupt(int arg)
{
    printf("\nBye\n");

    // 모든 클라이언트의 연결을 종료하고 스레드 정리
    for (int i = 0; i < MAX_CLIENT_CNT; i++)
    {
        if (client_sock[i] != 0)
        {
            pthread_cancel(tid[i]);
            pthread_join(tid[i], 0);
            close(client_sock[i]);
        }
    }
    close(server_sock);
    exit(1);
}

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

// 사용할 수 있는 클라이언트 ID를 반환하는 함수
int getClientID()
{
    for (int i = 0; i < MAX_CLIENT_CNT; i++)
    {
        if (client_sock[i] == 0)
        {
            return i;
        }
    }
    return -1;
}

// 각 클라이언트를 처리하는 스레드 함수
void *client_handler(void *arg)
{
    int id = *(int *)arg; // 클라이언트 id를 받아옴

    char client_IP[100];                                    // 클라이언트 IP주소를 저장할 변수
    strcpy(client_IP, inet_ntoa(client_addr[id].sin_addr)); // 클라이언트 IP 주소 복사
    printf("INFO :: Connect new Client (ID :  %d, IP : %s)\n", id, client_IP);

    char buf[MAX_CLIENT_CNT] = {0};
    while (1)
    {
        memset(buf, 0, MAX_CLIENT_CNT); // 버퍼 초기화
        int len = read(client_sock[id], buf, MAX_CLIENT_CNT);
        if (len == 0)
        {
            printf("INFO :: Disconnect with client.. BYE\n");
            exitFlag[id] = 1;
            break;
        }

        removeEnterChar(buf); // 메시지에서 '\n' 문자 제거
        pthread_mutex_lock(&mlock);

        // 모든 클라이언트에게 메시지를 브로드캐스트
        for (int i = 0; i < MAX_CLIENT_CNT; i++)
        {
            if (client_sock[i] != 0)
            {
                write(client_sock[i], buf, strlen(buf));
            }
        }

        pthread_mutex_unlock(&mlock);
    }
    close(client_sock[id]);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("ERROR Input Port Num\n");
        exit(1);
    }
    strcpy(PORT, argv[1]);

    signal(SIGINT, interrupt);
    pthread_mutex_init(&mlock, NULL); // 뮤텍스 초기화

    // 소켓 생성
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1)
    {
        printf("ERROR :: 1_Socket Create Error\n");
        exit(1);
    }

    printf("Server On..\n");

    int optval = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (void *)&optval, sizeof(optval)); // 소켓 옵션 설정

    struct sockaddr_in server_addr = {0};            // 서버 주소 구조체 초기화
    server_addr.sin_family = AF_INET;                // 주소 체계를 IPv4로 설정
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 모든 인터페이스에서 연결 허용
    server_addr.sin_port = htons(atoi(PORT));        // 포트 번호 설정

    // 소켓과 주소정보 바인딩
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        printf("ERROR :: 2_bind Error\n");
        exit(1);
    }

    printf("Bind Success\n");

    // 클라이언트 연결 요청 대기
    if (listen(server_sock, 5) == -1)
    {
        printf("ERROR :: 3_listen Error\n");
        exit(1);
    }

    printf("Wait Client...\n");

    socklen_t client_addr_len = sizeof(struct sockaddr_in);
    int id_table[MAX_CLIENT_CNT]; // 클라이언트 ID 테이블

    while (1)
    {
        int id = getClientID();
        id_table[id] = id;

        if (id == -1)
        {
            printf("WARNING :: Client FULL\n");
            sleep(1);
            continue;
        }

        memset(&client_addr[id], 0, sizeof(struct sockaddr_in)); // 클라이언트 주소 초기화

        // 클라이언트 연결 수락
        client_sock[id] = accept(server_sock, (struct sockaddr *)&client_addr[id], &client_addr_len);
        if (client_sock[id] == -1)
        {
            printf("ERROR :: 4_accept Error\n");
            break;
        }

        // 클라이언트를 처리할 스레드 생성
        pthread_create(&tid[id], NULL, client_handler, (void *)&id_table[id]);

        // 종료된 클라이언트 스레드 정리
        for (int i = 0; i < MAX_CLIENT_CNT; i++)
        {
            if (exitFlag[i] == 1)
            {
                exitFlag[i] = 0;
                pthread_join(tid[i], 0);
                client_sock[i] = 0;
            }
        }
    }
    pthread_mutex_destroy(&mlock);
    close(server_sock);
    return 0;
}