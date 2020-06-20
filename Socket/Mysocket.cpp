#include "Mysocket.h"

int CreateSocket(const char *ip, const int port)  //封装创建socket端口
{
    if ((NULL == ip) || port < 0)
        return -1;

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
        return -1;

    int op = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op));
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("bind error");
        return -1;
    }

    if (listen(server_socket, 20) < 0)
    {
        printf("listen error");
        return -1;
    }
    return server_socket;
}

int AcceptSocket(int server_socket) //接受socket
{
    struct sockaddr_in link;
    socklen_t link_size = sizeof(link);
    int link_socket = accept(server_socket, (struct sockaddr *)&link, &link_size);
    if (link_socket < 0)
    {
        printf("link error");
        return -1;
    }
    return link_socket;
}

int ConnectSocket(const char *ip, const int port) //连接socket
{
    if ((NULL == ip) || port < 0)
        return -1;

    int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket < 0)
        return -1;

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = htons(port);

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("connect failed");
        return -1;
    }
    return client_socket;
}

int SendCode(int sock, int code) //发送响应码
{
    int sendcode = htonl(code);
    //int sendcode = code;
    if (send(sock, &sendcode, sizeof(sendcode), 0) < 0)
    {
        return -1;
    }
    return 1;
}

int RecvCode(int sock) //接受响应码
{
    int recvcode = 0;
    if (recv(sock, &recvcode, sizeof(recvcode), 0) < 0)
    {
        printf("RecvCode failed\n");
        return -1;
    }
    return ntohl(recvcode);
}

void PrintCodeMessage(int Code) //根据响应码打印信息
{
    switch (Code)
    {
    case 220:
        // printf("Connected, sending welcome message... \n220-Ftp-sxf Server version 1.1 beta\n220 hello \n");
        printf("Connected, sending welcome message... \n");
        break;
    case 221:
        printf("successful quit\n-----------------------\n");
        break;
    case 550:
        printf("No such File or directory.\n");
        break;
    case 226:
        printf("Put file successful.\n");
        break;
    case 502:
        printf("Error \n");
        break;
    default:
        break;
    }
}

int readLine(char *buf, int buffersize) //读一行cmd
{
    memset(buf, 0, buffersize);
    if (NULL != fgets(buf, buffersize, stdin))
    {
        char *n = strchr(buf, '\n'); //读入一行字符串 以\n和\0结尾
        if (n)
            *n = '\0';
    }
}

int recv_data(int sock, char *buf, int bufsize) //接受数据
{
    memset(buf, 0, bufsize);
    int len = recv(sock, buf, bufsize, 0);
    if (len <= 0)
        return -1;
    else
        return len;
}
void trimstr(char *str, int n) //去除字符串中的空白和换行
{
    int i = 0;
    for (i = 0; i < n; i++)
    {
        if (isspace(str[i]) || ('\0' == str[i]))
            str[i] = 0;
    }
}
