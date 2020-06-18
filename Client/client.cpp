#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <zconf.h>
#include <cstring>
#include <stdio.h>
#include "ftpclient.h"
using namespace std;

int main()
{
    int client_socket = ConnectSocket("127.0.0.1", 1316);
    if (client_socket < 0)
    {
        printf("connect failed\n");
        return -1;
    }
    /*
    read(client_socket, buffer, sizeof(char) * 256);
    printf("%s", buffer);
    */
    PrintCodeMessage(RecvCode(client_socket));

    if (Client_Login(client_socket) < 0)
    {
        printf("password error\n");
        return -1;
    }
    int linktype = 0; //初始化主动连接
    while (true)
    {
        if (linktype == 0)
        {
            printf("== link type : PORT ==\n");
        }
        else
        {
            printf("== link type : PASV ==\n");
        }
        char buf[MAX];
        memset(buf, 0, sizeof(char) * MAX);
        struct clientcmd cmd;
        memset(&cmd, 0, sizeof(cmd)); //初始化工作

        int ret = Client_Read_cmd(buf, sizeof(buf), &cmd); //从cmd中读取指令
        if (ret < 0)
        {
            printf("Invalid Command,please use ? for help\n");
            continue;
        }
        else if (ret == 2)
        {
            printf("get <filename> -- 获取文件\nput <filename>-- 上传文件\npwd -- 显示当前文件路径\ndir -- 显示当前目录\ncd -- 切换目录\nPORT -- 切换主动连接模式\nPASV -- 切换被动连接模式\nmkdi <directory> -- 创建目录\nrm <filename> -- 删除文件\nquit -- 退出\n");
            continue;
        }
        else if (ret == 0)
        {
            if (send(client_socket, buf, strlen(buf), 0) < 0)
            {
                printf("send to server cmd error\n");
                break;
            }
        }

        int code = RecvCode(client_socket);
        printf("--the code is %d--\n", code);
        if (code == 221)
        {
            PrintCodeMessage(221);
            break;
        }

        if (code == 502)
        {
            printf("%d Invalid command.\n", code);
        }
        else
        {
            int datasock; //默认是主动连接
            if (linktype == 0)
                datasock = ClientPort(client_socket);
            else
            {
                datasock = ClientPASV(client_socket);
            }

            if (datasock < 0)
            {
                printf("open data socket failed\n");
            }
            if (strcmp(cmd.code, "DIR") == 0)
            {
                Client_DIR(datasock, client_socket);
                close(datasock);
            }
            else if (strcmp(cmd.code, "GET") == 0)
            {
                if (RecvCode(client_socket) == 550)
                {
                    PrintCodeMessage(550);
                    close(datasock);
                    continue;
                }
                Client_GET(datasock, cmd.arg);
                PrintCodeMessage(RecvCode(client_socket));
                close(datasock);
            }
            else if (strcmp(cmd.code, "PWD") == 0)
            {
                Client_PWD(datasock);
                close(datasock);
            }
            else if (strcmp(cmd.code, "CD") == 0)
            {
                Client_CD(datasock);
                close(datasock);
            }
            else if (strcmp(cmd.code, "PUT") == 0)
            {

                if (Client_PUT(datasock, cmd.arg) < 0)
                {
                    SendCode(client_socket, 553); //文件失败
                }
                else
                {
                    SendCode(client_socket, 200);
                }
                //      printf("filename is %s\n", cmd.arg);
                close(datasock);
                PrintCodeMessage(RecvCode(client_socket));
            }
            else if (strcmp(cmd.code, "PORT") == 0)
            {
                Client_PORT(&linktype);
                close(datasock);
            }
            else if (strcmp(cmd.code, "PASV") == 0)
            {
                Client_PASV(&linktype);
                close(datasock);
            }
            else if (strcmp(cmd.code, "MKDI") == 0)
            {
                Client_MKDIR(datasock);
                close(datasock);
            }
            else if (strcmp(cmd.code, "RM") == 0)
            {
                Client_RM(datasock);
                close(datasock);
            }
        }
    }
    close(client_socket);
    return 0;
}
