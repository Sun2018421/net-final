#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "ftpserver.h"
#include <pthread.h>
using namespace std;

int main(int agrc, char **argv)
{
    int server_socket = CreateSocket("127.0.0.1", 1316);
    if (server_socket < 0)
    {
        printf("create socket failed\n");
        exit(-1);
    }
    while (1)
    {
        int clnt_sock = AcceptSocket(server_socket);
        if (clnt_sock < 0)
        {
            printf("accept socket failed\n");
            continue;
        }   
        pthread_t tid;
        void * p = &clnt_sock;
      //  pthread_create(&tid , NULL ,handle,(void *)clnt_sock);
      pthread_create(&tid , NULL ,handle,p);
      //  handle((void *)clnt_sock);
    }
    close(server_socket);

    return 0;
}
