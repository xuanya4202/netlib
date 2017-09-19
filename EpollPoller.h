#ifndef EPOLLPOLLER_H_
#define EPOLLPOLLER_H_

#include "Poller.h"
class EpollPoller: public Poller
{
    public:
        EpollPoller(){};
        ~EpollPoller(){};
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
        int reset_oneshot(int fd);
};

#endif
