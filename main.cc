#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#define MAX_READ_FRAME 1024
#define MAX_EPOLL_NUM 1000
class net{
    public:
        int port;
        const char * ip;
        int (*callback)(unsigned char * buf, int leng);
        net(const char *ip, int port, int (*callback)(unsigned char *buf, int leng)):ip(ip), port(port), callback(callback){};
        ~net(){};
        int start_service();
    private:
        int fd;
        int socketfd;
        int epoll_fd;
        int printf_t(unsigned char *buf, size_t len);
        int RecvFrame(int fd, int *close_fd);
        int close_net(int fd);
        void Disconnected(int client_sock);
        void recv_poress(void);
        int init_socket(unsigned int port);
        int net_accept();
};

int net::printf_t(unsigned char *buf, size_t len)
{
    int i = 0;
    printf("..........\n");
    for(i = 0; i < len; ++i)
    {
        printf("%x ", buf[i]);
    }
    printf("----------\n");
}

int net::RecvFrame(int fd, int *close_fd)
{
    int count = 0;
    unsigned char rCharCase = 0;
    int recLen = 0;
    unsigned char buff[MAX_READ_FRAME];
    unsigned char recBuf[MAX_READ_FRAME];

    int i = 0;
    unsigned short frameLen = 0;
    unsigned char nChar;

    int rcvt;
    int rs = 1;
    int j = 0;
    unsigned char sum;
    while(rs)
    {
        count = recv(fd, buff, MAX_READ_FRAME, 0);
        if (count == 0) /* 对方正常断开*/
        {
            *close_fd = fd;
            return 1; 
        }
        else if( count < 0)
        {
            if (errno == EAGAIN)
            {
                printf("没有读到数据...\n");
                return 0;
            }
            else
            {
                printf("链接出现错误\n");
                return 1;
            }
        }
        callback(buff, count);
        if(count == MAX_READ_FRAME)
        {
            printf("接收帧数据超过最大值\n");
            rs = 1;
        }
//        else
//        {
//            rs = 0;
//        }
    }
}

int net::close_net(int fd)
{
    printf("close net dev!\n");
    if(fd > 0)
    {
        if(close(fd) == -1)
            return -1;
        fd = -1;
    }
    return 0;
}
void net::Disconnected(int client_sock)
{
    struct epoll_event ev_reg;
    close_net(client_sock);

    ev_reg.data.fd = client_sock;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_sock, &ev_reg) < 0)
    {
        printf("recv_process:epoll delete success \n");
    }
    else
    {
        printf("recv_process:epoll delete failed\n");
    }
}

void net::recv_poress(void)
{
    int client_sock;  
    int maxfds;
    int i;
    struct epoll_event eventstmp[MAX_EPOLL_NUM];
    struct epoll_event ev_reg;
    int ret;
    int close_fd;

    while(1)
    {
        maxfds = -1;
        printf("epoll_wait start \n");
        maxfds = epoll_wait(epoll_fd, eventstmp, MAX_EPOLL_NUM, -1);
        printf("epoll_wait stop\n");
        for(i = 0; i < maxfds; ++i)
        {
            client_sock = eventstmp[i].data.fd;
            if(client_sock == socketfd)
            {
              //新连接
              printf("new client!\n");
              net_accept();
            }
            else if(eventstmp[i].events & EPOLLIN)
            {
                printf("接收 数据\n");
                ret = RecvFrame(client_sock, &close_fd);
                if (1 == ret)
                {
                    printf("对方正常退出\n");
                    Disconnected(close_fd);
                }
                else if(2 == ret)
                {
                    printf("关闭连接\n");
                    Disconnected(close_fd);
                }
            }
            else
            {
                printf("对方出现错误\n");
                Disconnected(client_sock);
            }
        }
    }
}

int net::init_socket(unsigned int port)
{
    int socketfd;
    struct sockaddr_in server_addr;
    // SOCK_STREAM -> tcp
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == socketfd)
    {
        printf(" socket error! \n");
        return -1;
    }
    printf(" port:%d\n", port);
    memset((void *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    memset((void*)&(server_addr.sin_zero), 0, 8);

    if (bind(socketfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in)) < 0)
    {
        printf(" bind error! \n");
        return -1;
    }
    //等待队列为5
    if(listen(socketfd, 5) < 0)
    {
        printf(" listen error! \n");
    }
    
    return socketfd;
}

int net::net_accept()
{

    struct epoll_event ev_reg; 
    unsigned int clilen;
    int clifd;
    struct sockaddr_in cliaddr;
    //while(1)
   // {
        clilen = sizeof(cliaddr);
        clifd = accept(socketfd, (sockaddr *)&cliaddr, &clilen);
        if (clifd < 0 && errno == EINTR)
        {
        //    continue;
        }
        else if(clifd < 0)
        {
            printf(" net server accept error !");
      //      break;
        }
        printf("new client clifd:%d\n", clifd);
        ev_reg.data.fd = clifd;
        ev_reg.events = EPOLLPRI| EPOLLIN |EPOLLET | EPOLLERR |EPOLLHUP;
        if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clifd, &ev_reg) < 0)
        {
            printf(" server add client to epoll ctl failed. clifd:", clifd);
     //       continue;
        }
    //}
}
int net::start_service()
{
    int ret = 0;
    pthread_t recv_poress_fd;
    epoll_fd = epoll_create(100);
    struct epoll_event ev_reg; 
    socketfd = init_socket(port);
    if (0 >= socketfd)
    {
        printf(" init socket faile! \n");
        return -1;
    }
    printf("init socket succ socketfd:%d\n", socketfd);
    
    ev_reg.data.fd = socketfd;
    ev_reg.events = EPOLLPRI| EPOLLIN |EPOLLET | EPOLLERR |EPOLLHUP;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socketfd, &ev_reg) < 0)
    {
        printf(" server add client to epoll ctl failed. clifd:", socketfd);
    }
    recv_poress();
    return 0;

//    ret = pthread_create(&recv_poress_fd, NULL, (void*)recv_poress, NULL);
//    if(ret < 0)
//    {
//        printf("pthread create faile \n");
//        exit(1);
//    }
/*    
    int clilen;
    int clifd;
    struct sockaddr_in cliaddr;
    while(1)
    {
        clilen = sizeof(cliaddr);
        clifd = accept(socketfd, (void *)&cliaddr, &clilen);
        if (clifd < 0 && errno == EINTR)
        {
            continue;
        }
        else if(clifd < 0)
        {
            printf(" net server accept error !");
        }
        printf("new client clifd:%d\n", clifd);
        ev_reg.data.fd = clifd;
        ev_reg.events = EPOLLPRI| EPOLLIN |EPOLLET | EPOLLERR |EPOLLHUP;
        if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clifd, &ev_reg) < 0)
        {
            printf(" server add client to epoll ctl failed. clifd:", clifd);
            continue;
        }
    }*/
}
int read_back(unsigned char *buf, int len)
{
    int i = 0;
    printf("..........\n");
    for(i = 0; i < len; ++i)
    {
        printf("%x ", buf[i]);
    }
    printf("----------\n");
}

int main(int argc, char *argv[])
{
    net net("127.0.0.1", 8899, read_back);

    net.start_service();
    return 0;
}
