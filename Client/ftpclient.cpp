#include "ftpclient.h"

int Client_CAT(int datasock)
{
    char buf[MAX];
    printf("--------------------\n");
    while (recv(datasock, buf, sizeof(char) * MAX, 0) > 0)
    {
        printf("%s", buf);
        memset(buf, 0, sizeof(char) * MAX);
    }

    printf("--------------------\n");
}
int Client_RM(int datasock)
{
    int stat = 1;
    recv(datasock, &stat, sizeof(stat), 0);
    if (stat == 1)
    {
        printf("remove file successfully\n");
    }
    else
    {
        printf("remove file failed\n");
    }
}
int Client_MKDIR(int datasock)
{
    int stat = 1;
    recv(datasock, &stat, sizeof(stat), 0);
    if (stat == 1)
    {
        printf("created directory successfully\n");
    }
    else
    {
        printf("created directory failed\n");
    }
}
int Client_PORT(int *mode)
{
    *mode = 0;
}
int Client_PASV(int *mode)
{
    *mode = 1;
}

int Client_DIR(int sock, int sockctl)
{
    int temp = 0;
    if (recv(sockctl, &temp, sizeof(temp), 0) < 0)
    {
        return -1;
    }
    while (1)
    {
        char buf[MAX];
        memset(buf, 0, sizeof(buf));
        ssize_t s = recv(sock, buf, MAX, 0);
        if (s <= 0)
        {
            if (s < 0)
            {
                printf("dir recive error\n");
            }
            break;
        }
        printf("%s", buf);
    }
    int code;
    //修改
    RecvCode(sockctl);
}
int Client_CD(int sock)
{
    char buf[MAX];
    recv_data(sock, buf, sizeof(char) * MAX);
    printf("%s\n", buf);
}
int Client_PUT(int sock, char *filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
        return -1;

    struct stat st;
    if (stat(filename, &st) < 0)
        return -1;

    sendfile(sock, fd, NULL, st.st_size);
    close(fd);
    return 0;
}
int Client_GET(int datasock, char *filename)
{
    int fd = open(filename, O_CREAT | O_WRONLY, 0664);
    while (1)
    {
        ssize_t s = 0;
        char data[MAX];
        memset(data, 0, sizeof(data));
        s = recv(datasock, data, sizeof(char) * MAX, 0);
        if (s <= 0)
        {
            if (s < 0)
                printf("read file error");
            break;
        }
        write(fd, data, s);
    }
    close(fd);
    return 0;
}
int Client_Login(int sock)
{
    struct clientcmd cmd;
    char user[256];
    memset(user, 0, sizeof(user));
    printf("User Name: ");
    fflush(stdout);
    readLine(user, 256);
    // printf("%s", user);
    strcpy(cmd.code, "user");
    strcpy(cmd.arg, user);

    Client_Send_cmd(sock, &cmd); //发送给服务器账号

    int code;
    recv(sock, &code, sizeof(code), 0);
    // printf("接受到的code :%d\n",code);
    fflush(stdout);

    char *pass = getpass("Password: ");
    strcpy(cmd.code, "pass");
    strcpy(cmd.arg, pass);
    Client_Send_cmd(sock, &cmd);
    int reccode = RecvCode(sock);
    switch (reccode)
    {
    case 230:
        printf("----- welcome %s! -----\n", user);
        break;

    default:
        printf("--code is %d--\n", reccode);
        return -1;
        break;
    }
    return 0;
}

int Client_Send_cmd(int sock, struct clientcmd *cmd)
{
    char buf[MAX];
    sprintf(buf, "%s %s", cmd->code, cmd->arg);
    if (send(sock, buf, strlen(buf), 0) < 0)
    {
        printf("send to server cmd errro\n");
        return -1;
    }
}

