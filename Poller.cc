#include "Poller.h"
#include "Thread.h"
#include <vector>
#include <queue>
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include "EpollPoller.h"
using namespace std;

extern queue <struct epoll_event> g_vt_events;
extern pthread_mutex_t g_events_mutex;
void Poller::entry()
{
    std::cout<<"thread run \n"<<std::endl;
    int client_sock = 0;
    int close_fd = 0;
    int ret = 0;
    while(1)
    {
         pthread_mutex_lock(&g_events_mutex);
         if(!g_vt_events.empty())
         {  
             struct epoll_event event = g_vt_events.front();
             g_vt_events.pop();
             pthread_mutex_unlock(&g_events_mutex);
             client_sock = event.data.fd;
             cout<<" recv data fd:"<<client_sock<<" pthread id:"<<pthread_self()<<endl; 
             if(event.events & EPOLLIN)
             {
               std::cout<<"接收 数据\n"<<std::endl;
                 ret = RecvFrame(client_sock, &close_fd);
                 if (-1 == ret)
                 {
                   std::cout<<"对方正常退出\n"<<std::endl;
                     Disconnected(close_fd);
                 }
                 else if(-2 == ret)
                 {
                   std::cout<<"关闭连接\n"<<std::endl;
                     Disconnected(close_fd);
                 }
             }
             else
             {
               std::cout<<"对方出现错误\n"<<std::endl;
                 Disconnected(client_sock);
             }
        }
        else
        {
             pthread_mutex_unlock(&g_events_mutex);
        }
    }
}
Poller * Poller::newDefaultPoller()
{
    return new EpollPoller();
}
