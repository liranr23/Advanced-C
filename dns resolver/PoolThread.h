#ifndef POOLTHREAD_H
#define POOLTHREAD_H

#include "Thread.cpp"
#include "SafeQueue.cpp"
#include "Task.cpp"
//this thread is used for thread in the thread pool
class PoolThread : public Thread{
private:
    //safequeue to get tasks from
    SafeQueue& safe_queue;
protected:
    //running function(running the task)
    virtual void run();
public:
    PoolThread(SafeQueue& _safe_queue);
};

#endif
