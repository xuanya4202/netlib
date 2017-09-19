#include <iostream>
#include <pthread.h>
#include "Thread.h"
int Thread::start()
{
  //pthread_t pid = 0;
  if(pthread_create(&pid, NULL, start_thread, (void*)this) != 0)
  {
    return -1;
  }
  std::cout<<" pthread create  pid:"<<pid<<std::endl;
  return pid;
}

void *Thread::start_thread(void *arg)
{
  std::cout<<"start thread"<<std::endl;
  Thread *ptr = (Thread *)arg;
  ptr->entry();
}


