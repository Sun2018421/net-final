#include "ftpserver.h"

int Server_MKDIR(int datasock, char *filename)
{
    int stat = 0;
    if (mkdir(filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
    {
        stat = 1;
        send(datasock, &stat, sizeof(stat), 0);
    }
    else
    {
        send(datasock, &stat, sizeof(stat), 0);
    }
}
int Server_PORT(int *mode)
{
    *mode = 0;
}
int Server_PASV(int *mode)
{
    *mode = 1;
}
int Server_DIR(int sock, int sockctl)
{
    int ret = system("ls -l > tempdir");
    if (ret < 0)
    {
        return -1;
    }
    int fd = open("tempdir", O_RDONLY);
    SendCode(sockctl, 1);
    struct stat st;
    stat("tempdir", &st);
    size_t size = st.st_size;
    sendfile(sock, fd, NULL, size);
    close(fd);
    SendCode(sockctl, 226);
    return 0;
}
int Server_CD(int sock, char *dire)
{
    char buf[MAX];
    chdir(dire);
    getcwd(buf, MAX);
    send(sock, buf, sizeof(char) * MAX, 0);
}
int Server_PWD(int sock)
{
    char buf[MAX];
    getcwd(buf, MAX);
    send(sock, buf, sizeof(char) * MAX, 0);
}

void Server_PUT(int datasock, int sockctl, char *filename)
{
    int ack;
    if (recv(sockctl, &ack, sizeof(ack), 0) < 0)
    {
        SendCode(sockctl, 502); //命令执行失败
        return;
    }

    int status = ntohl(ack);
    if (553 == status) //客户端上传文件失败
    {
        SendCode(sockctl, 553);
        return;
    }

    char name[260];
    memset(name, 0, sizeof(name));
    strcat(name, filename);
    int fd = open(name, O_CREAT | O_WRONLY, 0664);
    if (fd < 0)
    {
        SendCode(sockctl, 502); //命令执行失败
        return;
    }

    flock(fd, LOCK_EX); //上互斥锁
    while (1)
    {
        char data[MAX];
        memset(data, 0, sizeof(data));
        ssize_t s = recv(datasock, data, sizeof(data), 0);
        if (s <= 0)
        {
            if (s < 0)
                SendCode(sockctl, 502); //命令执行失败
            else
                SendCode(sockctl, 226); //命令执行成功
            break;
        }
        write(fd, data, s);
    }
    flock(fd, LOCK_UN); //释放互斥锁
    close(fd);
}
void handle(int arg)
{
    // pthread_detach(pthread_self());

    //int clnt_sock = (long)arg;
    int clnt_sock = arg;
    // sayhello(clnt_sock); //问好
    SendCode(clnt_sock, 220); //连接成功处理相应码

    if (1 == Server_Login(clnt_sock))
    {
        SendCode(clnt_sock, 230);
        int linktype = 0; // 0 代表主动连接
        char cmd[5];
        char cmdarg[MAX];
        while (true)
        {
            int code = Server_recv_cmd(clnt_sock, cmd, cmdarg);
            if (code == 221)
                break;
            if (code == 200)
            {
                int sock_data;
                if (linktype == 0)
                {
                    sock_data = SeverPort(clnt_sock);
                }
                else
                {
                    sock_data = ServerPASV(clnt_sock);
                }

                if (sock_data < 0)
                {
                    close(sock_data);
                    printf("open data socket failed\n");
                    //          return;
                }

                if (strncmp(cmd, "PWD", 3) == 0)
                {
                    Server_PWD(sock_data);
                }
                else if (strncmp(cmd, "GET", 3) == 0)
                {
                    Server_GET(sock_data, clnt_sock, cmdarg);
                }
                else if (strncmp(cmd, "PUT", 3) == 0)
                {
                    Server_PUT(sock_data, clnt_sock, cmdarg);
                }
                else if (strncmp(cmd, "DIR", 3) == 0)
                {
                    Server_DIR(sock_data, clnt_sock);
                }
                else if (strncmp(cmd, "CD", 2) == 0)
                {
                    Server_CD(sock_data, cmdarg);
                }
                else if (strncmp(cmd, "PORT", 4) == 0)
                {
                    Server_PORT(&linktype);
                }
                else if (strncmp(cmd, "PASV", 4) == 0)
                {
                    Server_PASV(&linktype);
                }
                else if (strncmp(cmd, "MKDI", 4) == 0)
                {
                    Server_MKDIR(sock_data, cmdarg);
                }
                close(sock_data);
            }
        }
    }
    else
    {
        SendCode(clnt_sock, 430);
    }
    close(clnt_sock);
}

int Server_Login(int sock)
{
    char buf[MAX];
    char user[MAX];
    char pass[MAX];
    memset(buf, 0, MAX);
    memset(user, 0, MAX);
    memset(pass, 0, MAX);

    if (recv_data(sock, buf, sizeof(char) * MAX) < 0)
    {
        return -1;
    }
    int i = 5;
    int j = 0;
    while (buf[i] != 0)
        user[j++] = buf[i++];
    SendCode(sock, 331);
    // printf("%s",buf);
    memset(buf, 0, MAX);
    if (recv_data(sock, buf, sizeof(char) * MAX) < 0)
    {
        return -1;
    }
    i = 5;
    j = 0;
    while (buf[i] != 0)
    {
        pass[j++] = buf[i++];
    }
    int ret = Server_check(user, pass);
  //  printf("%s %s\n", user, pass);
    return ret;
}
int Server_check(char *user, char *pass)
{
    FILE *fd = fopen(USER, "r");
    if (NULL == fd)
    {
        //print_log()
        return -1;
    }

    char username[MAX];
    char password[MAX];
    char buf[MAX];
    char *line = NULL;
    size_t len = 0;
    int auth = -1;
    while (-1 != getline(&line, &len, fd))
    {
        memset(buf, 0, MAX);
        strcpy(buf, line);

        char *post = strtok(buf, " "); //分割出用户名
        strcpy(username, post);

        if (NULL != post)
        {
            char *post = strtok(NULL, " "); //分割出密码
            strcpy(password, post);
        }

        trimstr(password, (int)strlen(password)); //去掉字符串中放入空格和换行

        if ((strcmp(user, username) == 0) && (strcmp(pass, password) == 0))
        {
            auth = 1;
            break;
        }
    }

    free(line);
    fclose(fd);
    return auth;
}

int Server_recv_cmd(int sock, char *cmd, char *arg)
{
    int code = 200;
    char buf[MAX];
    memset(buf, 0, MAX);
    memset(cmd, 0, 5);
    memset(arg, 0, MAX);
    if (-1 == recv_data(sock, buf, MAX))
    {
        printf("recv cmd error\n");
        return -1;
    }

    //
    char *pos = strtok(buf, " ");
    pos = strtok(NULL, " ");
    strncpy(cmd, buf, 4);
    //strcpy(arg, buf + 4);
    if (pos != NULL)
    {
        strncpy(arg, pos, strlen(pos));
    }
    else
    {
        strcpy(arg, buf + 4);
    }

    if (strncmp(cmd, "QUIT", 4) == 0)
    {
        code = 221;
    }
    else if ((strncmp(cmd, "DIR", 3) == 0) || (strncmp(cmd, "GET", 3) == 0) ||
             (strncmp(cmd, "PWD", 3) == 0) || (strncmp(cmd, "CD", 2) == 0) ||
             (strncmp(cmd, "PUT", 3) == 0) || (strncmp(cmd, "PORT", 4) == 0) || (strncmp(cmd, "PASV", 4) == 0) ||
             (strncmp(cmd, "MKDI", 4) == 0))
    {
        code = 200;
    }
    else
        code = 502;
    printf("--pid : %d , receive :%s and code is %d the sock is %d\n",getpid() ,buf, code, sock);
    SendCode(sock, code);
    return code;
}

int SeverPort(int sock_ctl)
{
    int pre_port;
    if (recv(sock_ctl, &pre_port, sizeof(pre_port), 0) < 0)
    {
        //print_log()
        return -1;
    }

    char buf[1024];
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    getpeername(sock_ctl, (struct sockaddr *)&client_addr, &len);
    inet_ntop(AF_INET, &client_addr.sin_addr, buf, sizeof(buf));

    int sock_data = ConnectSocket(buf, pre_port);
    if (sock_data < 0)
    {
        //print_log()
        return -1;
    }
    printf("the current client port id %d--\n", pre_port);
    return sock_data;
}

int ServerPASV(int sockctl)
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
    if (send(sockctl, (char *)&ack, sizeof(ack), 0) < 0)
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
    printf("the current port is %d--\n", ack);
    return link_socket;
}
void Server_GET(int sockdata, int sockctl, char *filename)
{
  //  printf("get filename %s\n", filename);
    char name[260];
    memset(name, 0, sizeof(name));
    strcat(name, filename);
    int fd = open(name, O_RDONLY);
    if (fd < 0)
    {
        SendCode(sockctl, 550);
    }
    else
    { //共享锁
        flock(fd, LOCK_SH);
        SendCode(sockctl, 150);
        struct stat st;
        stat(name, &st);
        size_t size = st.st_size;
        sendfile(sockdata, fd, NULL, size);
        SendCode(sockctl, 226);
        flock(fd, LOCK_UN); //释放锁
        close(fd);
    }
}
