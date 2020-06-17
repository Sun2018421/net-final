#ifndef _FTPCLIENT_H__
#define _FTPCLIENT_H__
#include "../Socket/Mysocket.h"

struct clientcmd
{
    char code[5];
    char arg[256];
};
int Client_PORT(int *mode);
int Client_PASV(int *mode);
int Client_Login(int sock);
int Client_Send_cmd(int sock, struct clientcmd *cmd);
int Client_Read_cmd(char *buf, size_t size, struct clientcmd *cmd);
int ClientPort(int sock_ctl);
int Client_PWD(int sock);
int Client_GET(int datasock, char *filename);
int Client_PUT(int sock, char *filename);
int Client_CD(int sock);
int Client_DIR(int sock, int sockctl);
int ClientPASV(int sockctl);
int Client_MKDIR(int datasock);
#endif