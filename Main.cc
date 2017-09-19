#include <iostream>
#include <unistd.h>
#include "Thread.h"
#include "EpollPoller.h"
using namespace std;

int read_back(unsigned char *buf, int len)
{
    int i = 0;
    cout<<".........."<<endl;
    for(i = 0; i < len; ++i)
    {
        cout<<buf[i]<<endl;
    }
    cout<<"----------"<<endl;
    return 0;
}
int main(int argc, char *argv[])
{
    Poller *poller_ = Poller::newDefaultPoller();
    poller_->ip = "127.0.0.1";
    poller_->port = 2007;
    poller_->callback = read_back;
    poller_->set_thread_num(2);
    poller_->start_pool();
    poller_->start_service();
    
    return 0;
}

