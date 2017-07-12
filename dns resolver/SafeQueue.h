#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H

#include <pthread.h>
#include <queue>
class Task;
class SafeQueue{
private:
    //queue of tasks and mutex\condition to defend it
    /*NOTICE: std::queue is used here!
    by the PDF file it is written that we can use it.
    Also, after asking Orlov he said it's ok to use it.*/
    std::queue<Task*> tasks;
    pthread_mutex_t queue_def;
    pthread_cond_t wake_cond;
public:
    SafeQueue();
    ~SafeQueue();
    //get the next task in the queue
    Task *nextTask();
    //add a new task
    void addTask(Task *nt);
};

#endif