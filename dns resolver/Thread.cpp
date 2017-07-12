#include "Thread.h"

Thread::Thread(){
    //starting the thread with state = none
    state = Thread_state_None;
    handle = 0;
}

Thread::~Thread(){
    //checking if the state of the state is not working atm or done
    assert(state != Thread_state_Started || joined);
}
//starting job
void Thread::start(){
    //checking if the thread state is none(resting)
    assert(state == Thread_state_None);
    //creating the thread
    if (pthread_create(&handle, NULL, threadProc, this))
        //error case
        abort();
    //chaning the thread status to started job
    state = Thread_state_Started;
}
//finishing job function
void Thread::join(){
    //checking if the current state is started
    assert(state == Thread_state_Started);
    //waiting for join
    pthread_join(handle, NULL);
    //changing the thread status to joined(finished job)
    state = Thread_state_Joined;
}