int Client_Read_cmd(char *buf, size_t size, struct clientcmd *cmd)
{
    memset(cmd->code, 0, sizeof(cmd->code));
    memset(cmd->arg, 0, sizeof(cmd->arg));
    printf(">> ");
    fflush(stdout);
    readLine(buf, size);
    char *arg = NULL;
    arg = strtok(buf, " ");
    arg = strtok(NULL, " ");
    //    printf("%s",buf);
    if (NULL != arg)
    {
        strncpy(cmd->arg, arg, strlen(arg));
    }

    if (strncmp(buf, "dir", 2) == 0)
    {
        strcpy(cmd->code, "DIR");
    }
    else if (strncmp(buf, "get", 3) == 0)
    {
        strcpy(cmd->code, "GET");
    }
    else if (strncmp(buf, "pwd", 3) == 0)
    {
        strcpy(cmd->code, "PWD");
    }
    else if (strncmp(buf, "cd", 2) == 0)
    {
        strcpy(cmd->code, "CD");
    }
    else if (strncmp(buf, "quit", 4) == 0)
    {
        strcpy(cmd->code, "QUIT");
    }
    else if (strncmp(buf, "put", 3) == 0)
    {
        strcpy(cmd->code, "PUT");
    }
    else if (strncmp(buf, "PORT", 4) == 0)
    {
        strcpy(cmd->code, "PORT");
    }
    else if (strncmp(buf, "PASV", 4) == 0)
    {
        strcpy(cmd->code, "PASV");
    }
    else if (strncmp(buf, "mkdi", 4) == 0)
    {
        strcpy(cmd->code, "MKDI");
    }
    else if (strncmp(buf, "rm", 2) == 0)
    {
        strcpy(cmd->code, "RM");
    }
    else if (strncmp(buf, "cat", 3) == 0)
    {
        strcpy(cmd->code, "CAT");
    }
    else if (strncmp(buf, "?", 1) == 0)
    {
        return 2;
    }
    else
        return -1;
    memset(buf, 0, size);
    strcpy(buf, cmd->code);
    if (NULL != arg)
    {
        strcat(buf, " ");
        strncat(buf, cmd->arg, strlen(cmd->arg));
    }
    return 0;
}

int ClientPort(int sock_ctl) //打开数据连接
{
    int listenfd;
    // int sock_listen = CreateSocket("0.0.0.0", CLIENT_PORT); //创建一个数据连接，已经连接
    struct sockaddr_in servaddr;
    struct sockaddr_in listenaddr;
    socklen_t listernaddrLen;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    servaddr.sin_port = htons(0); //内核分配端口

    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    listen(listenfd, 20);
    listernaddrLen = sizeof(listenaddr);
    getsockname(listenfd, (struct sockaddr *)&listenaddr, &listernaddrLen);
    int ack = ntohs(listenaddr.sin_port);
    //给服务器发送一个确认，告诉服务器客户端创建好了一条数据链路
    if (send(sock_ctl, (char *)&ack, sizeof(ack), 0) < 0)
    {
        printf("client:ack write error:%d\n", errno);
        return -1;
    }

    struct sockaddr_in link;
    socklen_t link_size = sizeof(link);
    int link_socket = accept(listenfd, (struct sockaddr *)&link, &link_size);
    if (link_socket < 0)
    {
        printf("link error");
        return -1;
    }
    close(listenfd);
    printf("--the current port is %d--\n", ack);
    return link_socket;
}

int ClientPASV(int sockctl)
{
    int pre_port;
    if (recv(sockctl, &pre_port, sizeof(pre_port), 0) < 0)
    {
        //print_log()
        return -1;
    }

    char buf[1024];
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    getpeername(sockctl, (struct sockaddr *)&client_addr, &len);
    inet_ntop(AF_INET, &client_addr.sin_addr, buf, sizeof(buf));

    int sock_data = ConnectSocket(buf, pre_port);
    if (sock_data < 0)
    {
        //print_log()
        return -1;
    }
    printf("--the current server port id %d--\n", pre_port);
    return sock_data;
}
int Client_PWD(int sock)
{
    char buf[MAX];
    recv_data(sock, buf, sizeof(char) * MAX);
    printf("%s\n", buf);
    fflush(stdout);
}