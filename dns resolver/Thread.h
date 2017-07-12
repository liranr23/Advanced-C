#ifndef THREAD_H
#define THREAD_H

#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
//base thread class
class Thread{
protected:
    //run task function
    virtual void run() = 0;
private:
    //enum of the current thread state
    enum Thread_state
    {
        Thread_state_None,
        Thread_state_Started,
        Thread_state_Joined
    };
    Thread_state state;
    bool joined;
    //the current thread
    pthread_t handle;
    //casting from cpp class to c with the task, running it
    static void* threadProc(void* param)
    {
        Thread* thread = reinterpret_cast<Thread*>(param);
        thread->run();
        return NULL;
    }
public:
    Thread();
    virtual ~Thread();
    void start();
    void join();
};
#endif
