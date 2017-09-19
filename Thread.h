#ifndef THREAD_CC_
#define THREAD_CC_
#include <pthread.h>

class Thread{
    public:
        pthread_t pid;
    private:
        static void * start_thread(void *arg);
    public:
        int start();
        virtual void entry() = 0;
};

#endif
