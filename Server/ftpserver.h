#ifndef _FTPSERVER_H__
#define _FTPSERVER_H__

#include "../Socket/Mysocket.h"
#include <sys/stat.h> 

void *handle(void *arg);
int Server_Login(int sock);
int Server_check(char *user,char *pass);
int Server_recv_cmd(int sock,char *cmd , char*arg);
int Server_start_data_conn(int sock);
int SeverPort(int sock_ctl);
int Server_PWD(int sock);
void Server_GET(int sockdata,int sockctl,char *filename);
int Server_CD(int sock,char *dire);
int Server_DIR(int sock,int sockctl);
int Server_PORT(int *mode);
int Server_PASV(int *mode);
int ServerPASV(int sockctl);
int Server_MKDIR(int datasock ,char *filename);
#endif