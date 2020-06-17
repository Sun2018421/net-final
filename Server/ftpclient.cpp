#include "ftpclient.h"

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
        printf("welcome %s! \n", user);
        break;

    default:
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
    printf("client> ");
    fflush(stdout);
    readLine(buf, size);
    char *arg = NULL;
    arg = strtok(buf, " ");
    arg = strtok(NULL, " ");
    //  printf("%s",buf);
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

int ftpclient_open_conn(int sock_ctl) //打开数据连接
{
    int sock_listen = CreateSocket("0.0.0.0", CLIENT_PORT); //创建一个数据连接
    int ack = 1;
    //给服务器发送一个确认，告诉服务器客户端创建好了一条数据链路
    if (send(sock_ctl, (char *)&ack, sizeof(ack), 0) < 0)
    {
        printf("client:ack write error:%d\n", errno);
        return -1;
    }
    int sock_data = AcceptSocket(sock_listen);
    close(sock_listen);
    return sock_data;
}

int ftpclient_list(int sock_ctl, int sock_data) //处理list命令
{
    int tmp = 0;
    if (recv(sock_ctl, &tmp, sizeof(tmp), 0) < 0) //等待服务器连接成功发送过来一个信号
    {
        perror("client:error reading message from server.\n");
        return -1;
    }

    while (1) //接收服务器发送过来的数据
    {
        char buf[MAX];
        memset(buf, 0, sizeof(buf));
        ssize_t s = recv(sock_data, buf, MAX, 0);
        if (s <= 0)
        {
            if (s < 0)
                perror("error");
            break;
        }
        printf("%s", buf);
    }

    //等待服务器发送完成的信号
    if (recv(sock_ctl, &tmp, sizeof(tmp), 0) < 0)
    {
        perror("client:error reading message from server.");
        return -1;
    }
    return 0;
}
int Client_DIR(int sock)
{
    char buf[MAX];
    recv_data(sock, buf, sizeof(char) * MAX);
    printf("%s\n", buf);
}