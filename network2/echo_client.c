#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define NAME_SIZE 20
#define MSG_SIZE 100

char name[NAME_SIZE];
char msg[MSG_SIZE];

pthread_t send_tid;    // 메시지를 보내는 스레드 ID
pthread_t receive_tid; // 메시지를 받는 스레드 ID
int exitFlag;

char SERVER_IP[20];
char SERVER_PORT[6];

int client_sock;

void interrupt(int arg)
{
    printf("Bye\n");

    // 스레드를 취소하고 종료를 기다림
    pthread_cancel(send_tid);
    pthread_cancel(receive_tid);

    pthread_join(send_tid, 0);
    pthread_join(receive_tid, 0);

    close(client_sock);
    exit(1);
}

// 메시지를 서버로 보내는 스레드 함수
void *sendMsg()
{
    char buf[NAME_SIZE + MSG_SIZE + 1];
    while (!exitFlag)
    {
        memset(buf, 0, NAME_SIZE + MSG_SIZE);

        fgets(msg, MSG_SIZE, stdin); // 표준 입력으로부터 메시지를 받음
        if (!strcmp(msg, "exit\n"))
        {
            exitFlag = 1;
            write(client_sock, msg, strlen(msg));
            break;
        }
        if (exitFlag)
            break;
        sprintf(buf, "%s %s", name, msg);   // 이름과 메시지를 합쳐 버퍼에 저장
        write(client_sock, buf, strlen(buf)); // 서버로 메시지 전송
    }
}

// 서버로부터 메시지를 받는 스레드 함수
void *receiveMsg()
{
    char buf[NAME_SIZE + MSG_SIZE];
    while (!exitFlag)
    {
        memset(buf, 0, NAME_SIZE + MSG_SIZE);
        int len = read(client_sock, buf, NAME_SIZE + MSG_SIZE - 1);
        if (len == 0)
        {
            printf("INFO :: Server Disconnected\n");
            kill(0, SIGINT);
            exitFlag = 1;
            break;
        }
        printf("%s\n", buf);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("ERROR Input [Ip Addr] [Port Num] [User Name] \n");
        exit(1);
    }
    strcpy(SERVER_IP, argv[1]);
    strcpy(SERVER_PORT, argv[2]);
    sprintf(name, "[%s]", argv[3]);

    signal(SIGINT, interrupt);

    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == -1)
    {
        printf("ERROR :: 1_Socket Create Error\n");
        exit(1);
    }

    struct sockaddr_in server_addr = {0};               // 서버 주소 구조체 초기화
    server_addr.sin_family = AF_INET;                   // 주소 체계를 IPv4로 설정
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP); // 서버 IP 주소 설정
    server_addr.sin_port = htons(atoi(SERVER_PORT));    // 서버 포트 번호 설정
    socklen_t server_addr_len = sizeof(server_addr);

    // connect
    if (connect(client_sock, (struct sockaddr *)&server_addr, server_addr_len) == -1)
    {
        printf("ERROR :: 2_Connect Error\n");
        exit(1);
    }

    // 메시지를 보내고 받는 스레드 생성
    pthread_create(&send_tid, NULL, sendMsg, NULL);
    pthread_create(&receive_tid, NULL, receiveMsg, NULL);

    // 스레드가 종료될 때까지 대기
    pthread_join(send_tid, 0);
    pthread_join(receive_tid, 0);

    close(client_sock);
    return 0;
}