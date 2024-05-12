#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
// 참고 : https://www.binarytides.com/socket-programming-c-linux-tutorial/

#define BUFFSIZE 10000
#define MAX_CLNT 1000
// client handler

int BrodCastOpetion = 0; 
int EchoOpetion = 0; 

char *SERVER_IP = "127.0.0.1";
int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutex;

void client_handler(int socket_id)
{
    char msg[BUFFSIZE];
    int read_size = 1;
    while (1)
    {
        read_size = recv(socket_id, msg, BUFFSIZE, 0);
        if(read_size < 0){
            continue;
        }else if(read_size == 0){
            break;
        }
        msg[read_size] = '\0';

        if( BrodCastOpetion == 1){
            for (int i = 0; i < clnt_cnt; i++)
            {
                if (socket_id == clnt_socks[i])
                    continue;
                send(clnt_socks[i], msg, read_size, 0);
            }
        }

        if( EchoOpetion == 1){
            for (int i = 0; i < clnt_cnt; i++)
            {
                if (socket_id != clnt_socks[i])
                    continue;
                send(clnt_socks[i], msg, read_size, 0);
            }
        }
    }

    pthread_mutex_lock(&mutex);
    for (int i = 0; i < clnt_cnt; i++)
    {
        if (socket_id == clnt_socks[i])
        {
            while (i++ < clnt_cnt - 1)
                clnt_socks[i-1] = clnt_socks[i];
                
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutex);
    printf("클라이언트 하나 종료. 나머지 클라이언트 : %d\n", clnt_cnt);
}


void main(int argc, char *argv[])
{
    
    int socket_desc;
    struct sockaddr_in server, client;
    pthread_t t_id;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-e") == 0) {
            EchoOpetion = 1;
        } else if (strcmp(argv[i], "-b") == 0) {
            BrodCastOpetion = 1;
        }
    }


    int PORT = 0;
    while (*argv[1] != '\0')
    { // port : char[] to int
        if (isdigit(*argv[1]))
        {
            PORT = PORT * 10 + (*argv[1] - '0');
            argv[1]++;
        }
        else
        {
            printf("error : echo-client <ip> <port> \n");
            return 0;
        }
    }
    // printf("%d", PORT);

    // 서버 소켓 생성
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("소켓을 생성할 수 없음\n");
        return -1;
    }

    // 소켓 환경 설정
    server.sin_addr.s_addr = inet_addr(SERVER_IP); // socket ip 주소 format 으로 변경
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    // Bind
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("Bind error");
        return -2;
    }
    printf("< Bind Done > \n");

    // Listen
    listen(socket_desc, 5);
    printf("\nserver >> is listening in %d\n", PORT);
    int c = sizeof(struct sockaddr_in);
    int new_socket;
    while (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c))
    {
        if (new_socket < 0)
        {
            perror("accept failed");
        }
        printf("새로운 클라이언트 \n");

        // send(new_socket, "Hello! new Client", BUFFSIZE, 0);

        pthread_mutex_lock(&mutex);

        if (clnt_cnt >= MAX_CLNT)
        {
            perror("최대 크기의 클라이언트가 접속 중\n");
            continue;
        }
        clnt_socks[clnt_cnt++] = new_socket;

        pthread_mutex_unlock(&mutex);

        pthread_create(&t_id, NULL, client_handler, new_socket);
        pthread_detach(&t_id); // 쓰레드가 종료되는 즉시 쓰레드의 모든 자월은 되돌려(free)줄 것을 보증
    }

    close(socket_desc);
}
