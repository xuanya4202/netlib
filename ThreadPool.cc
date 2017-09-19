#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include "ThreadPool.h"

using namespace std;

int ThreadPool::start_pool()
{
    int i = 0;
    pthread_t pid = 0;
    for(i = 0; i < num; ++i)
    {
        pid = start();
        if(0 < pid)
        {
            threads_pid.push_back(pid);
        }
        else
        {
            cout<<"thread pool create thread failed !!! total num:"<<num<<" current i:"<<i<<endl;
            exit(0);
        }
    }
}

