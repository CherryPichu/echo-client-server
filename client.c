#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
// 참고 : https://www.binarytides.com/socket-programming-c-linux-tutorial/

int BUFFSIZE = 10000;


// 스레드로 동작할꺼고...
// 서버의 응답을 기달렸다가
// 서버의 응답을 받자마자 이 함수를 실행
void listenToServer(int sock){
    char server_message[BUFFSIZE];
    // char* server_message;
    int read_size;
    while(1) {
        int read_size = recv(sock, server_message, BUFFSIZE, 0);
        server_message[read_size] = '\0'; // 이상하게 공백 문자가 안넘겨 받아지네
        if(read_size > 0){
            printf("\necho : %s", server_message);
            printf("me > ");
            fflush(stdout);
        }
    }
}

void main(int argc, char *argv[], const char *str){
    
    int socket_desc;
    struct sockaddr_in server;

    int PORT;
    while(*argv[2] != '\0'){ // port : char[] to int 
        if(isdigit(*argv[2])){
            PORT = PORT * 10 + (*argv[2] - '0');
            argv[2]++;
        } else {
            printf("error : echo-client <ip> <port> \n");
            return 0;
        }
    }
    char* SERVER_IP = argv[1]; // get ip from argv

    // 서버 소켓 생성
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if( socket_desc == -1){
        printf("소켓을 생성할 수 없음\n");
        return -1;
    }

    // 소켓 환경 설정
    server.sin_addr.s_addr = inet_addr(SERVER_IP); // socket ip 주소 format 으로 변경
	server.sin_family = AF_INET;
	server.sin_port = htons( PORT );

    // 소켓 연결
	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("connect error");
        close(socket_desc);
		return -2;
	}
    printf("< Connected > \n");

    // 쓰레드 돌려서 서버의 응답에 printf("해보자");
    pthread_t thread_id;
    int sts;
    sts = pthread_create(&thread_id, NULL, listenToServer, socket_desc);
    pthread_detach(&thread_id); // 쓰레드가 종료되는 즉시 쓰레드의 모든 자월은 되돌려(free)줄 것을 보증
    if(sts != 0){
        perror("쓰레드 생성 에러");
        exit(1);
    }

    char keyboardInput[BUFFSIZE];
    while(strcmp(keyboardInput, "exit")){
        printf("me > ");
        fgets(keyboardInput, BUFFSIZE, stdin);
        keyboardInput[strlen(keyboardInput)] = '\0';
        
        if( send(socket_desc , keyboardInput , strlen(keyboardInput), 0) < 0){
            printf("Send failed");
            return -3;
        }
    }

    close(socket_desc);
}

