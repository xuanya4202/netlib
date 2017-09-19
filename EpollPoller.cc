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
#include "EpollPoller.h"
#include <queue>
#include <vector>
using namespace std;

queue <struct epoll_event> g_vt_events;
pthread_mutex_t g_events_mutex = PTHREAD_MUTEX_INITIALIZER;

/*int EpollPoller::printf_t(unsigned char *buf, size_t len)
{
    int i = 0;
    printf("..........\n");
    for(i = 0; i < len; ++i)
    {
        printf("%x ", buf[i]);
    }
    printf("----------\n");
}*/

int EpollPoller::RecvFrame(int fd, int *close_fd)
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
            return -1; 
        }
        else if( count < 0)
        {
            if (errno == EAGAIN)
            {
                cout << "没有读到数据..." << endl;
                reset_oneshot(fd);
                return 0;
            }
            else
            {
                cout << "链接出现错误" <<endl;
                return -1;
            }
        }
        callback(buff, count);
        if(count == MAX_READ_FRAME)
        {
            cout << "接收帧数据超过最大值" <<endl;
            rs = 1;
        }
    }
}

int EpollPoller::close_net(int fd)
{
    cout << "close net dev!" <<endl;
    if(fd > 0)
    {
        if(close(fd) == -1)
            return -1;
        fd = -1;
    }
    return 0;
}
void EpollPoller::Disconnected(int client_sock)
{
    struct epoll_event ev_reg;
    close_net(client_sock);

    ev_reg.data.fd = client_sock;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_sock, &ev_reg) < 0)
    {
        cout << "recv_process:epoll delete success " << endl;
    }
    else
    {
        cout << " recv_process:epoll delete failed " <<endl;
    }
}

void EpollPoller::recv_poress(void)
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
        cout << " epoll_wait start epoll_fd:" << epoll_fd << endl;
        maxfds = epoll_wait(epoll_fd, eventstmp, MAX_EPOLL_NUM, -1);
        cout << "epoll_wait stop" << endl;
        for(i = 0; i < maxfds; ++i)
        {
            client_sock = eventstmp[i].data.fd;
            if(client_sock == socketfd)
            {
                //新连接
                cout << "new client!" << endl;
                net_accept();
                continue;
            }
            cout<<"event fd:"<<client_sock<<endl;
            pthread_mutex_lock(&g_events_mutex);
            g_vt_events.push(eventstmp[i]);
            pthread_mutex_unlock(&g_events_mutex);
        }
    }
}

int EpollPoller::init_socket(unsigned int port)
{
    int socketfd;
    struct sockaddr_in server_addr;
    // SOCK_STREAM -> tcp
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == socketfd)
    {
        cout << " socket error! " << endl;
        return -1;
    }
    cout << " port:" << port << endl;
    memset((void *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    memset((void*)&(server_addr.sin_zero), 0, 8);

    if (bind(socketfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in)) < 0)
    {
        cout << " bind error! " << endl;
        return -1;
    }
    //等待队列为5
    if(listen(socketfd, 5) < 0)
    {
        cout << " listen error! " << endl;
    }
    
    return socketfd;
}

int EpollPoller::reset_oneshot(int fd)
{
    
    cout<<"reset oneshot fd:"<<fd<<" epoll_fd:"<<epoll_fd<<endl;
    struct epoll_event ev_reg; 
    ev_reg.data.fd = fd;
    //ev_reg.events = EPOLLPRI| EPOLLIN | EPOLLERR |EPOLLHUP|EPOLLONESHOT;
    ev_reg.events = EPOLLIN | EPOLLET|EPOLLONESHOT ;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev_reg) < 0)
    {
        cout << " server add client to epoll ctl failed. clifd:"<< fd << endl;
    }
}

int EpollPoller::net_accept()
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
        int old_option = fcntl(clifd, F_GETFL);
        int new_option = old_option|O_NONBLOCK;
        fcntl(clifd, F_SETFL, new_option);

        printf("new client clifd:%d\n", clifd);
        ev_reg.data.fd = clifd;
        //ev_reg.events = EPOLLPRI| EPOLLIN |EPOLLET | EPOLLERR |EPOLLHUP;
        //ev_reg.events = EPOLLPRI| EPOLLIN | EPOLLERR |EPOLLHUP|EPOLLONESHOT;
        ev_reg.events = EPOLLIN | EPOLLET |EPOLLONESHOT;
        if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clifd, &ev_reg) < 0)
        {
            printf(" server add client to epoll ctl failed. clifd:", clifd);
     //       continue;
        }
    //}
}
int EpollPoller::start_service()
{
    int ret = 0;
    pthread_t recv_poress_fd;
    epoll_fd = epoll_create(100);
    struct epoll_event ev_reg; 
    socketfd = init_socket(port);
    if (0 >= socketfd)
    {
        cout << " init socket faile! " << endl;
        return -1;
    }
    cout << "init socket succ socketfd:" << socketfd << endl;
    
    int old_option = fcntl(socketfd, F_GETFL);
    int new_option = old_option|O_NONBLOCK;
    fcntl(socketfd, F_SETFL, new_option);

    ev_reg.data.fd = socketfd;
    ev_reg.events = EPOLLPRI| EPOLLIN |EPOLLET | EPOLLERR |EPOLLHUP;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socketfd, &ev_reg) < 0)
    {
        cout << " server add client to epoll ctl failed. clifd:" <<  socketfd << endl;
    }
    recv_poress();
    return 0;
}
