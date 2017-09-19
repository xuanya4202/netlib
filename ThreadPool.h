#ifndef THREADPOOL_H_
#define THREADPOOL_H_
#include <pthread.h>
#include <vector>

#include "Thread.h"
using namespace std;

class ThreadPool:public Thread
{
    public:
        ThreadPool(){}
        ThreadPool(int n):num(n){}

        int set_thread_num(int n){num = n; return num;}
        int get_thread_num(void){return num;}
        int start_pool();
        //void (*entry_callback)(void);
    private:
        int num;//thread number
        vector<pthread_t> threads_pid;
        virtual void entry()=0;
};

#endif
