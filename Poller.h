#ifndef POLLER_H_
#define POLLER_H_

#include "Thread.h"
#include "ThreadPool.h"

class Poller:public ThreadPool
{
    public :
        int port;
        const char *ip;
        int (*callback)(unsigned char * buf, int leng);
        
        virtual void entry();
        static Poller* newDefaultPoller();
        virtual  int start_service() = 0;
    private :
    virtual int RecvFrame(int fd, int *close_fd) = 0;
    virtual void Disconnected(int client_sock) = 0;
    virtual int reset_oneshot(int fd) = 0;
};

#endif
