#ifndef _Mysocket__
#define _Mysocket__
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>    
#include <sys/stat.h>   
#include <fcntl.h>
#include <sys/sendfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#define MAX 1024
#define USER "us"
#define SERVER_PORT 1319
#define CLIENT_PORT 1320
int CreateSocket(const char *ip, const int port);
int AcceptSocket(int server_socket);
int ConnectSocket(const char *ip, const int port);
int recv_data(int sock,char * buf ,int bufsize);

int SendCode(int sock, int code); //服务器发送ftp标准的code
int RecvCode(int sock);           //客户端接受code
void PrintCodeMessage(int Code);  //客户端通过code打印消息
int readLine(char *buf ,int buffersize);
void trimstr(char* str,int n);
#endif